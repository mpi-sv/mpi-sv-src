/*-
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

#include <azq_types.h>
#include <xpn.h>
#include <addr.h>
#include <inet.h>
#include <thr.h>
#include <thr_dptr.h>
#include <mbx.h>
#include <rqst.h>
#include <util.h>
#include <string.h>
#include <p_rqst.h>
#include <stdlib.h>



/*----------------------------------------------------------------*
 *   Definition of private constants                              *
 *----------------------------------------------------------------*/
#define RECV_WAIT_MAX 1000000

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


/*----------------------------------------------------------------*
 *   Declaration private functions                                *
 *----------------------------------------------------------------*/
static inline int sWitchSendBuffer(Rqst_t rqst)
{
  if(posix_memalign(&rqst->SendBuffer, CACHE_LINE_SIZE, rqst->Hdr.MessageSize))
    return -1;
  memcpy(rqst->SendBuffer, rqst->Hdr.Payload, rqst->Hdr.MessageSize);
  rqst->Hdr.Payload = rqst->SendBuffer;
  RQST_setSendBufferSwitched(rqst); 
  return 0;
}


static inline int switchSendBuffer(Rqst_t rqst)
{
  if(rqst->Hdr.MessageSize <= 1024) {
    rqst->SendBuffer = NULL;
    memcpy(&rqst->CarriedData, rqst->Hdr.Payload, rqst->Hdr.MessageSize);
    rqst->Hdr.Payload = &rqst->CarriedData;
    //printf("#%d ", rqst->Hdr.MessageSize);
  }
  else {
    if(posix_memalign(&rqst->SendBuffer, CACHE_LINE_SIZE, rqst->Hdr.MessageSize))      return -1;
    memcpy(rqst->SendBuffer, rqst->Hdr.Payload, rqst->Hdr.MessageSize);
    rqst->Hdr.Payload = rqst->SendBuffer;
  }
  RQST_setSendBufferSwitched(rqst); 
  return 0;
}


/*----------------------------------------------------------------*
 *   implementation of public function interface                  *
 *----------------------------------------------------------------*/

     /*________________________________________________________________
     /                                                                  \
    |    AZQ_sendLocalZero                                               |
    |                                                                    |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int AZQ_sendLocalZero(const int dst, const Tag_t tag) 
{ 
  Thr_t  me = self();
  Rqst_t rqst = &me->SyncRqst;
 
  Thr_t dstThr     = (me->GrpInfo)[(dst)].Thr; 
  int dstLocalRank = THR_getLocalRank(dstThr); 
  int msgSeqNr     = ++me->SendSeqNr[dstLocalRank]; 

#ifdef USE_FASTBOXES
  int srcLocalRank = THR_getLocalRank(me); 
  fastBox_t fBox   = &dstThr->FastBox[srcLocalRank]; 

  if(fBox->Turn == TURN_SEND  ) { 
    fBox->MessageSize = 0; 
    fBox->SeqNr       = msgSeqNr; 
    fBox->Tag         = tag; 
    fBox->Turn        = TURN_RECV; 
    return(AZQ_SUCCESS); 
  }
#endif

  RQST_initSendLocal(rqst, me->Address.Rank, tag, NULL, 0, me, msgSeqNr);
  LFQ_enq(&dstThr->PubMailBox, (LFQ_Link_t)rqst); 
  while (!RQST_isSatisfied(rqst));
  return(AZQ_SUCCESS);
}


      /*________________________________________________________________
     /                                                                  \
    |    AZQ_send                                                        |
    |                                                                    |
    |    Send the data in  "buff"                                        |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o dstAddr  (Input)                                              |
    |        The desired destination address                             |
    |    o buff     (Input)                                              |
    |        User buffer of outgoing data                                |
    |    o cnt    (Input)                                                |
    |        Input:  Size of "buffer"                                    |
    |    o tag      (Input)                                              |
    |        Tag of the outgoing message                                 |
    |    o timeout  (Input)                                              |
    |        Relative timeout (in milliseconds)                          |
    |                                                                    |
    |    RETURN:                                                         |
    |    0    : On success                                               |
    |    <  0 : On error                                                 |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int AZQ_send(const int dst, const char *buff, const int cnt, const Tag_t tag) 
{ 
  const Thr_t  me = self();
  int    number;
  const Rqst_t rqst;
  int    recvWaitMax;
 
  //fprintf(stdout, "\nsend(%p): To [%d]. Tag %x. Count: %d. Rqst: %p, BEGIN\n", me, (dst), (tag), (cnt), &me->SyncRqst); //fflush(stdout);
  if ((me->GrpInfo)[(dst)].Mchn == AZQ_cpuId) {  
    Thr_t dstThr     = (me->GrpInfo)[(dst)].Thr; 
    int dstLocalRank = THR_getLocalRank(dstThr); 
    int msgSeqNr     = ++me->SendSeqNr[dstLocalRank]; 
    

#ifdef USE_FASTBOXES
    if( cnt <= FBOX_BUF_MAX) {  
      int       srcLocalRank = THR_getLocalRank(me); 
      fastBox_t fBox         = &dstThr->FastBox[srcLocalRank]; 

      if(fBox->Turn == TURN_SEND  ) { 
        memcpy(fBox->Payload, (buff), (cnt));  
        fBox->MessageSize = (cnt); 
        fBox->SeqNr       = msgSeqNr; 
        fBox->Tag         = (tag); 
        fBox->Turn        = TURN_RECV; 
        //fprintf(stdout, "send(%p): delivered in fastBox %p. END\n", me, fBox); fflush(stdout); 
        return(AZQ_SUCCESS); 
      }
    } 
#endif
    rqstAlloc(me->RqstTable, &rqst, 0, 0);
    RQST_initSendLocal(rqst, me->Address.Rank, tag, buff, cnt, me, msgSeqNr);
    /*fprintf(stdout, "send(%p): Put in LFQ... \n", me); fflush(stdout); */
    LFQ_enq(&dstThr->PubMailBox, (LFQ_Link_t)rqst); 
  } 
  else { 
    RQST_initSendRemote(rqst, dst, tag, buff, cnt, me);
    send_remote(rqst); 
  }
  
  recvWaitMax = 1000 + cnt;
  RQST_setWaiting(rqst); 
  while ((!RQST_isSatisfied(rqst)) && (recvWaitMax-- > 0)) AZQ_progress(me);
  RQST_unsetWaiting(rqst);
  
  number = ATOMIC_ADD(&rqst->SendCounter, 1);
  switch(number) {
    case 1:
      switchSendBuffer(rqst);
      break; 
    case 2: 
      while (!RQST_isSatisfied(rqst)); 
      rqstFree(me->RqstTable, &rqst);
      break;
    default:
      panic("AZQ_send: Bad number");
  }
  //fprintf(stdout, "send(%p): END\n", me); fflush(stdout); 

  return(AZQ_SUCCESS);
}


 



      /*________________________________________________________________
     /                                                                  \
    |    AZQ_ssend                                                       |
    |                                                                    |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
/*
int AZQ_ssend_old(const int dst, const char *buff, const int cnt, const Tag_t tag) 
{ 
  Thr_t  me = self();
  Rqst_t rqst;
 
//fprintf(stdout, "\nsend(%p): To [%d]. Tag %x. Count: %d. Rqst: %p, BEGIN\n", me, (dst), (tag), (cnt), &me->SyncRqst); //fflush(stdout);

  if ((me->GrpInfo)[(dst)].Mchn == AZQ_cpuId) {  
    Thr_t dstThr     = (me->GrpInfo)[(dst)].Thr; 
    rqstAlloc(me->RqstTable, &rqst, 0, 0);
#ifdef USE_FASTBOXES
    int dstLocalRank = THR_getLocalRank(dstThr); 
    int msgSeqNr     = ++me->SendSeqNr[dstLocalRank]; 
    RQST_initSendLocal(rqst, me->Address.Rank, tag, buff, cnt, me, msgSeqNr);
#else
    RQST_initAsendLocal(rqst, me->Address.Rank, tag, buff, cnt, me);
#endif
    LFQ_enq(&dstThr->PubMailBox, (LFQ_Link_t)rqst); 
  } 
  else { 
    RQST_initSendRemote(rqst, dst, tag, buff, cnt, me);
    send_remote(rqst); 
  }

  RQST_setWaiting(rqst); 
  while (!RQST_isSatisfied(rqst))  AZQ_progress(me);
  RQST_unsetWaiting(rqst);

  ATOMIC_ADD(&rqst->SendCounter, 1);
  rqstFree(me->RqstTable, &rqst);

//fprintf(stdout, "send(%p): END\n", me); fflush(stdout); 
  return(AZQ_SUCCESS);
}
*/



int AZQ_ssend(const int dst, const char *buff, const int cnt, const Tag_t tag) 
{ 
  Thr_t  me = self();
  Rqst_t rqst;
 
//fprintf(stdout, "\nsend(%p): To [%d]. Tag %x. Count: %d. Rqst: %p, BEGIN\n", me, (dst), (tag), (cnt), &me->SyncRqst); //fflush(stdout);

  if ((me->GrpInfo)[(dst)].Mchn == AZQ_cpuId) {  
    Thr_t dstThr     = (me->GrpInfo)[(dst)].Thr; 
    rqstAlloc(me->RqstTable, &rqst, 0, 0);
    int dstLocalRank = THR_getLocalRank(dstThr); 
    int msgSeqNr     = ++me->SendSeqNr[dstLocalRank]; 
    RQST_initSendLocal(rqst, me->Address.Rank, tag, buff, cnt, me, msgSeqNr);
    LFQ_enq(&dstThr->PubMailBox, (LFQ_Link_t)rqst); 
  } 
  else { 
    RQST_initSendRemote(rqst, dst, tag, buff, cnt, me);
    send_remote(rqst); 
  }

  RQST_setWaiting(rqst); 
  while (!RQST_isSatisfied(rqst))  AZQ_progress(me);
  RQST_unsetWaiting(rqst);

  ATOMIC_ADD(&rqst->SendCounter, 1);
  rqstFree(me->RqstTable, &rqst);

//fprintf(stdout, "send(%p): END\n", me); fflush(stdout); 
  return(AZQ_SUCCESS);
}

