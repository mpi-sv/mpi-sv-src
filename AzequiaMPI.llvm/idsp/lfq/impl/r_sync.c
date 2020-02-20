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

#define self()        ((Thr_t)pthread_getspecific(key))
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

/*----------------------------------------------------------------*
 *   Declaration of external functions                            *
 *----------------------------------------------------------------*/


/*----------------------------------------------------------------*
 *   Implementation of public interface                           *
 *----------------------------------------------------------------*/

      /*________________________________________________________________
     /                                                                  \
    |    AZQ_recv                                                        |
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
    |    o status   (Output)                                             |
    |        Relevant only to sync receive requests.                     |
    |    o timeout  (Input)                                              |
    |        Relative timeout (in milliseconds)                          |
    |                                                                    |
    |    RETURN:                                                         |
    |    = 0  : On success                                               |
    |    <  0 : On error                                                 |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int AZQ_recv(const int rank, const char *buff, const int cnt, const Tag_t tag, RQST_Status *status)
{
  Thr_t  me = self();

  DBG_PRNT((stdout, "\nAZQ_recv[%p, 0x%x, %d]: From [%d]. Tag %d.\n", self(), 
                                               getGroup(), getRank(), rank, tag));

#ifdef USE_FASTBOXES
  if( (cnt) <= FBOX_BUF_MAX) { 
    int done;
    recvFromFbox(me, rank, buff, cnt, tag, status, &done); 
    if(done) { 
      return AZQ_SUCCESS; 
    } 
  } 
#endif

  RQST_initRecv(&me->SyncRqst,  
                  (rank), 
                  (tag), 
                  (buff), 
                  (cnt), 
                  me); 

  if(!AZQ_getFromMBX(&me->SyncRqst))
    DLQ_put(&me->RecvPendReg, &me->SyncRqst);
  while (me->SyncRqst.State != RQST_SATISFIED) {
    AZQ_progress(me); 
  }
  if((status) != AZQ_STATUS_IGNORE) 
    RQST_getStatus((&me->SyncRqst), (status)); 

  DBG_PRNT((stdout, "AZQ_recv(%p): End\n", me));
  return AZQ_SUCCESS;
}



