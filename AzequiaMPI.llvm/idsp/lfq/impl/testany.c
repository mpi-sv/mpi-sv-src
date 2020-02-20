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


/*----------------------------------------------------------------*
 *   Declaration of external functions                            *
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------*
 *   Implementation of public interface                           *
 *----------------------------------------------------------------*/

 /*----------------------------------------------------------------*
 *   Private functions                                            *
 *----------------------------------------------------------------*/
#define SLIDE_MAX 32
#define BITS_PER_SLIDE (8 * sizeof(unsigned long long))

static inline int rqst2slide(int i)
{
  return i/BITS_PER_SLIDE;
}

static inline int rqst2bit(int r)
{  
  return r%BITS_PER_SLIDE;
}

static inline int bitsOfSlide(int slide, int slideNr, int count)
{
  int bits;
  if(slide == (slideNr - 1)) {
    if(count % BITS_PER_SLIDE == 0)
      bits = BITS_PER_SLIDE;
    else
      bits = count % BITS_PER_SLIDE;
  }
  else
    bits = BITS_PER_SLIDE;

  return bits;
}

static inline int allDone(unsigned long long *finished, int slideNr, int count) 
{
  int                 slide;
  unsigned long long  fullSlide;
  int                 bits;
  int                 done       = 1;
  int                 slideDone;
 
  for(slide = 0; slide < slideNr; slide++) {
    bits      = bitsOfSlide(slide, slideNr, count);
    if(bits == BITS_PER_SLIDE)          
      fullSlide = -1;
    else
      fullSlide = ((unsigned long long)1 << bits) - 1;
    slideDone = (finished[slide] == fullSlide);
    done *= slideDone;
  }
  return done;
}


      /*________________________________________________________________
     /                                                                  \
    |    testany                                                        |
    |    Test if some async request has finished                         |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o rqst (Input)                                                  |
    |        The tested asynchronous array of requests previously done   |
    |    o incnt (Input)                                                 |
    |        How many requests to test                                   |
    |    o outcnt (Output)                                               |
    |        How many requests completed                                 |
    |    o indices (Output)                                              |
    |        Requests completes in array of requests                     |
    |    o status (Output)                                               |
    |        Statuses of requests completed                              |
    |                                                                    |
    |    RETURN:                                                         |
    |    = 0 : On success                                                |
    |    < 0 : On other case                                             |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int testany (Rqst_t *rqst, int count, int *index, int *flag, RQST_Status *status)
{
  int        slideNr, 
             i, 
             excpn       = AZQ_SUCCESS;  
  unsigned 
  long long  finished[SLIDE_MAX];
  Thr_t      me          = self();
  static
  char      *where       = "testany";


  DBG_PRNT((stdout, "\ntestany(%p): \n", self()));

  if(rqst == (Rqst_t *)AZQ_RQST_NULL)                                          {excpn = AZQ_E_REQUEST;
                                                                                goto exception;}
  *flag = 0;
  *index = AZQ_UNDEFINED;

  /* 1. Calculate the number of slides to support the bitmap of "count" requests */
  slideNr = count/BITS_PER_SLIDE ;
  if(count % BITS_PER_SLIDE != 0)
    slideNr += 1;
  if(slideNr > SLIDE_MAX)                                                      {excpn = AZQ_E_EXHAUST;
                                                                                goto exception;}
  /* 2. Activate the bits corresponding to NULL requests passed to testany
   *    as well as to persistent inactive ones */
  for(i = 0; i < slideNr; i++) 
    finished[i] = 0;
  for(i = 0; i < count; i++) {
    if((rqst[i] == AZQ_RQST_NULL) || RQST_isInactive(rqst[i]))  {
      finished[rqst2slide(i)] |= ((unsigned long long)1 << (rqst2bit(i)));
    }
  }
  if(allDone(finished, slideNr, count)){
    /* The standard says:
       "The array_of_requests list may contain null or inactive handles. 
       If the list contains no active handles (list has length zero or all 
       entries are null or inactive), then the call returns immediately with
       index = MPI_UNDEFINED, and a empty status." 
     */
    RQST_setEmptyStatus(status);
    *flag = 1;
    goto retorno;
  }

  /* We are here because not all the requests of "rqst" are NULL or inactive. 
     So, explore "rqst" identifying the requests that are already covered, if any */
  for(i = 0; i < count; i++) {
    if(!(finished[rqst2slide(i)] & ((unsigned long long)1 << rqst2bit(i)))) {
      switch(rqst[i]->State) {
        case RQST_PENDING: 
        case RQST_PENDING + 1 :
        case RQST_FEEDING - 1 :
        case RQST_FEEDING: 
          if(RQST_isRecv(rqst[i]) && AZQ_getFromMBX(rqst[i]))
            RPQ_remove(&me->RecvPendReg, rqst[i]);
          AZQ_progress(me);
          break;
        case RQST_SATISFIED:
          if(status != AZQ_STATUS_IGNORE)
            RQST_getStatus(rqst[i], status);
          if (RQST_isPersistent(rqst[i]))  { RQST_setInactive(rqst[i]); }
          *flag = TRUE;
          *index = i;
          goto retorno;
          break;
        case RQST_CANCELLED:
          if (RQST_isPersistent(rqst[i]) && rqst[i]->PersistentWait)
            break;
          if(status != AZQ_STATUS_IGNORE)
            RQST_getStatus(rqst[i], status);
          if (RQST_isPersistent(rqst[i]))  {RQST_setInactive(rqst[i]); }
          *flag = TRUE;
          *index = i;
          goto retorno;
          break;
        default:
          excpn = AZQ_E_INTEGRITY;
          goto exception;
      } 
    }  
  }
  
retorno:
  DBG_PRNT((stdout, "timed_waitall(%p): End\n", self()));
  return AZQ_SUCCESS;

exception:
  XPN_print(excpn);
  return(azq_err[-excpn]);
}

