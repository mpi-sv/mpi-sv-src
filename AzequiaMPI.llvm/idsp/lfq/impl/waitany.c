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
#define SCHED_YIELD_PERIOD 32  /* Must be power of 2 */
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

#define self()        ((Thr_t)pthread_getspecific(key))


/*----------------------------------------------------------------*
 *   Declaration of external functions                            *
 *----------------------------------------------------------------*/


/*----------------------------------------------------------------*
 *   Implementation of function interface                         *
 *----------------------------------------------------------------*/

      /*________________________________________________________________
     /                                                                  \
    |    timed_waitany                                                  |
    |    Block the invoking thread until                                 |
    |    1) Any of the requests in the "rqst" vector                     |
    |       a)  is satisfied                                             |
    |       b)  times out                                                |
    |    2) It is killed                                                 |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o rqst     (Input)                                              |
    |        Vector of requests                                          |
    |    o count    (Input)                                              |
    |        Dimension of the "rqst" request vector                      |
    |    o index    (Output)                                             |
    |        The either satisfied or timedout request in "rqst"          |
    |    o status   (Output)                                             |
    |        Status of the satisfied request                             |
    |    o timeout  (Input)                                              |
    |        Relative timeout (in milliseconds)                          |
    |                                                                    |
    |    RETURN:                                                         |
    |    = 0 : On success                                                |
    |    < 0 : On other case                                             |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */

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



int AZQ_waitany(Rqst_t *rqst, int count, int *index, RQST_Status *status) 
{
  int        slideNr, 
             i, 
             j           = 0,
             excpn       = AZQ_SUCCESS;  
  Thr_t      me          = self();
  unsigned 
  long long  finished[SLIDE_MAX];
  static
  char      *where       = "timed_waitany";


  DBG_PRNT((stdout, "\ntimed_waitany(%p): \n", self()));

  if(rqst == (Rqst_t *)AZQ_RQST_NULL)                                          {excpn = AZQ_E_REQUEST;
                                                                                goto exception;}

  /* 1. Calculate the number of slides to support the bitmap of "count" requests */
  slideNr = count/BITS_PER_SLIDE ;
  if(count % BITS_PER_SLIDE != 0)
    slideNr += 1;
  if(slideNr > SLIDE_MAX)                                                      {excpn = AZQ_E_EXHAUST;
                                                                                goto exception;}
  /* 2. Activate the bits corresponding to the NULL and 
   *    inactive persistent requests passed to waitall */
  for(i = 0; i < slideNr; i++) 
    finished[i] = 0;
  for(i = 0; i < count; i++) {
    if(rqst[i] == AZQ_RQST_NULL  || RQST_isInactive(rqst[i]))  {
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
    *index  = AZQ_UNDEFINED;
    RQST_setEmptyStatus(status);
    goto retorno;    
  }

  i = 0;
  while(1) { /* The "count" right-most bits must be filled to get out the loop */
    if(!(finished[rqst2slide(i)] & ((unsigned long long)1 << rqst2bit(i)))) {
      switch(rqst[i]->State) {
        case RQST_PENDING: 
        case RQST_PENDING + 1: 
        case RQST_FEEDING - 1:
        case RQST_FEEDING: 
          if(RQST_isRecv(rqst[i]) && AZQ_getFromMBX(rqst[i]))
            RPQ_remove(&rqst[i]->Owner->RecvPendReg, rqst[i]);
          AZQ_progress(me);
          if((++j & (SCHED_YIELD_PERIOD - 1)) == 0) /* if(++j % SCHED_YIELD_PERIOD == 0) */
            sched_yield();
          break;
        case RQST_SATISFIED:
          if(status != AZQ_STATUS_IGNORE)
            RQST_getStatus(rqst[i], status);
          if (RQST_isPersistent(rqst[i]))  { RQST_setInactive(rqst[i]); }
          finished[rqst2slide(i)] |= ((unsigned long long)1 << rqst2bit(i));
          *index  = i;
          goto retorno;    
          break;
        case RQST_CANCELLED:
          if(status != AZQ_STATUS_IGNORE)
            RQST_getStatus(rqst[i], status);
          if (RQST_isPersistent(rqst[i]))  { 
            while (rqst[i]->PersistentWait) { 
              AZQ_progress(me);
              if((++j & (SCHED_YIELD_PERIOD - 1)) == 0) sched_yield();
            }
            RQST_setInactive(rqst[i]); 
          }
          finished[rqst2slide(i)] |= ((unsigned long long)1 << rqst2bit(i));
          *index  = i;
          goto retorno;    
          break;
        default:
          excpn = AZQ_E_INTEGRITY;
          goto exception;
      } 
    }
    if(++i == count)
      i = 0;
  }

retorno:
  DBG_PRNT((stdout, "timed_waitall(%p): End\n", self()));
  return AZQ_SUCCESS;

exception:
  XPN_print(excpn);
  return(azq_err[-excpn]);
}
