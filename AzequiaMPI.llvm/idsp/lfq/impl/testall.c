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

#define self()        ((Thr_t)pthread_getspecific(key))


/*----------------------------------------------------------------*
 *   Declaration of external functions                            *
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------*
 *   Implementation of public interface                           *
 *----------------------------------------------------------------*/

      /*________________________________________________________________
     /                                                                  \
    |    testall                                                         |
    |    Test if all async request has finished                          |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o rqst (Input)                                                  |
    |        The tested asynchronous array of requests previously done   |
    |    o cnt (Input)                                                   |
    |        How many requests to test                                   |
    |    o flag (Output)                                                 |
    |        TRUE  if the request is done                                |
    |        FALSE if the request not done                               |
    |    o status (Output)                                               |
    |        The array of statuses                                       |
    |                                                                    |
    |    RETURN:                                                         |
    |    = 0 : On success                                                |
    |    < 0 : On other case                                             |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */ 
int testall (Rqst_t *rqst, int cnt, int *flag, RQST_Status *status) 
{
  int      i,
           excpn       = AZQ_SUCCESS;
  int      satfd       = 0;
  Thr_t    me          = self();
  static
  char    *where       = "testall";

  DBG_PRNT((stdout, "\ntestall(%p): \n", self()));

  if(rqst == (Rqst_t *)AZQ_RQST_NULL)                                          {excpn = AZQ_E_REQUEST;
                                                                                goto exception;}
  *flag = FALSE;
  for(i = 0; i < cnt; i++) {
    if (rqst[i] == AZQ_RQST_NULL)
      satfd++;
    else {
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
          satfd++;
          break;
        case RQST_CANCELLED:
          if (RQST_isPersistent(rqst[i]) && rqst[i]->PersistentWait)
            break;
          satfd++;
          break;
        default:
          printf("testall(%p). Rqst: %p with state %x\n", self(), rqst[i], (rqst[i])->State); fflush(stdout);
          excpn = AZQ_E_INTEGRITY;
          goto exception;
      }
    }
  }
  
  if(satfd == cnt) {
    *flag = TRUE;
    for (i = 0; i < cnt; i++) {
      if((rqst[i] == AZQ_RQST_NULL))  {
        RQST_setEmptyStatus(&status[i]);
      } 
      else {
        RQST_getStatus(rqst[i], &status[i]);
      }
    }
  }

  DBG_PRNT((stdout, "testall(%p): End\n", self()));

  return AZQ_SUCCESS;

exception:
  XPN_print(excpn);
  return(excpn);
}



