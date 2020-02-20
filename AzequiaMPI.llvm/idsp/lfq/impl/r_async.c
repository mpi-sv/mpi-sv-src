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

#define self()        ((Thr_t)pthread_getspecific(key))


/*----------------------------------------------------------------*
 *   Declaration of external functions                            *
 *----------------------------------------------------------------*/


/*----------------------------------------------------------------*
 *   Implementation of public interface                           *
 *----------------------------------------------------------------*/

      /*________________________________________________________________
     /                                                                  \
    |    AZQ_arecv                                                       |
    |    Receive data                                                    |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o srcAddr  (Input)                                              |
    |        The desired source address                                  |
    |    o buffer   (Input)                                              |
    |        User buffer to store the incomming data                     |
    |    o count    (Input)                                              |
    |        Input:  Available space in "buffer" to receive the message  |
    |    o tag      (Input)                                              |
    |        Desired Tag in the incoming message                         |
    |    o rqst     (Output)                                             |
    |        Relevant only to async receive requests.                    |
    |        Allows further testing (GC_test) or waiting (GC_wait)       |
    |                                                                    |
    |    RETURN:                                                         |
    |    =  0 : On success                                               |
    |    <  0 : On error                                                 |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int AZQ_arecv(const int srcRank, const char *buff, const int cnt, const Tag_t tag, Rqst_t rqst) 
{
  Thr_t        me         = self();
  int          done;
  int          excpn;
  static
  char        *where      = "AZQ_arecv";

  DBG_PRNT((stdout, "AZQ_arecv(%p): From [%x]. Tag %x  Rqst: %p. \n", me, srcRank, tag, rqst));

#ifdef USE_FASTBOXES
  if( (cnt) <= FBOX_BUF_MAX) { 
    recvFromFbox(me, srcRank, buff, cnt, tag, &rqst->Status, &done);
    if(done) { 
      RQST_setSatisfied(rqst);
      return AZQ_SUCCESS; 
    }
  }
#endif

  /* 2. Start */
  RQST_initARecv(rqst, 
				 srcRank,
				 tag,
				 buff,
				 cnt,
				 me);
  DLQ_put(&me->RecvPendReg, rqst);

  DBG_PRNT((stdout, "AZQ_arecv(%p): End\n", me));

  return AZQ_SUCCESS;

exception:
  XPN_print(excpn);
  return(azq_err[-excpn]);
}

