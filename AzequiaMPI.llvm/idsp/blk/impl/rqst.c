/*-
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
#include <rqst.h>


/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   used by this module                                          *
 *----------------------------------------------------------------*/
#if defined(__OSI)
  #include <osi.h>
#else
  #include <stdio.h>
  #include <string.h>
#endif

#include <xpn.h>
#include <thr_dptr.h>
#include <azq_types.h>
#include <addr_hddn.h>
#include <com.h>

extern pthread_key_t   key;
#define self()         ((Thr_t)pthread_getspecific(key))
extern  void panic     (char *where);

/*----------------------------------------------------------------*
 *   Definition of private macros                                 *
 *----------------------------------------------------------------*/
#ifdef __RQST_DEBUG
#define DBG_PRNT(pmsg)  fprintf pmsg
#else
#define DBG_PRNT(pmsg)
#endif

/*----------------------------------------------------------------*
 *   Definition of private data                                   *
 *----------------------------------------------------------------*/
static const
       char  *e_names[8] = { // This order has to be consistent with com.h
                             /*  0 */ "RQST_E_OK",
                             /*  1 */ "RQST_E_EXHAUST",
                             /*  2 */ "RQST_E_INTEGRITY",
                             /*  3 */ "RQST_E_TIMEOUT",
                             /*  4 */ "RQST_E_INTERFACE",
                             /*  5 */ "RQST_E_SYSTEM",
                             /*  6 */ "RQST_E_SIGNALED",
                             /*  7 */ "RQST_E_DEADPART"
                           };


/*----------------------------------------------------------------*
 *   Implementation of private functions                          *
 *----------------------------------------------------------------*/
#ifdef __RQST_DEBUG

void RQST_printHdr(Header_t hdr) {

  fprintf(stdout, "AZQ_hdr |      Dst.G      Dst.R      Src.G      Src.R    SrcMchn    DstMchn        Tag       Mode Payload  PayloadSize\n");
  fprintf(stdout, "        |  ---------  ---------  ---------  ---------  ---------  ---------  ---------  ---------  ---------  ---------\n");
  fprintf(stdout, "        | %10x %10d %10x %10d %10d %10d %10d %10x %10p %10d\n", hdr->Dst.Group, hdr->Dst.Rank, hdr->Src.Group, hdr->Src.Rank, hdr->SrcMchn, hdr->DstMchn, hdr->Tag, hdr->Mode, hdr->Payload, hdr->PayloadSize);
  return;
}

void rqst_print (Rqst_t rqst) {
  fprintf(stdout, "\tREQUEST: \n");
  fprintf(stdout, "\t\tState:  0x%x\n", rqst->State);
  fprintf(stdout, "\t\tOwner:  %p\n", rqst->Owner);
  fprintf(stdout, "\t\tSrc:    [%d,%d]\n", rqst->Status.Src.Group, rqst->Status.Src.Rank);
  fprintf(stdout, "\t\tTag:    %d\n", rqst->Status.Tag);
  fprintf(stdout, "\t\tCount:  %d (left count %d)\n", rqst->BuffSize, rqst->BuffSize - rqst->Status.Count);
  fprintf(stdout, "\t\tSource THR  %p  RQST  %p: \n", rqst->Hdr.SrcThr, rqst->Hdr.SrcRqst);
  fprintf(stdout, "\t\tType: %x \n", rqst->Type);
}

#endif



/*----------------------------------------------------------------*
 *   Public variables                                             *
 *----------------------------------------------------------------*/
void (*RQST_freeUpCall)(Rqst_t rqst);  /* Function pointer that must be seen by progress.c */


/*----------------------------------------------------------------*
 *   Implementation of public interface                           *
 *----------------------------------------------------------------*/


      /*________________________________________________________________
     /                                                                  \
    |    RQST_feed                                                       |
    |                                                                    |
    |      A fragment of a message, described by "hdr", has arrived      |
    |    that matches the "dstRqst" request. Copy the data of this       |
    |    fragment to the user buffer associated to "dstRqst".            |
    |      Info on the message and its origin is built inside "dstRqst". |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o dstRqst  (Input/Output)                                       |
    |        Target request                                              |
    |    o hdr      (Input/Output)                                       |
    |        message descriptor                                          |
    |                                                                    |
    |    RETURN:                                                         |
    |    = 0  : On success                                               |
    |    <  0 : On error                                                 |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int RQST_feed(Rqst_t rqst, Header_t hdr) {

  int      excpn;
  static
  char    *where    = "RQST_feed";
  bool old_chk_flag;

  DBG_PRNT((stdout, "RQST_feed(%p): (start) Trying to feed %d bytes (rqst left %d bytes)\n", self(), hdr->PayloadSize, rqst->BuffSize - rqst->Status.Count));

  /* 1. Is the user buffer big enough? */
  if((rqst->BuffSize - rqst->Status.Count) < hdr->PayloadSize) {
    //DBG_PRNT
	  fprintf(stderr, "RQST_feed (%p): Receiving buffer not big enough (buffLeft = %d / hdr->PayloadSize = %d)\n",  rqst, rqst->BuffSize - rqst->Status.Count, hdr->PayloadSize);

    //DBG_PRNT
	  fprintf(stderr, "\t\t from [G %d R %d] and tag %d\n\n", hdr->Src.Group, hdr->Src.Rank, hdr->Tag);
    panic("RQST_feed");
    excpn = RQST_E_EXHAUST;
    goto exception;
  }

  /* 2. Short cut: copy direct to user buffer from network */
  if (!(hdr->Mode & MODE_NO_COPY))
  {
	 old_chk_flag = klee_disable_sync_chk(1);
    memcpy(rqst->BuffInPtr, hdr->Payload, hdr->PayloadSize);   
    if(old_chk_flag)
    	klee_enable_sync_chk(1);
  }
  rqst->Status.Count += hdr->PayloadSize;

  /* 3. A remote message come in fragments. Once the first fragment is received,
        all the other fragments copied must be from the same source and tag.
        This avoid merge fragments from different messages if wildcards are used */
  rqst->Status.Src    = hdr->Src;
  rqst->Status.Tag    = hdr->Tag;

  /* 4. Request full */
  if(hdr->Mode & MODE_LAST_FRAGMENT) { 
    DBG_PRNT((stdout, "RQST_feed(%p): Request %p filled (state: %x)!!. End.\n", self(), rqst, rqst->State));
    rqst->Status.SrcMchn = hdr->SrcMchn;
  } else {
    rqst->BuffInPtr += hdr->PayloadSize;
  }

  DBG_PRNT((stdout, "RQST_feed(%p): End \n", self()));

  return(RQST_E_OK);

exception:
  XPN_print(excpn);
  return(excpn);
}



      /*________________________________________________________________
     /                                                                  \
    |    RQST_ready                                                      |
    |                                                                    |
    |    Mark a request as ready                                         |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o dstThr       (Input)                                          |
    |        Thread where is the request                                 |
    |    o rqst         (Output)                                         |
    |        Request                                                     |
    |                                                                    |
    |    RETURN:                                                         |
    |    = TRUE  : if rqst get ready                                     |
    |    = FALSE : if not                                                |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int RQST_ready (void *dstThr, Rqst_t rqst) {

  Thr_t  thr = (Thr_t)dstThr;

  DBG_PRNT((stdout, "RQST_ready(%p): rqst %p has state %x\n", self(), rqst, (rqst)->State));

  if(rqst->State == RQST_MAKES_WAITING) {
    thr->WaitRqstLeftCnt -= 1;

    if(thr->WaitRqstLeftCnt == 0) {
      thr->SatisfiedRqst = rqst->WaitIdx;
      return (TRUE);
    }

  }

  return (FALSE);
}


     /*________________________________________________________________
    /                                                                  \
   |    RQST_registerFree                                               |
   |                                                                    |
   |    Register the rqstFreeUpCall AzequiaMPI upcall                   |
   |                                                                    |
   |    PARAMETERS:                                                     |
   |    o upCall       (Input)                                          |
   |                                                                    |
    \____________/  ___________________________________________________/
                / _/
               /_/
              */
void RQST_registerFree(void (*upCall)(Rqst_t rqst)) {
  
  RQST_freeUpCall = upCall;
  return;
}

