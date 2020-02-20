/******************************/  /*-
 * Copyright (c) 2009-2011 Universidad de Extremadura
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
#include <config.h>

#if defined (__OSI)
  #include <osi.h>
#else
  #include <pthread.h>
  #include <string.h>
  #include <stdio.h>
  #include <errno.h>
#endif

#include <atomic.h>
#include <arch.h>
#include <azq.h>
#include <azq_types.h>
#include <xpn.h>
#include <addr.h>
#include <inet.h>
#include <thr.h>
#include <thr_dptr.h>
#include <mbx.h>
#include <rqst.h>
#include <util.h>
#include <lfq.h>
#include <dlq.h>

/*----------------------------------------------------------------*
 *   Definition of private constants                              *
 *----------------------------------------------------------------*/
#define NONE_SATISFIED   (-1)

/* Bypass RPQ queue */
//#define RPQ_BY_PASS_OPTIMISE


/*----------------------------------------------------------------*
 *   Definition of private data                                   *
 *----------------------------------------------------------------*/
extern void INET_start(void);

extern pthread_key_t  key;
       int            AZQ_cpuId = -1;
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

static  int  sendRrvRqstAndWait  (INET_iovec *iov, int dstMchn, Thr_t me, unsigned timeout);
static  int  sendRrvRqst         (INET_iovec *iov, int dstMchn, Thr_t me, unsigned timeout);
static  int  sendRrvAck          (Header_t  hdr, Rqst_t dstRqst);



/*----------------------------------------------------------------*
 *   Implementation of private functions                          *
 *----------------------------------------------------------------*/

      /*________________________________________________________________
     /                                                                  \
    |    AZQ_progress                                                    |
    |    Serve the lock-free queue of incoming messages                  |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
void AZQ_progress(Thr_t me) 
{ 
  Rqst_t     srcRqstP; 
  int        lRank; 
 
#ifdef USE_FASTBOXES 
  fastBox_t  fBox; 

  /* Sweep all the fast boxes */ 
  wr_barrier();
  for(lRank = 0; lRank < ((Thr_t)(me))->LocalGroupSize; lRank++) { 
    fBox = &((Thr_t)(me))->FastBox[lRank]; 
    /* Test if it is full */ 
    if((fBox->Turn == TURN_RECV)) { 
      if (fBox->SeqNr == (1 + ((Thr_t)(me))->LastRecvSeqNr[fBox->SenderLocalRank])) { 
        deliverFromFbox(((Thr_t)(me)), fBox); 
      } 
    } 
  } 
#endif

  while(!LFQ_isEmpty(&((Thr_t)(me))->PubMailBox)) {  
    LFQ_deq(&((Thr_t)(me))->PubMailBox, &srcRqstP);  
    //fprintf(stdout, "AZQ_progress(%p). Dequeued Request %p with SeqNr %d\n", self(), srcRqstP, srcRqstP->SeqNr); fflush(stdout); 
      
    if(RQST_isLocal(srcRqstP)) {  
      if((Thr_t)srcRqstP->Owner == (Thr_t)(me)) { 
        if(RQST_isSplitCopy(srcRqstP)) {  
          Rqst_t dstRqst = srcRqstP->BuffInPtr; 
          MEMCPY(dstRqst->BuffInPtr, srcRqstP->Hdr.Payload, srcRqstP->Hdr.MessageSize>>1); 
          ATOMIC_ADD(&(srcRqstP)->State, (RQST_isSync(srcRqstP) ? 1 : -1 )); 
          ATOMIC_ADD(&dstRqst->State, 1); 
          //fprintf(stdout, "AZQ_progress(%p): Half copy LOW done: src %p with state %x / dst %p with state %x. \n", 
          //               self(), srcRqstP,srcRqstP->State, dstRqst, dstRqst->State);    fflush(stdout);  
          return; 
        } 
        if((srcRqstP->State == RQST_CANCELLED)) {
          if(!RQST_isPersistent(srcRqstP)) { 
            rqstFree(me->RqstTable, &srcRqstP);     
          } 
          continue; 
        } 
      } 
      deliverLocalFromLfq((Thr_t)(me), srcRqstP); 
    } 
    else { 
      deliverRemoteFromLfq((Thr_t)(me), srcRqstP); 
    } 
  } 
  return;
}

/*----------------------------------------------------------------*
 *   Progress package (END)                                       *
 *----------------------------------------------------------------*/





/*----------------------------------------------------------------*
 *   Implementation of private functions                          *
 *----------------------------------------------------------------*/


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
int AZQ_deliver (INET_iovec *iov, int last_frgmt, int *success) {

  Thr_t       dstThr;
  Header_t    hdr;
  int         dummy;
  int         excpn = 0;
  static
  char       *where      = "deliver";


  hdr = (Header_t)(iov->Data);

  DBG_PRNT((stdout, "AZQ_deliver (): Message from [%x, %d] to [%x, %d] and tag %x (mode: %x :%s:) (last_frgmt: %d)\n",
		  hdr->Src.Group, hdr->Src.Rank, hdr->Dst.Group, hdr->Dst.Rank, hdr->Tag, hdr->Mode, 
			(hdr->Mode & MODE_RRV_RQST) ? "MODE_RRV_RQST" : (hdr->Mode & MODE_RRV_ACK) ? "MODE_RRV_ACK" : (hdr->Mode & MODE_RRV_DATA) ? "MODE_RRV_DATA" : "MODE_EAGER", last_frgmt));

  /* 1. Who is the receiver? */
  if(0 > getEndCom(NULL, &(hdr->Dst), &dummy, &dstThr))                        {excpn = COM_E_DEADPART;
	                                                                            goto exception;}
    
  /* 2. Build header from fragment received */
  hdr->Payload     = (char *)(iov->Data) + HEADER_NET_SZ;
  hdr->PayloadSize = iov->Size           - HEADER_NET_SZ;
  if(last_frgmt)
    hdr->Mode |= MODE_LAST_FRAGMENT;

  /* 3. Handle message */
  static int seq = 0;
  DBG_PRNT((stdout, "AZQ_deliver (): (seq_nr: %d) Put in LFQ header: %p  dstThr: %p [%x, %d] Size: %d  Mode: %x\n", 
		   ++seq, hdr, dstThr, hdr->Dst.Group, hdr->Dst.Rank, hdr->PayloadSize, hdr->Mode));
  
  LFQ_enq(&dstThr->PubMailBox, (LFQ_Link_t) hdr);  
  *success = FALSE;
  
  DBG_PRNT((stdout, "AZQ_deliver: (end)\n"));
  
  return(COM_E_OK);

exception:
  XPN_print(excpn);
  return(excpn);
}



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
static int sendRrvRqst (INET_iovec *iov, int dstMchn, Thr_t me, unsigned timeout) {

  int          excpn;
  Header_t     hdr       = (Header_t)iov->Data;
  Header       hdr2      = *hdr;
  static char *where     = "sendRrvRqst";

  DBG_PRNT((stdout, "sendRrvRqst(%p): (start) \n", me));

  iov[0].Data   = (char *)&hdr2;
  iov[1].Data   = (char *)NULL;
  iov[1].Size   = 0;

  if (0 > (excpn = INET_send(iov, dstMchn, COM_PROTOCOL)))                      goto exception;

  DBG_PRNT((stdout, "sendRrvRqst(%p): (end)\n", me));

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

  
  hdr2.Dst         = hdr->Src;
  hdr2.DstMchn     = hdr->SrcMchn;
  hdr2.Src         = hdr->Src; /***/
  hdr2.SrcMchn     = hdr->DstMchn;
  hdr2.Tag         = hdr->Tag;
  hdr2.Mode        = MODE_RRV_ACK | MODE_REMOTE | MODE_LAST_FRAGMENT;
  hdr2.RrvDstAddr  = dstRqst->Buff;

  DBG_PRNT((stdout, "sendRrvAck (%p): Send to [%x, %d] from [%x, %d]  (start)\n", self(), hdr2.Dst.Group, hdr2.Dst.Rank, hdr2.Src.Group, hdr2.Src.Rank));
  
  iov[0].Data   = (char *)&hdr2;
  iov[0].Size   = HEADER_NET_SZ;
  iov[1].Data   = (char *)NULL;
  iov[1].Size   = 0;

  if(0 > (excpn = INET_send(iov, hdr2.DstMchn, COM_PROTOCOL)))                 goto exception;

  DBG_PRNT((stdout, "sendRrvAck (%p): (end)\n", self()));

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


int deliverRemoteFromLfq(Thr_t me, Header_t srcHdr) {

  Rqst_t      dstRqst;
  int         success_local, excpn;
  Header_t    hdr2;
  INET_iovec  iov[2]; 

  DBG_PRNT((stdout, "deliverRemoteFromLfq(%p). Header %p Mode: %s  (start)\n", me, srcHdr, (srcHdr->Mode & MODE_RRV_RQST) ? "MODE_RRV_RQST" : (srcHdr->Mode & MODE_RRV_ACK) ? "MODE_RRV_ACK" : (srcHdr->Mode & MODE_RRV_DATA) ? "MODE_RRV_DATA" : "MODE_EAGER")); 
  DBG_PRNT((stdout, "\t Looking up request in RPQ from [%x, %d] and tag: %x \n", srcHdr->Src.Group, srcHdr->Src.Rank, srcHdr->Tag)); 

  DLQ_findRPQ(&me->RecvPendReg, &dstRqst, srcHdr->Src.Rank, srcHdr->Tag, srcHdr->Mode, &success_local); 
  DBG_PRNT((stdout, "deliverRemoteFromLfq(%p). SUCCESS: %d\n", self(), success_local)); 

  if(success_local) { 
	
	if (srcHdr->Mode & MODE_RRV_RQST) { 
	  DBG_PRNT((stdout, "deliverRemoteFromLfq(%p): MODE_RRV_RQST message. Mark rqst %p for receiving only from [%d] Rag: %d\n", ((Thr_t) dstRqst->Owner), dstRqst, srcHdr->Src.Rank, srcHdr->Tag)); 

	  /* Mark the request to ONLY receive all message fragments from this source (deberia ser solamente para el caso de recibir de ANY) */ 
	  dstRqst->Status.Src  = srcHdr->Src; 
	  dstRqst->Status.Tag  = srcHdr->Tag; 
	  dstRqst->Hdr.Mode   &= ~MODE_ANY; 
	  dstRqst->Hdr.Mode   |= MODE_RRV_DATA; 

	  /* Send "ready-to-receive" message to sender */
	  sendRrvAck(srcHdr, dstRqst);

	  /* Message received. Safely erase it from low-level buffer */
	  DBG_PRNT((stdout, "deliverRemoteFromLfq(%p): INET_recv of message header %p\n", self(), srcHdr)); 
	  INET_recv((char *)srcHdr);
	  
	} 
	else if (srcHdr->Mode & MODE_RRV_ACK) { 
	  /* Remove send request from the queue */ 
	  DBG_PRNT((stdout, "deliverRemoteFromLfq(%p). Founded! Remove %p from RPQ\n", self(), dstRqst)); 
	  RPQ_remove(&me->RecvPendReg, dstRqst); 
	  
	  /* Build message and send data */ 
	  hdr2             = &(dstRqst->Hdr); 
	  hdr2->RrvDstAddr = srcHdr->RrvDstAddr; 
	  hdr2->Mode      &= ~MODE_RRV_ACK; 
	  hdr2->Mode      |= MODE_RRV_DATA; 
	  
	  iov[0].Data     = (char *)hdr2; 
	  iov[0].Size     = HEADER_NET_SZ;
	  iov[1].Data     = (char *)hdr2->Payload; 
	  iov[1].Size     = hdr2->PayloadSize; 

	  DBG_PRNT((stdout, "deliverRemoteFromLfq(%p). Sending DATA to [%x, %d]\n", self(), srcHdr->Src.Group, srcHdr->Src.Rank)); 
	  INET_send(iov, hdr2->DstMchn, COM_PROTOCOL); 

	  /* Send request is satisfied */ 
	  RQST_setState(dstRqst, RQST_SATISFIED);

	  /* Message received, erase it from low-level buffer */ 
	  DBG_PRNT((stdout, "deliverRemoteFromLfq(%p): INET_recv of message header %p\n", self(), srcHdr)); 
	  INET_recv((char *)srcHdr); 
	  
	  return COM_E_OK;

	} 
	else {

	  /* Feed the destination request */
	  DBG_PRNT((stdout, "deliverRemoteFromLfq(%p): Feeding rqst %p with header %p\n", ((Thr_t)me), dstRqst, srcHdr)); 
	  RQST_feed(dstRqst, srcHdr);
	  
	  /* Last fragment arrived completes the destination request */
	  if (srcHdr->Mode & MODE_LAST_FRAGMENT) {
		RPQ_remove(&me->RecvPendReg, dstRqst);
		RQST_setState(dstRqst, RQST_SATISFIED);
		DBG_PRNT((stdout, "deliverRemoteFromLfq(%p): Delete rqst %p from RPQ. SATISFIED \n", ((Thr_t)me), dstRqst)); 
	  } else {
		/* Mark the request to ONLY receive all message fragments from this source */
		if (srcHdr->Mode & MODE_EAGER) { 
		  DBG_PRNT((stdout, "deliverRemoteFromLfq(%p): MODE_EAGER message. Mark rqst %p for receiving only from [%d] Tag: %d\n", ((Thr_t) dstRqst->Owner), dstRqst, srcHdr->Src.Rank, srcHdr->Tag)); 
		  dstRqst->Status.Src  = srcHdr->Src;
		  dstRqst->Status.Tag  = srcHdr->Tag;  
		  dstRqst->Hdr.Mode   &= ~MODE_ANY; 
		  dstRqst->Hdr.Mode   |= MODE_EAGER; 
		}
	  }
	  
	  /* Message received, erase it from low-level buffer */
	  DBG_PRNT((stdout, "deliverRemoteFromLfq(%p): INET_recv of message header %p\n", self(), srcHdr)); 
	  INET_recv((char *)(srcHdr));
	}
	
  } else {
	/* Receiver not waiting. Put message in mailbox */
	DBG_PRNT((stdout, "deliverRemoteFromLfq(%p): Put rqst %p in MBX \n", ((Thr_t)me), srcHdr)); 
	MBX_put(&(me)->MailBox, srcHdr);
  }
  
  DBG_PRNT((stdout, "deliverRemoteFromLfq(%p). (end) \n", ((Thr_t)me))); 

  return 0;
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
int send_remote (Rqst_t rqst) {

  INET_iovec    iov[2];
  Thr_t         me         = self();
  register
  Header_t      hdr;
  int           excpn;
  static
  char         *where      = "send_remote";


  DBG_PRNT((stdout, "send_remote(%p): Rqst %p (start)\n", me, rqst));

  hdr  = &(rqst->Hdr);

  /* 1. Build the message. Header */
  iov[0].Data  = (char *)hdr;
  iov[0].Size  = HEADER_NET_SZ;

  /* 2. Large message. Send it using the Remote RendezVous protocol */
  if (hdr->Mode & MODE_RRV_RQST) {
	
	DBG_PRNT((stdout, "send_remote(%p): RRV send. Putting rqst %p in RPQ %p\n", me, rqst, me->RecvPendReg));
	
    if (0 > (excpn = sendRrvRqst(iov, hdr->DstMchn, me, 0)))                    goto exception;
	
	/* Next to receive: ACK */
    hdr->Mode &= ~MODE_RRV_RQST;
	hdr->Mode |=  MODE_RRV_ACK;
	
	RPQ_put(&me->RecvPendReg, rqst);

	DBG_PRNT((stdout, "send_remote(%p): (end)\n", me));
	return COM_E_OK;

  /* 3. Short message. Send it using Eager (default) protocol */
  } else {
	DBG_PRNT((stdout, "send_remote(%p): Eager send. \n", me));
  }

  /* 4. Build data */
  iov[1].Data     = (char *)hdr->Payload;
  iov[1].Size     = hdr->PayloadSize;

  /* 5. Send */
  if(0 > (excpn = INET_send(iov, hdr->DstMchn, COM_PROTOCOL)))                  goto exception;

  RQST_setState(rqst, RQST_SATISFIED);

  DBG_PRNT((stdout, "send_remote(%p): (end)\n", me));
  
  return COM_E_OK;

exception:
  XPN_print(excpn);
  return(excpn);
}



      /*________________________________________________________________
     /                                                                  \
    |    AZQ_getFromMBX                                                  |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int AZQ_getFromMBX(Rqst_t dstRqst)  
{
  int          success, excpn;  
  Header_t     srcHdr;
  
  DBG_PRNT((stdout, "\nAZQ_getFromMBX(%p): Begin\n", ((Thr_t) dstRqst->Owner)));
  
  while(1) { 
    /* Is there a message in MBX matching the request ? */
    DBG_PRNT((stdout, "AZQ_getFromMBX(%p): Looking up in MBX ...\n", ((Thr_t) dstRqst->Owner)));
    MBX_lookUp(&((Thr_t)(dstRqst->Owner))->MailBox, &srcHdr, dstRqst->Status.Src.Rank, dstRqst->Status.Tag, dstRqst->Hdr.Mode, &success);
    if(success) {
      DBG_PRNT((stdout, "AZQ_getFromMBX(%p): Found message in MBX %p (%s) mode: %x\n", 
				((Thr_t) dstRqst->Owner), srcHdr, HDR_isLocal(srcHdr) ? "LOCAL" : "REMOTE", srcHdr->Mode));
	  
      /* Handle found message */
      if (HDR_isLocal(srcHdr)) {
        Rqst_t  srcRqst = (Rqst_t)srcHdr;
        Thr_t   srcThr  = (Thr_t)(srcRqst)->Owner;

        if( srcRqst->SeqNr == (1 + ((Thr_t) dstRqst->Owner)->LastRecvSeqNr[srcThr->LocalRank] ) && 
		   ++((Thr_t) dstRqst->Owner)->LastRecvSeqNr[srcThr->LocalRank])  /* Always TRUE */
        {
          int number = 0;

          DLQ_delete(&((Thr_t)(dstRqst->Owner))->MailBox, srcHdr);
          if((srcRqst->Type & RQST_PROBE)) {
            //printf("p");
            if(RQST_isSendBufferSwitched(srcRqst)) {
              //printf(".p.");
            } 
          }
          else {
            if(RQST_isAsync(srcRqst)) {
              int alreadyCancelled; 
              makeUncancellable(srcRqst, &alreadyCancelled);  
              if(alreadyCancelled) { 
                if(!RQST_isPersistent(srcRqst)) {
                  LFQ_t   lfq    =  &srcThr->PubMailBox;
                  LFQ_enq(lfq, (LFQ_Link_t)(srcRqst));
                } 
                else
                  srcRqst->PersistentWait = FALSE; /* The cancelled source request is not now in any queue of the receiver,
                                                      so it can be reused */
                continue;
              }
            } 
            else {
              number = ATOMIC_ADD(&srcRqst->SendCounter, 1);
              switch(number) {
                case 1:
                  break;
                case 2:
                  //printf("บบ2บบ");
                  while(!RQST_isSendBufferSwitched(srcRqst));
                  break;
                default:
                  panic("AZQ_getFromMBX");
              }
            }
          }
		  
          if(srcRqst->Hdr.MessageSize >= SPLIT_COPY_THRESHOLD && (RQST_isWaiting(srcRqst) /*|| RQST_isSync(srcRqst) */)) { 
            Thr_t   sender      = (Thr_t)srcRqst->Owner; 
            LFQ_t   lfq         = &sender->PubMailBox; 
			
            srcRqst->BuffInPtr = dstRqst; 
            RQST_setSplitCopy(srcRqst);
            LFQ_enq(lfq, (LFQ_Link_t)srcRqst); 
            RQST_halfFeedLocal(dstRqst, srcHdr->Payload, srcHdr->MessageSize, srcHdr->Src.Rank, srcHdr->Tag); 
            ATOMIC_ADD(&srcRqst->State, (RQST_isSync(srcRqst) ? 1 : -1)); 
            ATOMIC_ADD(&dstRqst->State, 1); 
          } 
          else {
            if(srcRqst->Hdr.MessageSize > dstRqst->BuffSize) panic("Too big Message");
            RQST_feedLocal(dstRqst, srcHdr);  
            RQST_setState(srcRqst, RQST_SATISFIED);
            RQST_setState(dstRqst, RQST_SATISFIED);
            DBG_PRNT((stdout, "AZQ_getFromMBX(%p): End\n", self()));
          }
          wr_barrier();

          if(number == 2) {
            if((srcRqst)->SendBuffer)
              free(srcRqst->SendBuffer); 
            rqstFree(srcThr->RqstTable, &srcRqst); 
          } 
          return 1;
        }
      }
	  
      else  { /* Remote message */
	
        Thr_t me = ((Thr_t)dstRqst->Owner);//self();
        if (srcHdr->Mode & MODE_RRV_RQST) {		  
          DBG_PRNT((stdout, "AZQ_getFromMBX(%p): RRV_RQST message. Mark rqst %p as ACKED\n", ((Thr_t) dstRqst->Owner), dstRqst));
          /* Mark the request to ONLY receive all message fragments from this source 
		   (deberia ser solamente para el caso de recibir de ANY) */
          dstRqst->Status.Src  =  srcHdr->Src;
          dstRqst->Status.Tag  =  srcHdr->Tag;
          dstRqst->Hdr.Mode   &= ~MODE_ANY;
          dstRqst->Hdr.Mode   |=  MODE_RRV_DATA;
		  
          /* Put the request pending in RPQ */
          DBG_PRNT((stdout, "AZQ_getFromMBX(%p): Put rqst %p in RPQ, delete from MBX \n", ((Thr_t)me), dstRqst));
          DLQ_put(&(me)->RecvPendReg, dstRqst);
          DLQ_delete(&(me)->MailBox, srcHdr);
		  
          /* Send "ready-to-receive" message to sender */
          if (0 > (excpn = sendRrvAck(srcHdr, dstRqst)))                                    goto exception;
		  
          /* Message received. Safely erase it from low-level buffer */
          DBG_PRNT((stdout, "AZQ_getFromMBX(%p): INET_recv of message header %p\n", ((Thr_t)me), srcHdr));
          INET_recv((char *)srcHdr);		  		  
          return 1;  
        } 
        else {
          /* Mark the request to ONLY receive all message fragments from this source */
          if (srcHdr->Mode & MODE_EAGER) {
            DBG_PRNT((stdout, "AZQ_getFromMBX(%p): MODE_EAGER message. Mark rqst %p for receiving only from [%d] Rag: %d\n", 
					  ((Thr_t) dstRqst->Owner), dstRqst, srcHdr->Src.Rank, srcHdr->Tag));
            dstRqst->Status.Src   =  srcHdr->Src;
            dstRqst->Status.Tag   =  srcHdr->Tag;
            dstRqst->Hdr.Mode    &= ~MODE_ANY;
            dstRqst->Hdr.Mode    |=  MODE_EAGER;
          }
		  
          /* Feed the destination request */
          DBG_PRNT((stdout, "AZQ_getFromMBX(%p): Feeding rqst %p with message %p\n", ((Thr_t)me), dstRqst, srcHdr));
          RQST_feed(dstRqst, srcHdr);
		  
          /* Message received. Safely erase it from low-level buffer */
          DBG_PRNT((stdout, "AZQ_getFromMBX(%p): Delete header %p from MBX \n", ((Thr_t)me), srcHdr));
          DLQ_delete(&me->MailBox, srcHdr);
		  
          DBG_PRNT((stdout, "AZQ_getFromMBX(%p): INET_recv of message header %p\n", ((Thr_t)me), srcHdr));
          INET_recv((char *)srcHdr);
		  
          /* Last fragment arrived completes the destination request */
          if (srcHdr->Mode & MODE_LAST_FRAGMENT) {
            DBG_PRNT((stdout, "AZQ_getFromMBX(%p): rqst %p SATISFIED \n", me, dstRqst));
            RQST_setState(dstRqst, RQST_SATISFIED);
            return 1;
          }
          continue;
        }	
      }
	  
    }
    DBG_PRNT((stdout, "AZQ_getFromMBX(%p): End\n", (Thr_t) dstRqst->Owner));
    return 0;
  }
  
  panic("AZQ_getFromMBX");  /* Should never get here */
  
exception:
  fprintf(stdout, "AZQ_getFromMBX(%p): Error. END\n"); fflush(stdout);
  return -1;
}




       /*________________________________________________________________
      /                                                                  \
     |    AZQ_testFromMBX                                                 |
     |                                                                    |
      \____________/  ___________________________________________________/
                  / _/
                 /_/ 
                */
int AZQ_testFromMBX(Rqst_t  dstRqst, RQST_Status_t status)
{
  int          success, number, excpn;
  Header_t     srcHdr;
  
  DBG_PRNT((stdout, "\nAZQ_testFromMBX(%p): Begin\n", ((Thr_t) dstRqst->Owner)));
  
  while(1) { 
    /* Is there a message in MBX matching the request ? */
    DBG_PRNT((stdout, "AZQ_testFromMBX(%p): Looking up in MBX ...\n", ((Thr_t) dstRqst->Owner)));	
    MBX_lookUp (&((Thr_t)(dstRqst->Owner))->MailBox, &srcHdr, dstRqst->Status.Src.Rank, 
				dstRqst->Status.Tag, dstRqst->Hdr.Mode, &success);
    if(success) {
      DBG_PRNT((stdout, "AZQ_testFromMBX(%p): Found message in MBX %p (%s) mode: %x\n", 
        ((Thr_t) dstRqst->Owner), srcHdr, HDR_isLocal(srcHdr) ? "LOCAL" : "REMOTE", srcHdr->Mode));
	  
      /* Handle found message */
      if (HDR_isLocal(srcHdr)) {
        Rqst_t  srcRqst = (Rqst_t)srcHdr;
        Thr_t   srcThr  = (Thr_t)(srcRqst)->Owner;
        if( srcRqst->SeqNr == (1 + ((Thr_t) dstRqst->Owner)->LastRecvSeqNr[srcThr->LocalRank] ))
        {
          if(RQST_isAsync(srcRqst)) {
            int alreadyCancelled; 
            makeUncancellable(srcRqst, &alreadyCancelled);  
            if(alreadyCancelled) { 
              ((Thr_t) dstRqst->Owner)->LastRecvSeqNr[srcThr->LocalRank]++;
              DLQ_delete(&((Thr_t)(dstRqst->Owner))->MailBox, srcHdr);
              if(!RQST_isPersistent(srcRqst)) {
                LFQ_t   lfq    =  &srcThr->PubMailBox;
                LFQ_enq(lfq, (LFQ_Link_t)(srcRqst));
              } 
              else
                srcRqst->PersistentWait = FALSE; /* The cancelled source request is not now in any queue of the receiver,
                                                    so it can be reused */
              continue;
            }
          }
          else {
            number = ATOMIC_ADD(&srcRqst->SendCounter, 1);
            switch(number) {
              case 1:
                  break;
              case 2:
                  //printf("บ2บ");
                  while(!RQST_isSendBufferSwitched(srcRqst)) printf("w");
                  break;
              default:
                  panic("AZQ_testFromMBX: Bad Number");
            }
          }
          if(status != AZQ_STATUS_IGNORE) {
            status->Src.Rank = srcHdr->Src.Rank;
            status->Tag      = srcHdr->Tag;
            status->Count    = srcHdr->MessageSize;
          }
          srcRqst->Type |= RQST_PROBE;
          return 1;
        }
      }  
      else  { /* Remote message */
      }
    }
    DBG_PRNT((stdout, "AZQ_testFromMBX(%p): End\n", (Thr_t) dstRqst->Owner));
    return 0;
  }
  panic("AZQ_testFromMBX");  /* Should never get here */
  
exception:
  return -1;
}



      /*________________________________________________________________
     /                                                                  \
    |    COM_init                                                        |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int COM_init()
{
  int            excpn;
  static char   *where     = "COM_init";

  DBG_PRNT((stdout, "COM_init: \n"));

  if(initialised)
    return(COM_E_OK);

  initialised = TRUE;

  if(0 > (excpn = INET_init(NULL)))                                            goto exception;
  if(0 > (excpn = INET_subscribe(AZQ_deliver, COM_PROTOCOL)))                      goto exception;

  AZQ_cpuId = INET_getCpuId();

  DBG_PRNT((stdout, "COM_init: Subscribed %p for protocol %d. End\n", AZQ_deliver, COM_PROTOCOL));

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
  return AZQ_cpuId;
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


