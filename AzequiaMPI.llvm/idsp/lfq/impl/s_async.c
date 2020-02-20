/*-
 * Copyright (c) 2009-2011. Universidad de Extremadura
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

      /*________________________________________________________________
     /                                                                  \
    |    AZQ_asend                                                       |
    |                                                                    |
    |    Send the data in  "buffer"                                      |
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
    |    o rqst     (Output)                                             |
    |        Allows further test and wait, waitany and waitall           |
    |                                                                    |
    |    RETURN:                                                         |
    |    =  0 : On success                                               |
    |    <  0 : On error                                                 |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int AZQ_asend(const int dst, const char *buf, const int cnt, const Tag_t tag, Rqst_t rqst) 
{ 
   Thr_t  me = self(); 
   int    excpn;
   char  *where = "AZQ_asend";

 
   /*fprintf(stdout, "asend(%p): To [%d]. Tag %x. Count: %d. Rqst: %p, BEGIN\n", me, (dst), (tag), (cnt), (rqst)); fflush(stdout); */
   if ((me->GrpInfo)[(dst)].Mchn == AZQ_cpuId) {  
     Thr_t dstThr     = (me->GrpInfo)[(dst)].Thr; 
     int dstLocalRank = THR_getLocalRank(dstThr);
     int msgSeqNr     = ++me->SendSeqNr[dstLocalRank];

#ifdef USE_FASTBOXES
     if( cnt <= FBOX_BUF_MAX ) {
       int srcLocalRank = THR_getLocalRank(me);
       fastBox_t fBox   = &dstThr->FastBox[srcLocalRank];

       if(fBox->Turn == TURN_SEND  ) {
	   memcpy(fBox->Payload, (buf), (cnt));
	   fBox->MessageSize = (cnt);
	   fBox->SeqNr       = msgSeqNr;
	   fBox->Tag         = (tag);
	   (rqst)->State  = RQST_SATISFIED;
	   fBox->Turn        = TURN_RECV;
          return(AZQ_SUCCESS); 
       }
     }
#endif

     RQST_initAsendLocal(rqst, me->Address.Rank, tag, buf, cnt, me, msgSeqNr);
     /*fprintf(stdout, "asend(%p): Put in LFQ...\n", me); fflush(stdout); */ 
     LFQ_enq(&dstThr->PubMailBox, (LFQ_Link_t)rqst);  
   } 
   else {  
     RQST_initASendRemote_2(rqst, dst, tag, buf, cnt, me); 
     send_remote(rqst); 
   } 
   /*fprintf(stdout, "asend(%p): END\n", me); fflush(stdout); */
  return AZQ_SUCCESS;

exception:
  XPN_print(excpn);
  return(azq_err[-excpn]);
}




     /*________________________________________________________________
    /                                                                  \
   |    AZQ_assend                                                      |
   |                                                                    |
   |    Send the data in  "buffer"                                      |
   |                                                                    |
   |    PARAMETERS:                                                     |
   |    o dst  (Input)                                                  |
   |        The desired destination rank                                |
   |    o buf   (Input)                                                 |
   |        User buffer of outgoing data                                |
   |    o cnt    (Input)                                                |
   |        Input:  Size of "buffer"                                    |
   |    o tag      (Input)                                              |
   |        Tag of the outgoing message                                 |
   |    o rqst     (Output)                                             |
   |        Allows further test and wait, waitany and waitall           |
   |                                                                    |
   |    RETURN:                                                         |
   |    =  0 : On success                                               |
   |    <  0 : On error                                                 |
   |                                                                    |
    \____________/  ___________________________________________________/
                / _/
               /_/
              */
int AZQ_assend(const int dst, const char *buf, const int cnt, const Tag_t tag, Rqst_t rqst) 
{ 
  Thr_t  me = self(); 
  int    excpn;
  char  *where = "AZQ_assend";
  
  /*fprintf(stdout, "asend(%p): To [%d]. Tag %x. Count: %d. Rqst: %p, BEGIN\n", me, (dst), (tag), (cnt), (rqst)); fflush(stdout); */
  if ((me->GrpInfo)[(dst)].Mchn == AZQ_cpuId) {  
    Thr_t dstThr     = (me->GrpInfo)[(dst)].Thr; 
    int dstLocalRank = THR_getLocalRank(dstThr); 
    int msgSeqNr     = ++me->SendSeqNr[dstLocalRank]; 
	
    /*fprintf(stdout, "AZQ_assend(%p): SeqNr %d\n", me, msgSeqNr); fflush(stdout); */ 
    RQST_initAsendLocal(rqst, me->Address.Rank, tag, buf, cnt, me, msgSeqNr); 

    /*fprintf(stdout, "AZQ_assend(%p): Put in LFQ...\n", me); fflush(stdout); */ 
    LFQ_enq(&dstThr->PubMailBox, (LFQ_Link_t)rqst);  
  } 
  else {  
    RQST_initSendRemote(rqst, dst, tag, buf, cnt, me); 
    send_remote(rqst); 
  } 
  /*fprintf(stdout, "AZQ_assend(%p): END\n", me); fflush(stdout); */
  return AZQ_SUCCESS;
  
exception:
  XPN_print(excpn);
  return(azq_err[-excpn]);
}


