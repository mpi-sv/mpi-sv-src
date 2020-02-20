/******************************/  /*-
 * Copyright (c) 2009 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */


/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   implemented by this module                                   *
 *----------------------------------------------------------------*/
#include <com.h>

/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   used by this module                                          *
 *----------------------------------------------------------------*/
#if defined (__OSI)
  #include <osi.h>
#else
  #include <pthread.h>
  #include <string.h>
  #include <stdio.h>
  #include <errno.h>
#endif

#include <azq.h>
#include <azq_types.h>
#include <xpn.h>
#include <addr.h>
#include <addr_hddn.h>
#include <inet.h>
#include <thr.h>
#include <thr_dptr.h>
#include <mbx.h>
#include <rpq.h>
#include <rqst.h>
#include <util.h>

/*----------------------------------------------------------------*
 *   Definition of private constants                              *
 *----------------------------------------------------------------*/
#define NONE_SATISFIED   (-1)

/*----------------------------------------------------------------*
 *   Definition of private data                                   *
 *----------------------------------------------------------------*/
extern void INET_start(void);

extern pthread_key_t  key;
       int            cpuId = -1;
static int            initialised = FALSE;

static const
       char  *e_names[10] = { /* This order has to be consistent with com.h */
                              /*  0 */ "COM_E_OK",
                              /*  1 */ "COM_E_EXHAUST",
                              /*  2 */ "COM_E_INTEGRITY",
                              /*  3 */ "COM_E_TIMEOUT",
                              /*  4 */ "COM_E_INTERFACE",
                              /*  5 */ "COM_E_SYSTEM",
                              /*  6 */ "COM_E_SIGNALED",
                              /*  7 */ "COM_E_DEADPART",
                              /*  8 */ "COM_E_REQUEST",
                              /*  9 */ "COM_E_INTERN"
                            };

int azq_err[10] = { AZQ_SUCCESS,
                    AZQ_E_EXHAUST,
                    AZQ_E_INTEGRITY,
                    AZQ_E_TIMEOUT,
                    AZQ_E_INTERFACE,
                    AZQ_E_SYSTEM,
                    AZQ_E_SIGNALED,
                    AZQ_E_DEADPART,
                    AZQ_E_REQUEST,
                    AZQ_E_INTERN
                  };

/*----------------------------------------------------------------*
 *   Declaration of private functions                             *
 *----------------------------------------------------------------*/
#define self()                   ((Thr_t)pthread_getspecific(key))
        int  (*getEndCom)        (void *srcThr, Addr_t dst, int *mchn, void *thr);

static  int  deliver_rrv_ack     (Header_t hdr, Thr_t dstThr, int *success);
static  int  deliver_rrv_rqst    (Header_t hdr, Thr_t dstThr, int *success);
static  int  deliver_data        (Header_t hdr, Thr_t dstThr, int *success);

static  void SLM_SET_put         (SlmSet_t set);
  
static  int  sendRrvRqstAndWait  (INET_iovec *iov, int dstMchn, Thr_t me, unsigned timeout);
static  int  sendRrvRqst         (INET_iovec *iov, int dstMchn, Thr_t me, unsigned timeout);
static  int  sendRrvAck          (Header_t  hdr, Rqst_t dstRqst);


//#define _ATOMIC
#ifdef _ATOMIC

#define ACQUIRE(thr)  \
while (!__sync_bool_compare_and_swap(&((thr)->s_counter), 0, 1)) { \
sched_yield(); \
}


#define RELEASE(thr)  {   \
__sync_fetch_and_add(&((thr)->s_counter), -1); \
}


#define LOCK(thr)    ACQUIRE(thr)
#define UNLOCK(thr)  RELEASE(thr)

#endif


/*----------------------------------------------------------------*
 *   Implementation of private functions                          *
 *----------------------------------------------------------------*/

   /*________________________________________________________________
  /                                                                  \
 |    deliver_rrv_ack                                                 |
 |                                                                    |
 |    A clear to send arrives from the receiver                       |
 |                                                                    |
 |    PARAMETERS:                                                     |
 |    o hdr          (input)                                          |
 |        Message or fragment arrived                                 |
 |    o dstThr       (input)                                          |
 |        Thread descriptor to deliver the message                    |
 |    o success      (output)                                         |
 |        TRUE if the message can be erased from low-level buffers    |
 |                                                                    |
  \____________/  ___________________________________________________/
              / _/
             /_/
			*/
static int deliver_rrv_ack (Header_t hdr, Thr_t dstThr, int *success) {
  
  Rqst_t      dstRqst;
  Header_t    hdr2;
  INET_iovec  iov[2];
  int         ready      = 0;
  int         excpn      = COM_E_SYSTEM;
  static
  char       *where      = "deliver_rrv_ack";
  
  
  DBG_PRNT((stdout, "deliver_rrv_ack: header: %p  dstThr: %p\n", hdr, dstThr));
  
  LOCK(dstThr);
  
  /* 1. Is there a request matching the message received? */
  if (RPQ_get(dstThr->AsendPendReg, &dstRqst, &hdr->Src, hdr->Tag, 0)) {
	
		/* 1.1. Register the destination user buffer address into the async request */
		dstRqst->RrvAddr = hdr->RrvDstAddr;
	
		/* 1.2. Remove send request from the queue */
		RPQ_remove(dstThr->AsendPendReg, dstRqst);
	
		UNLOCK(dstThr);
	
		/* 1.3. Build message and send data */
		hdr2 = &(dstRqst->Hdr);
		hdr2->RrvDstAddr = dstRqst->RrvAddr;
		hdr2->Mode      |= MODE_RRV_DATA;
		hdr2->Mode      &= ~MODE_RRV_ACK;
	
		iov[0].Data     = (char *)hdr2;
		iov[0].Size     = HEADER_NET_SZ;
		iov[1].Data     = (char *)hdr2->Payload;
		iov[1].Size     = hdr2->PayloadSize;
	
		if (0 > (INET_send(iov, hdr2->DstMchn, COM_PROTOCOL)))                      goto exception;
	
		/* 1.4. Wake up destination thread if neccessary */
		LOCK(dstThr);
	
		if (RQST_ready(dstThr, dstRqst)) ready = 1;
		dstRqst->State = RQST_SATISFIED;
	
		UNLOCK(dstThr);
		if (ready)   SIGNAL(dstThr);
	
		*success = TRUE;
		DBG_PRNT((stdout, "deliver_rrv_ack: sended data. End.\n"));
		return COM_E_OK;
  }
  
  /* 2. Register the destination user buffer address into the sync request */
  dstThr->SyncRqst.RrvAddr = hdr->RrvDstAddr;
  
  /* 3. Wake up the thread waiting */
  UNLOCK(dstThr);
  SIGNAL(dstThr);
  
  *success = TRUE;
  DBG_PRNT((stdout, "deliver_rrv_ack: End (signaled %p by RRV_ACK)\n", dstThr));
  return(COM_E_OK);
  
exception:
  XPN_print(excpn);
  return(excpn);  
}



   /*________________________________________________________________
  /                                                                  \
 |    deliver_rrv_rqst                                                |
 |                                                                    |
 |    A message of type MODE_RRV_RQST has arrived, handle it          |
 |                                                                    |
 |    PARAMETERS:                                                     |
 |    o hdr          (input)                                          |
 |        Message or fragment arrived                                 |
 |    o dstThr       (input)                                          |
 |        Thread descriptor to deliver the message                    |
 |    o success      (output)                                         |
 |        TRUE if the message can be erased from low-level buffers    |
 |                                                                    |
  \____________/  ___________________________________________________/
              / _/
             /_/
            */
static int deliver_rrv_rqst (Header_t hdr, Thr_t dstThr, int *success) {
  
  Rqst_t      dstRqst;
  int         ready      = 0;
  int         excpn;
  static
  char       *where      = "deliver_rrv_rqst";
  
  
  DBG_PRNT((stdout, "deliver_rrv_rqst: header: %p  dstThr: %p\n", hdr, dstThr));
  
  LOCK(dstThr);
  /* 1. Is there a request matching the message received? */
  if (RPQ_get(dstThr->RecvPendReg, &dstRqst, &hdr->Src, hdr->Tag, hdr->Mode)) {
	
	/* 1.2. Receiver waiting in a PROBE request */
	if (dstRqst->Type & RQST_PROBE) {
	  
	  RPQ_remove(dstThr->RecvPendReg, dstRqst);
	  
	  /* 1.2.1. Wake up destination thread */
	  if (RQST_ready(dstThr, dstRqst)) {
		DBG_PRNT((stdout, "\ndeliver: Probed request. Signaling to %p\n", dstThr));
		ready = 1;
	  }
	  dstRqst->State = RQST_SATISFIED;
	  
	  /* 1.2.2. Fill data for probe */
	  dstRqst->Status.Src    = hdr->Src;
	  dstRqst->Status.Tag    = hdr->Tag;
	  dstRqst->Status.Count  = hdr->MessageSize;
	  
	  /* 1.2.3. Enqueue the message containing data */
	  MBX_put(dstThr->MailBox, hdr);
	  
	  UNLOCK(dstThr);
	  if (ready) SIGNAL(dstThr);
	  
	  *success = FALSE;
	  return COM_E_OK;
	}
	
	/* 1.1. Mark the request to only receive the rest of the message from this source  */
	dstRqst->Status.Src  = hdr->Src;
	dstRqst->Status.Tag  = hdr->Tag;
	dstRqst->Type       &= ~RQST_ANY;
	dstRqst->Type       |= RQST_RRV_DATA;		
	dstRqst->Type       |= RQST_ACKED;
	
	/* 2. Receiver not waiting. Enqueue message in mailbox. */
  } else {
	DBG_PRNT((stdout, "\ndeliver_rrv_rqst: Putting message in MBX %p of %p\n", dstThr->MailBox, dstThr));
	MBX_put(dstThr->MailBox, hdr);
	
	UNLOCK(dstThr);
	
	*success = FALSE;
	DBG_PRNT((stdout, "deliver_rrv_rqst: End\n"));	
	return(COM_E_OK);
  }  
  
  UNLOCK(dstThr);	  
  
  /* 3. Request found. Send Ack to sender */
  hdr->RrvDstAddr = dstRqst->Buff;
  if (0 > (excpn = sendRrvAck(hdr, dstRqst)))                                   goto exception;
  *success = TRUE;
  
  return(COM_E_OK);
  
exception:
  XPN_print(excpn);
  return(excpn);  
}


   /*________________________________________________________________
  /                                                                  \
 |    deliver_data                                                    |
 |                                                                    |
 |    A message containing data arrives from network                  |
 |                                                                    |
 |    PARAMETERS:                                                     |
 |    o hdr          (input)                                          |
 |        Message or fragment arrived                                 |
 |    o dstThr       (input)                                          |
 |        Thread descriptor to deliver the message                    |
 |    o success      (output)                                         |
 |        TRUE if the message can be erased from low-level buffers    |
 |                                                                    |
  \____________/  ___________________________________________________/
              / _/
             /_/
            */
static int deliver_data (Header_t hdr, Thr_t dstThr, int *success) {
  
  Rqst_t      dstRqst;
  int         ready      = 0;
  int         excpn;
  static
  char       *where      = "deliver_data";
  
  
  DBG_PRNT((stdout, "deliver_data: header: %p  dstThr: %p\n", hdr, dstThr));
  
  LOCK(dstThr);
  
  /* 1. Is there a request matching the message received? */
  if (RPQ_get(dstThr->RecvPendReg, &dstRqst, &hdr->Src, hdr->Tag, hdr->Mode)) {
    
	DBG_PRNT((stdout, "\ndeliver_data: Found rqst %p in RPQ\n", dstRqst, dstThr->RecvPendReg));
	
	/* 1.1. Receiver waiting in a PROBE request */
	if (dstRqst->Type & RQST_PROBE) {
	  
	  RPQ_remove(dstThr->RecvPendReg, dstRqst);
	  
	  /* 1.2.1. Wake up destination thread */
	  if (RQST_ready(dstThr, dstRqst)) {
		DBG_PRNT((stdout, "\ndeliver: Probed request. Signaling to %p\n", dstThr));
		ready = 1;
	  }
	  dstRqst->State = RQST_SATISFIED;
	  
	  /* 1.2.2. Fill data for probe */
	  dstRqst->Status.Src    = hdr->Src;
	  dstRqst->Status.Tag    = hdr->Tag;
	  dstRqst->Status.Count  = hdr->MessageSize;
	  
	  /* 1.2.3. Enqueue the message containing data */
	  MBX_put(dstThr->MailBox, hdr);
	  
	  UNLOCK(dstThr);
	  if (ready) SIGNAL(dstThr);
	  
	  *success = FALSE;
	  return COM_E_OK;
	}
	
	/* 1.2. Last fragment arrived completes request */
	if (hdr->Mode & MODE_LAST_FRAGMENT)  RPQ_remove(dstThr->RecvPendReg, dstRqst);

	/* 1.3. Initially the request could receive data or RRV_RQST. If data, the 
	 rest of fragments needs to be marked as DATA */
	if (hdr->Mode & MODE_DATA) {
		  dstRqst->Status.Src    = hdr->Src;
		  dstRqst->Status.Tag    = hdr->Tag;
      dstRqst->Type 				&= ~RQST_ANY;
      dstRqst->Type 				|= RQST_DATA;
    }
	
	/* 2. No request found, put header in MBX */
  }  else {
    /* 2.1. Receiver not waiting. Enqueue message in mailbox */
    DBG_PRNT((stdout, "\ndeliver_data: Putting message in MBX %p of %p\n", dstThr->MailBox, dstThr));
	
    MBX_put(dstThr->MailBox, hdr);
	UNLOCK(dstThr);
	
	/* 2.2. Message can not be erased from low-level buffer */
	*success = FALSE;
	DBG_PRNT((stdout, "deliver_data: End\n"));
	return(COM_E_OK);
  }
  
  UNLOCK(dstThr);
  
  
  /* 3. Data message. Feed and eventually signal the receiver */
  if (0 > (excpn == RQST_feed(dstRqst, hdr)))                                   goto exception;
  
  if (hdr->Mode & MODE_LAST_FRAGMENT) {
	
	LOCK(dstThr);
	/* 3.1. Is it the destination blocked waiting this message? */
	if (RQST_ready(dstThr, dstRqst)) {
	  DBG_PRNT((stdout, "\ndeliver_data: Signaling to %p\n", dstThr));
	  ready = 1;
	}
	dstRqst->State = RQST_SATISFIED;
	
	UNLOCK(dstThr);
	if (ready)  SIGNAL(dstThr);
	
  }
  
  *success = TRUE;
  DBG_PRNT((stdout, "deliver_data: End\n"));
  return(COM_E_OK);
  
exception:
  XPN_print(excpn);
  return(excpn);  
}



   /*________________________________________________________________
  /                                                                  \
 |    deliver                                                         |
 |                                                                    |
 |    Upcall called from underlying level when a message has arrived  |
 |    Puts the message in its right mailbox                           |
 |                                                                    |
 |    PARAMETERS:                                                     |
 |    o iov          (input)                                          |
 |        Describes the message: Header + Payload                     |
 |    o last_frgmt   (input)                                          |
 |        Is it the last fragment of a network message?               |
 |    o success      (output)                                         |
 |        TRUE if the message can be erased from low-level buffers    |
 |                                                                    |
  \____________/  ___________________________________________________/
              / _/
             /_/
            */
int deliver (INET_iovec *iov, int last_frgmt, int *success) {
  
  Thr_t       dstThr;
  Header_t    hdr;
  int         dummy;
  int         excpn;
  static
  char       *where      = "deliver";
  
  
  hdr = (Header_t)(iov->Data);
  
  DBG_PRNT((stdout, "deliver: LastFrgmt = %d. Message from [0x%x, %d] to [0x%x, %d] and tag %d (mode: %x)\n",
            last_frgmt, hdr->Src.Group, hdr->Src.Rank, hdr->Dst.Group, hdr->Dst.Rank, hdr->Tag, hdr->Mode));
  
  /* 1. Who is the receiver? */
  if(0 > getEndCom(NULL, &(hdr->Dst), &dummy, &dstThr))                        {excpn = COM_E_DEADPART;
	                                                                            goto exception;}
  
  /* 2. Build header from fragment received */
  hdr->Payload     = (char *)(iov->Data) + HEADER_NET_SZ;
  hdr->PayloadSize = iov->Size           - HEADER_NET_SZ;
  if(last_frgmt)
    hdr->Mode |= MODE_LAST_FRAGMENT;
  
  /* 3. Handle message */
  if (hdr->Mode & MODE_RRV_RQST)     excpn = deliver_rrv_rqst (hdr, dstThr, success);
  else if (hdr->Mode & MODE_RRV_ACK) excpn = deliver_rrv_ack  (hdr, dstThr, success);
  else                               excpn = deliver_data     (hdr, dstThr, success);
  
  if (0 > excpn)                     goto exception;
  
  DBG_PRNT((stdout, "deliver: End\n"));
  return(COM_E_OK);
  
exception:
  XPN_print(excpn);
  return(excpn);  
}
  

/*----------------------------------------------------------------*
 *   SLM_SET package (begin)                                      *
 *----------------------------------------------------------------*/

      /*________________________________________________________________
     /                                                                  \
    |    SLM_SET_put                                                     |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
static void SLM_SET_put(SlmSet_t set) {

  Header_t hdr = &set->Hdr[set->Turno];

  DBG_PRNT((stdout, "SLM_SET_put(%p): (turn %d  Cnt %d)\n", self(), set->Turno, set->Cnt));

  memcpy(&(set->Buffer[set->Turno][0]), hdr->Payload, hdr->PayloadSize);
  hdr->Payload = &(set->Buffer[set->Turno][0]);
  set->Cnt++;
  set->Turno++;
  if(set->Turno == SLM_BUFFERS_PER_THREAD) {
    set->Turno  = 0;
    set->State |= SLM_SET_FULL;
  }

  DBG_PRNT((stdout, "SLM_SET_put(%p):  End.\n", self()));

  return;
}

/*----------------------------------------------------------------*
 *   SLM package (end)                                            *
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------*
 *   RRV package (start)                                          *
 *----------------------------------------------------------------*/


      /*________________________________________________________________
     /                                                                  \
    |    sendRrvRqst                                                     |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
static int sendRrvRqstAndWait(INET_iovec *iov, int dstMchn, Thr_t me, unsigned timeout) {

  int          excpn;
  Header_t     hdr     = (Header_t)iov->Data;
  static char *where   = "sendRrvRqstAndWait";


  DBG_PRNT((stdout, "sendRrvRqstAndWait(%p): \n", me));

  hdr->Mode    |= MODE_RRV_RQST;
  iov[1].Data   = (char *)NULL;
  iov[1].Size   = 0;

  LOCK(me);

  if(0 > (excpn = INET_send(iov, dstMchn, COM_PROTOCOL)))                      goto exception;

  excpn = TIMEDWAIT(me, me, timeout);

  hdr->Mode    &= ~MODE_RRV_RQST;

  UNLOCK(me);

  DBG_PRNT((stdout, "sendRrvRqstAndWait(%p): End\n", me));

  return(COM_E_OK);

exception:
  XPN_print(excpn);
  return(excpn);
}

      /*________________________________________________________________
     /                                                                  \
    |    sendRrvRqst                                                     |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
static int sendRrvRqst (INET_iovec *iov, int dstMchn, Thr_t me, unsigned timeout) {

  int          excpn;
  Header_t     hdr       = (Header_t)iov->Data;
  Header       hdr2      = *hdr;
  static char *where     = "sendRrvRqst";

  DBG_PRNT((stdout, "sendRrvRqst(%p): \n", me));

  hdr2.Mode    |= MODE_RRV_RQST;
  iov[0].Data   = (char *)&hdr2;
  iov[1].Data   = (char *)NULL;
  iov[1].Size   = 0;

  if (0 > (excpn = INET_send(iov, dstMchn, COM_PROTOCOL)))                      goto exception;

  DBG_PRNT((stdout, "sendRrvRqst(%p): End\n", me));

  return COM_E_OK;

exception:
  XPN_print(excpn);
  return excpn;
}

      /*________________________________________________________________
     /                                                                  \
    |    sendRrvAck                                                      |
    |                                                                    |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
static int sendRrvAck(Header_t hdr, Rqst_t dstRqst) {

  INET_iovec        iov[2];
  Header            hdr2;
  int               excpn;
  static char      *where    = "sendRrvAck";

  DBG_PRNT((stdout, "sendRrvAck to [G %d  R %d]:\n", hdr->Src.Group, hdr->Src.Rank));

  hdr2.Dst         = hdr->Src;
  hdr2.DstMchn     = hdr->SrcMchn;
  hdr2.Src         = hdr->Dst;
  hdr2.SrcMchn     = hdr->DstMchn;
  hdr2.Tag         = hdr->Tag;
  hdr2.Mode        = hdr->Mode | MODE_RRV_ACK;
  hdr2.Mode       &= ~MODE_RRV_RQST;
  hdr2.RrvDstAddr  = dstRqst->Buff;

  iov[0].Data   = (char *)&hdr2;
  iov[0].Size   = HEADER_NET_SZ;
  iov[1].Data   = (char *)NULL;
  iov[1].Size   = 0;

  if(0 > (excpn = INET_send(iov, hdr2.DstMchn, COM_PROTOCOL)))                 goto exception;

  DBG_PRNT((stdout, "sendRrvAck: End\n"));

  return(COM_E_OK);

exception:
  XPN_print(excpn);
  return(excpn);
}

/*----------------------------------------------------------------*
 *   RRV package (end)                                            *
 *----------------------------------------------------------------*/


/*----------------------------------------------------------------*
 *   Implementation of public interface                           *
 *----------------------------------------------------------------*/

      /*________________________________________________________________
     /                                                                  \
    |    TIMEDWAIT                                                       |
    |                                                                    |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int TIMEDWAIT(Thr_t me, Thr_t dstThr, unsigned relTimeout) {

  int               excpn;
  static char      *where    = "TIMEDWAIT";

  DBG_PRNT((stdout, "TIMEDWAIT(%p): timeout %d\n", me, relTimeout));

  if(((me)->State & SIGNALED) ) {
    (me)->State &= ~SIGNALED;
    DBG_PRNT((stdout, "TIMEDWAIT(%p): Killed. End\n", me));
    return(COM_E_SIGNALED);
  }

  /* 1. Use timeout */
  if(relTimeout != COM_FOREVER) {
    struct timespec   abstime;
    milliRel2posixAbs(relTimeout, &abstime);

    DBG_PRNT((stdout, "TIMEDWAIT(%p): calling pthread_cond_timedwait\n", me));

    if((excpn = pthread_cond_timedwait(&((me)->Ready), &((dstThr)->Lock), &abstime))) {
      switch(excpn) {
        case ETIMEDOUT:
          XPN_print(COM_E_TIMEOUT);
          DBG_PRNT((stdout, "TIMEDWAIT(%p): TIMEOUT!!!  End\n", me));
          return(COM_E_TIMEOUT);
          break;
        default:
          DBG_PRNT((stdout, "TIMEDWAIT(%p): pthread_cond_timedwait returned %d\n", me, excpn));
          panic("TIMEDWAIT");
      }
    }
  /* 2. Unconditional wait */
  } else {

    DBG_PRNT((stdout, "TIMEDWAIT(%p): calling pthread_cond_wait\n", me));
    if(pthread_cond_wait(&((me)->Ready), &((dstThr)->Lock)))                 panic("TIMEDWAIT");

    DBG_PRNT((stdout, "TIMEDWAIT(%p): waking up from pthread_cond_wait\n", me));

  }

  if(((me)->State & SIGNALED)) {
    (me)->State &= ~SIGNALED;
    DBG_PRNT((stdout, "TIMEDWAIT(%p): Killed. End\n", me));
    return(COM_E_SIGNALED);
  }

  DBG_PRNT((stdout, "TIMEDWAIT(%p): End\n", me));
  return(COM_E_OK);
}



   /*________________________________________________________________
  /                                                                  \
 |    send_local                                                      |
 |                                                                    |
 |    Deliver a local message with timeout                            |
 |                                                                    |
 |    PARAMETERS:                                                     |
 |    o rqst     (input/output)                                       |
 |        Send request                                                |	
 |                                                                    |	
 |    o timeout  (input)                                              |
 |                                                                    |
  \____________/  ___________________________________________________/
              / _/
             /_/
            */

int send_local (Rqst_t rqst, unsigned timeout) {
  
  Thr_t         dstThr     = rqst->DstThr;
  Rqst_t        dstRqst;
  int           ready      = 0;
  register
  Header_t      hdr;
  int           excpn;
  static
  char         *where      = "send_local";
  

  DBG_PRNT((stdout, "\nsend_local(%p): start\n", self()));  
  hdr  = &(rqst->Hdr);  
  LOCK(dstThr);  
  /*if(dstThr->State & ZOMBIE)                                                   {excpn = COM_E_DEADPART;
	                                                                            UNLOCK(dstThr);
                                                                                goto exception;}*/
  /* 1. Is there a pending request from receiver? */
  if (RPQ_get(dstThr->RecvPendReg, &dstRqst, &hdr->Src, hdr->Tag, 0)) {
    RPQ_remove(dstThr->RecvPendReg, dstRqst);

    /* 1.1. Probe request. Complete PROBE request, enqueue message in MBX and signal */
    if (dstRqst->Type & RQST_PROBE) {
      if (RQST_ready(dstThr, dstRqst)) {
		DBG_PRNT((stdout, "\nsend_probe(%p): Probed request (%p). Signaling to %p\n", self(), dstRqst, dstThr));
		ready = 1;
      }
      dstRqst->State = RQST_SATISFIED;
      dstRqst->Status.Src    = hdr->Src;
      dstRqst->Status.Tag    = hdr->Tag;
      dstRqst->Status.Count  = hdr->MessageSize;
      rqst->State = RQST_PENDING;

      /* 1.1.2. Enqueue the message */
      MBX_put(dstThr->MailBox, hdr);
      UNLOCK(dstThr);
      if (ready) SIGNAL(dstThr);
      return COM_E_OK;
    }
  /* 2. Put message header in mailbox */
  } else {
    DBG_PRNT((stdout, "\nsend_local(%p): Putting message in MBX %p of %p\n", self(), dstThr->MailBox, dstThr));
	
    MBX_put(dstThr->MailBox, hdr);
    RQST_setState(rqst, RQST_PENDING);
    UNLOCK(dstThr);
    return COM_E_OK;
  }
  UNLOCK(dstThr);
  
  /* 3. Copy message */
  if (0 > (excpn = RQST_feed(dstRqst, hdr)))                                    goto exception;
  
  LOCK(dstThr);
  /* 4. Wake up thread if blocked in the request */
  if (RQST_ready(dstThr, dstRqst)) {
    DBG_PRNT((stdout, "\nsend_local(%p): Signaling to %p\n", self(), dstThr));
    ready = 1; 
  }
  RQST_setState(dstRqst, RQST_SATISFIED);
  UNLOCK(dstThr);
  if (ready) SIGNAL(dstThr);
  RQST_setState(rqst, RQST_SATISFIED);
  klee_mpi_nonblock(0, 0, 8, (void*)rqst);
  DBG_PRNT((stdout, "\nsend_local(%p): End.\n", self()));
  return COM_E_OK;
  
exception:
  XPN_print(excpn);
  return(azq_err[-excpn]);
}


   /*________________________________________________________________
  /                                                                  \
 |    send_slm                                                        |
 |                                                                    |
 |    Deliver a local message using Short Local Messages interface    |
 |     with timeout                                                   |
 |                                                                    |
 |    PARAMETERS:                                                     |
 |    o rqst     (input/output)                                       |
 |        Send request                                                |	
 |                                                                    |	
 |    o timeout  (input)                                              |
 |                                                                    |
  \____________/  ___________________________________________________/
              / _/
             /_/
            */
int send_slm (Rqst_t rqst, unsigned timeout) {
  
  Thr_t         dstThr     = rqst->DstThr;
  Thr_t         me         = self();
  Rqst_t        dstRqst;
  int           ready      = 0;
  register
  Header_t      hdr;
  int           excpn;
  static
  char         *where      = "send_slm";
  
  
  DBG_PRNT((stdout, "\nsend_slm(%p): start\n", self()));
  
  hdr  = &(me->SlmSet->Hdr[me->SlmSet->Turno]);
  
  LOCK(dstThr);
  
  if(dstThr->State & ZOMBIE)                                                   {excpn = COM_E_DEADPART;
	                                                                            UNLOCK(dstThr);
	                                                                            goto exception;}
  /* 1. Is there any request in the receiver RPQ? */
  if (RPQ_get(dstThr->RecvPendReg, &dstRqst, &hdr->Src, hdr->Tag, 0)) {
	
	RPQ_remove(dstThr->RecvPendReg, dstRqst);
	
	/* 1.1. Probe request. Complete PROBE request, enqueue message in MBX and signal */
	if (dstRqst->Type & RQST_PROBE) {

	  if (RQST_ready(dstThr, dstRqst)) {
		DBG_PRNT((stdout, "\nsend_probe(%p): Probed request (%p). Signaling to %p\n", me, dstRqst, dstThr));
		ready = 1;
	  }
	  dstRqst->State = RQST_SATISFIED;
	  
	  dstRqst->Status.Src    = hdr->Src;
	  dstRqst->Status.Tag    = hdr->Tag;
	  dstRqst->Status.Count  = hdr->MessageSize;
	  
	  rqst->State = RQST_SATISFIED;
	  SLM_SET_put(me->SlmSet);

	  /* 1.1.2. Enqueue the message */
	  MBX_put(dstThr->MailBox, hdr);
	  	  
	  UNLOCK(dstThr);
	  if (ready) SIGNAL(dstThr);
	  
	  return COM_E_OK;
	}
	
  /* 2. Put header in receiver mailbox */
  } else {
	
	DBG_PRNT((stdout, "\nsend_slm(%p): Putting message in MBX %p of %p\n", self(), dstThr->MailBox, dstThr));
	
	SLM_SET_put(me->SlmSet);
	MBX_put(dstThr->MailBox, hdr);
	RQST_setState(rqst, RQST_SATISFIED);
	
	UNLOCK(dstThr);
	return COM_E_OK;
	
  }
  
  UNLOCK(dstThr);
  
  /* 3. Copy message */
  if (0 > (excpn = RQST_feed(dstRqst, hdr)))                                    goto exception;
    
  LOCK(dstThr);
  
  if (RQST_ready(dstThr, dstRqst)) {
	DBG_PRNT((stdout, "\nsend_slm(%p): Signaling to %p\n", self(), dstThr));
	ready = 1; 
  }
  RQST_setState(dstRqst, RQST_SATISFIED);
  
  UNLOCK(dstThr);
  if (ready) SIGNAL(dstThr);
  
  RQST_setState(rqst, RQST_SATISFIED);
  
  DBG_PRNT((stdout, "\nsend_slm(%p): End.\n", self()));
  return COM_E_OK;
  
exception:
  XPN_print(excpn);
  return(azq_err[-excpn]);
}


   /*________________________________________________________________
  /                                                                  \
 |    send_remote                                                     |
 |                                                                    |
 |    Send a message to a remote thread with timeout                  |
 |                                                                    |
 |    PARAMETERS:                                                     |
 |    o rqst     (input/output)                                       |
 |        Send request                                                |	
 |                                                                    |	
 |    o timeout  (input)                                              |
 |                                                                    |
  \____________/  ___________________________________________________/
              / _/
             /_/
            */
int send_remote (Rqst_t rqst, unsigned timeout) {
  
  INET_iovec    iov[2];
  Thr_t         me         = self();
  register
  Header_t      hdr;
  int           excpn;
  static
  char         *where      = "send_remote";
  
  
  DBG_PRNT((stdout, "\nsend_remote(%p): start\n", me));
  
  hdr  = &(rqst->Hdr);  
  
  if(((me)->State & SIGNALED)) {
	(me)->State &= ~SIGNALED;
	DBG_PRNT((stdout, "send_remote(%p): Killed. End\n", me));
	return(COM_E_SIGNALED);
  }
  
  /* 1. Build the message. Header */
  iov[0].Data  = (char *)hdr;
  iov[0].Size  = HEADER_NET_SZ;
  
  /* 2. Large message. Send it using the Remote RendezVous protocol */
  if (hdr->Mode & MODE_RRV) {
	
	/* 2.1. Asynchronous send */
	if (rqst->Type & RQST_ASYNC) {
	  
	  DBG_PRNT((stdout, "\nsend_remote(%p): RRV Async send. Putting rqst %p in AsendPendReg %p\n", me, rqst, me->AsendPendReg));
	  
	  LOCK(me);
	  rqst->State = RQST_PENDING;
	  RPQ_put(me->AsendPendReg, rqst);
	  UNLOCK(me);
	  
	  if (0 > (excpn = sendRrvRqst(iov, hdr->DstMchn, me, timeout)))          goto exception;
	  
	  return COM_E_OK;
	  /* 2.2. Synchronous send */
	} else {
	  
	  DBG_PRNT((stdout, "\nsend_remote(%p): RRV Sync send. Calling sendRrvRqstAndWait...\n", me));
	  
	  if (0 > (excpn = sendRrvRqstAndWait(iov, hdr->DstMchn, me, timeout)))   goto exception;
	  hdr->RrvDstAddr = me->SyncRqst.RrvAddr;
	  hdr->Mode      |= MODE_RRV_DATA;
	  
	}
	
	/* 3. Short message. Send it using Eager (default) protocol */
  } else {
	hdr->Mode      |= MODE_DATA;
	DBG_PRNT((stdout, "\nsend_remote(%p): Eager send. \n", me));
  }
  
  /* 4. Build data */
  iov[1].Data     = (char *)hdr->Payload;
  iov[1].Size     = hdr->PayloadSize;
  
  /* 5. Send */
  if(0 > (excpn = INET_send(iov, hdr->DstMchn, COM_PROTOCOL)))               goto exception;
  
  rqst->State = RQST_SATISFIED;
  
  DBG_PRNT((stdout, "\nsend_remote(%p): End\n", me));
  return COM_E_OK;
  
exception:
  XPN_print(excpn);
  return(excpn);
}


   /*________________________________________________________________
  /                                                                  \
 |    deal_send                                                       |
 |                                                                    |
 |    Send a message to a remote thread with timeout                  |
 |                                                                    |
 |    PARAMETERS:                                                     |
 |    o rqst     (input/output)                                       |
 |        Send request                                                |	
 |                                                                    |	
 |    o timeout  (input)                                              |
 |                                                                    |
  \____________/  ___________________________________________________/
              / _/
             /_/
            */
int deal_send (Rqst_t rqst, unsigned timeout) {
  
  int           excpn;
  static
  char         *where      = "deal_send";
  
  
  DBG_PRNT((stdout, "\ndeal_send(%p): start\n", self()));
  
  /* All message to send are dealt here. SLM, REMOTE or DEFAULT */
  if      (RQST_isSlm(rqst))               excpn = send_slm    (rqst, timeout); 
  else if (rqst->Hdr.Mode & MODE_REMOTE)   excpn = send_remote (rqst, timeout);
  else                                     excpn = send_local  (rqst, timeout);
  
  if (0 > excpn)   goto exception;
  
  DBG_PRNT((stdout, "\ndeal_send(%p): end.\n", self()));
  
  return COM_E_OK;
  
exception:
  XPN_print(excpn);
  return(azq_err[-excpn]);
}



   /*________________________________________________________________
  /                                                                  \
 |    recv_local                                                      |
 |                                                                    |
 |    Receive a message from a local thread                           |
 |                                                                    |
 |    PARAMETERS:                                                     |
 |    o rqst     (input/output)                                       |
 |      Receive request                                               |
 |    o hdr      (input/output)                                       |
 |      Header of a message found in MBX                              |
 |                                                                    |
  \____________/  ___________________________________________________/
              / _/
             /_/
            */
int recv_local (Rqst_t rqst, Header_t hdr) {
  
  int        excpn;
  Thr_t      srcThr     = (Thr_t) NULL;
  Rqst_t     srcRqst;
  int        ready      = 0;
  static
  char      *where      = "recv_local";
  
  /* 1. Copy message */
  if(0 > (excpn = RQST_feed(rqst, hdr)))                                        goto exception;
  
  /* 2. Wake up sender if blocked */
  srcRqst = (Rqst_t)(hdr->SrcRqst);
  srcThr  = (Thr_t) (hdr->SrcThr);
  
  LOCK(srcThr); 
  
  if (RQST_ready(srcThr, srcRqst)) {
	DBG_PRNT((stdout, "\nrecv_local(%p): Signaling to %p\n", self(), srcThr));
	ready = 1; 
  }
  
  RQST_setState(srcRqst, RQST_SATISFIED);
  
  UNLOCK(srcThr);
  if (ready) SIGNAL(srcThr);
  
  RQST_setState(rqst, RQST_SATISFIED);
    
  return AZQ_SUCCESS;
  
exception:
  XPN_print(excpn);
  return(excpn);  
}


   /*________________________________________________________________
  /                                                                  \
 |    recv_remote                                                     |
 |                                                                    |
 |    Receive a message from a remote thread                          |
 |                                                                    |
 |    PARAMETERS:                                                     |
 |    o rqst     (input/output)                                       |
 |      Receive request                                               |
 |    o hdr      (input/output)                                       |
 |      Header of a message found in MBX                              |
 |                                                                    |
  \____________/  ___________________________________________________/
              / _/
             /_/
            */
int recv_remote (Rqst_t rqst, Header_t hdr) {
  
  int        excpn;
  Addr       t_src      = rqst->Status.Src;
  Tag_t      t_tag      = rqst->Status.Tag;
  static
  char      *where      = "recv_remote";
  
  
  /* 1. Large message. Use RRV protocol */
  if(hdr->Mode & MODE_RRV_RQST) {
		t_src = hdr->Src;
		t_tag = hdr->Tag;
		rqst->Type |= RQST_ACKED;
	
		/* 1.1. Mark the request to only receive the big message from this source  */
		rqst->Status.Src  = hdr->Src;
		rqst->Status.Tag  = hdr->Tag;
		rqst->Type       &= ~RQST_ANY;
		rqst->Type       |= RQST_RRV_DATA;		
	
		hdr->RrvDstAddr   = rqst->Buff;
	
		/* 1.2. Send clear-to-send to sender */
		if (0 > (excpn = sendRrvAck(hdr, rqst)))                                    goto exception;
	
		/* 1.3. Message received, erase it from low-level buffer */
		INET_recv((char *)hdr);
	
		return COM_E_OK;
  }
  
  /* 2. Copy message (fragment) */
  if(0 > (excpn = RQST_feed(rqst, hdr)))                                        goto exception;
  
  /* 3. If last fragment of the message, request satisfied */
  if (hdr->Mode & MODE_LAST_FRAGMENT)  rqst->State = RQST_SATISFIED;
  
  /* 4. A remote message come in fragments. Once the first fragment is received,
   all the other fragments copied must be from the same source and tag.
   This avoid merge fragments from different messages if wildcards are used 
   */
  t_src = hdr->Src;
  t_tag = hdr->Tag;
  if (hdr->Mode & MODE_DATA) {
	rqst->Type &= ~RQST_ANY;
	rqst->Type |= RQST_DATA;
  }
  
  INET_recv((char *)hdr);
  
  return COM_E_OK;
  
exception:
  XPN_print(excpn);
  return(excpn);  
}


   /*________________________________________________________________
  /                                                                  \
 |    deal_recv                                                       |
 |                                                                    |
 |    Receive a message                                               |
 |                                                                    |
 |    PARAMETERS:                                                     |
 |    o rqst     (input/output)                                       |
 |      Receive request                                               |
 |                                                                    |
  \____________/  ___________________________________________________/
              / _/
             /_/
            */
int deal_recv (Rqst_t rqst) {
  
  int        excpn;
  Thr_t      srcThr     = (Thr_t) NULL;
  Thr_t      me         = self();
  Addr_t     t_src      = &rqst->Status.Src;
  Tag_t     *t_tag      = &rqst->Status.Tag;
  Header_t   hdr;
  static
  char      *where      = "deal_recv";
  
  
  DBG_PRNT((stdout, "\ndeal_recv(%p): start\n", me));
  
  do {
	
    LOCK(me);
	
	/* 1. Is there any message matching the request in MBX? */
    if (MBX_get(me->MailBox, &hdr, t_src, *t_tag, rqst->Type)) {
	  
	  DBG_PRNT((stdout, "\ndeal_recv(%p): Found message in MBX\n", me));
	  
	  if(hdr->Mode & MODE_SLM) {
      srcThr  = (Thr_t) (hdr->SrcThr);
      if(0 > (excpn = RQST_feed(rqst, hdr/*, &satisfied*/)))                     goto exception;
      SLM_SET_free(srcThr->SlmSet);
  		RQST_setState(rqst, RQST_SATISFIED);
      UNLOCK(me);
      return AZQ_SUCCESS;
    }
	  
	/* 2. Not found. Request pending */
	} else {
	  
	  DBG_PRNT((stdout, "\ndeal_recv(%p): Putting request in RPQ %p\n", me, rqst));
	  RPQ_put(me->RecvPendReg, rqst);
	  RQST_setState(rqst, RQST_PENDING);
	  UNLOCK(me);
      return AZQ_SUCCESS;
	  
	}
	
    UNLOCK(me);
	
	/* 3. Handle found pending message */
	if (hdr->Mode & MODE_REMOTE)  excpn = recv_remote (rqst, hdr);
	else                          excpn = recv_local  (rqst, hdr);
	
	
  } while (rqst->State != RQST_SATISFIED);
  
  return AZQ_SUCCESS;
  
exception:
  XPN_print(excpn);
  return(azq_err[-excpn]);
}


      /*________________________________________________________________
     /                                                                  \
    |    COM_init                                                        |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int COM_init() {

  int            excpn;
  static char   *where     = "COM_init";

  DBG_PRNT((stdout, "COM_init: \n"));

  if(initialised)
    return(COM_E_OK);

  initialised = TRUE;

  if(0 > (excpn = INET_init(NULL)))                                            goto exception;
  if(0 > (excpn = INET_subscribe(deliver, COM_PROTOCOL)))                      goto exception;
  
  cpuId = INET_getCpuId();

  DBG_PRNT((stdout, "COM_init: Subscribed %p for protocol %d. End\n", deliver, COM_PROTOCOL));

  return(COM_E_OK);

exception:
  XPN_print(excpn);
  return(excpn);
}



      /*________________________________________________________________
     /                                                                  \
    |    COM_finalize                                                    |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
void COM_finalize (void) {

  INET_finalize();

}



      /*________________________________________________________________
     /                                                                  \
    |    COM_setLoc                                                      |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
void COM_setLoc(int (*f)(void *srcThr, Addr_t dstAddr, int *mchn, void *thr)) {
  getEndCom = f;
  return;
}



      /*________________________________________________________________
     /                                                                  \
    |    COM_start                                                       |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
void COM_start (void) {
  INET_start();
}



/*----------------------------------------------------------------*
 *   Implementation of EXPORTED function interface                *
 *----------------------------------------------------------------*/

      /*________________________________________________________________
     /                                                                  \
    |    getCpuId                                                        |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int  getCpuId(void) {
  return cpuId;
}


      /*________________________________________________________________
     /                                                                  \
    |    getAddr                                                         |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
Addr_t getAddr(void) {

  Thr_t   me     = self();

  return(&me->Address);
}

