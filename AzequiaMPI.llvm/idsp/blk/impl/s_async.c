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
    |    _asend                                                          |
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
int _asend(const int dstRank, char *buff, int cnt, Tag_t tag, int s_mode, Rqst_t rqst) {

  Addr      dst;
  Thr_t     dstThr;
  int       dstMchn;
  Thr_t     me        = self();
  int       excpn;
  static
  char     *where     = "_asend";

  DBG_PRNT((stdout, "\nasend[%p, 0x%x, %d]: To [%d]. Tag %d. \n", me, getGroup(), getRank(), dstRank, tag));

  /* 0. Check integrity */
  if(rqst == (Rqst_t)AZQ_RQST_NULL)                                            {excpn = COM_E_INTEGRITY;
                                                                                goto exception;}
  /* 1. Init */
  dst.Rank  = dstRank;
  dst.Group = getGroup();

  if(0 > getEndCom(me, &dst, &dstMchn, &dstThr))                               {excpn = COM_E_INTERFACE;
                                                                                goto exception;}

  RQST_init(rqst, RQST_SEND | RQST_ASYNC | SLM_SET_msg(s_mode, me, cnt, dstMchn) | RRV_msg(s_mode, cnt),
                  &dst,
                  tag,
                  buff,
                  cnt,
                  dstMchn,
                  me,
                  dstThr);

  /* 2. Start */
  if (0 > (excpn = deal_send(rqst, COM_FOREVER)))                              goto exception;

  DBG_PRNT((stdout, "asend(%p): End\n", me));

  return AZQ_SUCCESS;

exception:
  XPN_print(excpn);
  return(azq_err[-excpn]);
}


