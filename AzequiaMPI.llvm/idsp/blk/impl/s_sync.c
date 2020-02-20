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

extern  int           (*getEndCom)(void *srcThr, Addr_t dst, int *mchn, void *thr);
#define self()        ((Thr_t)pthread_getspecific(key))


      /*________________________________________________________________
     /                                                                  \
    |    timed_send                                                      |
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
int timed_send(const int dstRank, char *buff, int cnt, Tag_t tag, int s_mode, unsigned timeout) {

  Addr      dst;
  Thr_t     dstThr;
  int       dstMchn;
  Rqst_t    rqst;
  Thr_t     me        = self();
  int       excpn;
  static
  char     *where     = "timed_send";


  DBG_PRNT((stdout, "\ntimed_send(%p): To [%d]. Tag %d. Count: %d. Timeout %x\n", me, dstRank, tag, cnt, timeout));

  /* 1. Init */
  dst.Rank  = dstRank;
  dst.Group = getGroup();

  if(0 > getEndCom(me, &dst, &dstMchn, &dstThr))                               {excpn = COM_E_INTERFACE;
                                                                                goto exception;}
  rqst = &(me->SyncRqst);
  RQST_init(rqst, RQST_SEND | RQST_SYNC | SLM_SET_msg(s_mode, me, cnt, dstMchn) | RRV_msg(s_mode, cnt),
                  &dst,
                  tag,
                  buff,
                  cnt,
                  dstMchn,
                  me,
                  dstThr);
  
  /* 2. Start */
  if (0 > (excpn = deal_send(rqst, timeout)))                                  goto exception;

  /* 3. Wait */
  timed_wait(&rqst, AZQ_STATUS_IGNORE, COM_FOREVER);

  DBG_PRNT((stdout, "timed_send(%p): End\n", me));

  return (AZQ_SUCCESS);

exception:
  XPN_print(excpn);
  return(azq_err[-excpn]);
}


