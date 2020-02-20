/*-
 * Copyright (c) 2009-2010 Universidad de Extremadura
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
#include <azq_types.h>
#include <xpn.h>
#include <addr.h>
#include <inet.h>
#include <thr.h>
#include <thr_dptr.h>
#include <mbx.h>
#include <rqst.h>
#include <util.h>


/*----------------------------------------------------------------*
 *   Definition of private constants                              *
 *----------------------------------------------------------------*/
static const
       char  *e_names[8] = { /* This order has to be consistent with com.h */
                             /*  0 */ "COM_E_OK",
                             /*  1 */ "COM_E_EXHAUST",
                             /*  2 */ "COM_E_INTEGRITY",
                             /*  3 */ "COM_E_TIMEOUT",
                             /*  4 */ "COM_E_INTERFACE",
                             /*  5 */ "COM_E_SYSTEM",
                             /*  6 */ "COM_E_SIGNALED",
                             /*  7 */ "COM_E_DEADPART"
                           };


extern  int           (*getEndCom)(void *srcThr, Addr_t dst, int *mchn, void *thr);
#define self()        ((Thr_t)pthread_getspecific(key))

/*----------------------------------------------------------------*
 *   Declaration of external functions                            *
 *----------------------------------------------------------------*/
void AZQ_deal_recv (Rqst_t dstRqst);


/*----------------------------------------------------------------*
 *   Implementation of public interface                           *
 *----------------------------------------------------------------*/

      /*________________________________________________________________
     /                                                                  \
    |    AZQ_psend_init                                                  |
    |                                                                    |
    |    Create a persistent communiction request                        |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o dstAddr  (Input)                                              |
    |        The desired destination address                             |
    |    o buffer   (Input)                                              |
    |        User buffer of outgoing data                                |
    |    o count    (Input)                                              |
    |        Input:  Size of "buffer"                                    |
    |    o tag      (Input)                                              |
    |        Tag of the outgoing message                                 |
    |    o rqst     (Input/Output)                                       |
    |        Allow persistent. Completed with test/wait or deleted       |
    |        with free_request                                           |
    |                                                                    |
    |    RETURN:                                                         |
    |    = 0 : On success                                                |
    |    < 0 : On error                                                  |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int AZQ_psend_init (const int dstRank, char *buff, int cnt, int tag, int s_mode, Rqst_t rqst) 
{
  Thr_t    dstThr;
  int      dstMchn;
  Thr_t    me             = self();
  int      excpn          = AZQ_SUCCESS;
  Addr     dst;
  static
  char    *where          = "AZQ_psend_init";
  
  
  DBG_PRNT((stdout, "AZQ_psend_init(%p): To [%d]. Tag %x. Count: %d., BEGIN\n", me, dstRank, tag, cnt));  fflush(stdout);
  
  if(rqst == (Rqst_t)AZQ_RQST_NULL)                                            {excpn = COM_E_INTEGRITY;
	goto exception;}
  dst.Rank  = dstRank;
  dst.Group = me->Address.Group;
  if(0 > getEndCom(me, &dst, &dstMchn, &dstThr))                                {excpn = COM_E_INTERFACE;
	goto exception;}
  
  RQST_initSendPersistent(rqst, RQST_SEND | RQST_ASYNC | RQST_PERSISTENT,
						  &dst,
						  &me->Address,
						  tag,
						  buff,
						  cnt,
						  dstMchn,
						  me,
						  dstThr);
  
  DBG_PRNT((stdout, "AZQ_psend_init(%p):  END\n", self()));  fflush(stdout);
  
  return AZQ_SUCCESS;
  
exception:
  XPN_print(excpn);
  return(azq_err[-excpn]);
}



      /*________________________________________________________________
     /                                                                  \
    |    AZQ_psend_start                                                 |
    |                                                                    |
    |    Send the data in  "buffer"                                      |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o rqst     (Input)                                              |
    |        Persistent request filled in init_send                      |
    |                                                                    |
    |    RETURN:                                                         |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int AZQ_psend_start (Rqst_t rqst) {
  
  int           excpn; 
  static char  *where          = " AZQ_psend_start";
  
  DBG_PRNT((stdout, "AZQ_psend_start(%p): Rqst: %p, BEGIN\n", self(), rqst));  
  
  if (!RQST_isPersistent(rqst))                                                {excpn = COM_E_INTERFACE;
	goto exception;}
  if (!RQST_isAsync(rqst))                                                     {excpn = COM_E_INTERFACE;
	goto exception;}
  
  if(RQST_isLocal(rqst)) {
    Thr_t dstThr       = (Thr_t)rqst->Hdr.DstThr;
    int   dstLocalRank = THR_getLocalRank(dstThr); 
    Thr_t me           = (Thr_t)(rqst->Owner);
    int   msgSeqNr     = ++me->SendSeqNr[dstLocalRank]; 
    int   count        = rqst->Hdr.PayloadSize;
    RQST_setSeqNr(rqst, msgSeqNr);
#ifdef USE_FASTBOXES
    if(count <= FBOX_BUF_MAX) {  
      int        srcLocalRank = THR_getLocalRank(me); 
      fastBox_t  fBox         = &dstThr->FastBox[srcLocalRank]; 
      if(fBox->Turn == TURN_SEND) { 
        void *buff  = rqst->Hdr.Payload;
        int   tag   = rqst->Hdr.Tag;
        memcpy(fBox->Payload, buff, count);  
        fBox->SeqNr       = msgSeqNr; 
        fBox->MessageSize = count; 
        fBox->Tag         = tag; 
        fBox->Turn        = TURN_RECV; 
        RQST_setSatisfied(rqst);    
        return(AZQ_SUCCESS); 
      } 
    } 
#endif
    RQST_setState(rqst, RQST_PENDING);
    LFQ_enq(&dstThr->PubMailBox, ((LFQ_Link_t)rqst));                              
  }
  else {
    if (0 > (excpn = send_remote (rqst, 0)))                                    goto exception;
  }
  
  DBG_PRNT((stdout, "AZQ_psend_start(%p): END\n", self()));  
  
  return(AZQ_SUCCESS);
  
exception:
  XPN_print(excpn);
  return(azq_err[-excpn]);
}


      /*________________________________________________________________
     /                                                                  \
    |    AZQ_precv_init                                                  |
    |    Create a persistent receive communication request               |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o srcAddr  (Input)                                              |
    |        The desired source address                                  |
    |    o buffer   (Input)                                              |
    |        User buffer to store the incomming data                     |
    |    o count    (Input)                                              |
    |        Input:  Available space in "buffer" to receive the message  |
    |    o mode     (Input)                                              |
    |        Determines if reception is synchronous or asynchronous      |
    |    o tag      (Input)                                              |
    |        Desired Tag in the incoming message                         |
    |    o rqst     (Output)                                             |
    |        Relevant only to async receive requests.                    |
    |        Allows further testing (GC_test) or waiting (GC_wait)       |
    |    o timeout  (Input)                                              |
    |        Relative timeout (in milliseconds)                          |
    |                                                                    |
    |    RETURN:                                                         |
    |    = 0 : On success                                                |
    |    < 0 : On error                                                  |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int AZQ_precv_init(const int srcRank, char *buf, int cnt, Tag_t tag, Rqst_t rqst)  
{
  Thr_t      me         = self();
  int        excpn      = AZQ_SUCCESS;
  static
  char      *where      = "AZQ_precv_init";
  
  
  DBG_PRNT((stdout, "AZQ_precv_init(%p): From [%d]. Tag %x. Count: %d., BEGIN\n", me, srcRank, tag, cnt));
  
  if (rqst == (Rqst_t)AZQ_RQST_NULL)                                           {excpn = COM_E_INTEGRITY;
	goto exception;}
  
  /* 1. Init */
  RQST_initRecvPersistent(rqst, RQST_RECV | RQST_ASYNC | RQST_PERSISTENT,
						  srcRank,
						  &me->Address,
						  tag,
						  buf,
						  cnt,
						  DFLT_MCHN,
						  me,
						  NULL);
  
  DBG_PRNT((stdout, "AZQ_precv_init(%p): End\n", me));
  
  return AZQ_SUCCESS;
  
exception:
  XPN_print(excpn);
  return(azq_err[-excpn]);
}



      /*________________________________________________________________
     /                                                                  \
    |    AZQ_precv_start                                                 |
    |    Receive with a persistent receive communication request         |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o rqst     (Input)                                              |
    |        Persistent receive communication request filled in          |
    |        init_recv                                                   |
    |                                                                    |
    |    RETURN:                                                         |
    |    = 0 : On success                                                |
    |    < 0 : On error                                                  |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int AZQ_precv_start (Rqst_t rqst)
{
  Thr_t      me         = (Thr_t)rqst->Owner;
  int        done;
  int        excpn      = AZQ_SUCCESS;
  static
  char      *where      = "AZQ_precv_start";
  
  DBG_PRNT((stdout, "AZQ_precv_start(%p): Rqst: %p, BEGIN\n", self(), rqst));
  
  if(!RQST_isPersistent(rqst))                                                 {excpn =	COM_E_INTERFACE;
                                                                                goto exception;}
  rqst->Status.Cancelled = 0;
  
#ifdef USE_FASTBOXES
  if( (rqst->BuffSize) <= FBOX_BUF_MAX) {
	recvFromFbox(me, rqst->RankPrst, rqst->Buff, rqst->BuffSize, rqst->TagPrst, &rqst->Status,
				 &done);
	if(done) {
	  RQST_setSatisfied(rqst);
	  return AZQ_SUCCESS;
	}
  }
#endif
  rqst->Status.Src.Rank = rqst->RankPrst;
  rqst->Status.Tag      = rqst->TagPrst;
  RQST_setState(rqst, RQST_PENDING);
  DLQ_put(&me->RecvPendReg, rqst);
  
  DBG_PRNT((stdout, "AZQ_precv_start (%p): END\n", self()));
  return AZQ_SUCCESS;
  
exception:
  XPN_print(excpn);
  return(azq_err[-excpn]);
}

