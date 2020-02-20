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
#include <addr_hddn.h>
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



      /*________________________________________________________________
     /                                                                  \
    |    timed_recv                                                      |
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
int timed_recv(const int srcRank, char *buff, int cnt, Tag_t tag, Status *status, unsigned timeout)  {

  Addr       src;
  Thr_t      me         = self();
  Rqst      *rqst;
  int        excpn;
  static
  char      *where      = "timed_recv";

  DBG_PRNT((stdout, "\ntimed_recv[%p, 0x%x, %d]: From [%d]. Tag %d. Timeout %x\n", self(), getGroup(), getRank(), srcRank, tag, timeout));

  /* [1]. Init */
  src.Rank  = srcRank;
  src.Group = getGroup();

  rqst = &me->SyncRqst;
  RQST_init(rqst, RQST_RECV | RQST_SYNC | RQST_ANY,
                  &src,
                  tag,
                  buff,
                  cnt,
                  DFLT_MCHN,
                  me,
                  NULL);

  /* 2. Start */
  if (0 > (excpn = deal_recv(rqst)))                                           goto exception;

  /* 3. Wait */
  timed_wait(&rqst, status, COM_FOREVER);
  DBG_PRNT((stdout, "timed_recv(%p): End\n", me));
  //added by Herman for debugging
  //fprintf(stdout, "\ntimed_recv[%p, 0x%x, %d]: From [%d]. Tag %d. Timeout %x, buff %s, count: %d\n", self(),
  //	  getGroup(), getRank(), srcRank, tag, timeout,buff, cnt);
  //fflush(stdout);
  return AZQ_SUCCESS;

exception:
  XPN_print(excpn);
  return(azq_err[-excpn]);
}



