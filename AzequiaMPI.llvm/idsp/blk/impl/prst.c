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
    |    send_init                                                       |
    |                                                                    |
    |    Create a persistent communiction request                        |
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
    |    o rqst     (Input/Output)                                       |
    |        Allow persistent. Completed with test/wait or deleted       |
    |        with free_request                                           |
    |                                                                    |
    |    RETURN:                                                         |
    |    = 0 : On success                                                |
    |    < 0 : On error                                                  |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int send_init (const int dstRank, char *buff, int cnt, int tag, int s_mode, Rqst_t rqst) {

  Addr     dst;
  Thr_t    dstThr;
  int      dstMchn;
  Thr_t    me             = self();
  int      excpn          = AZQ_SUCCESS;
  static
  char    *where          = "send_init";


  DBG_PRNT((stdout, "Send_init(%p): \n", me));

  /* 0. Check integrity */
  if(rqst == (Rqst_t)AZQ_RQST_NULL)                                            {excpn = COM_E_INTEGRITY;
                                                                                goto exception;}
  /* 1. Init */
  dst.Rank  = dstRank;
  dst.Group = getGroup();

  if(0 > getEndCom(me, &dst, &dstMchn, &dstThr))                               {excpn = COM_E_INTERFACE;
                                                                                goto exception;}

  RQST_init(rqst, RQST_SEND | RQST_ASYNC | RQST_PERSISTENT | SLM_SET_msg(s_mode, me, cnt, dstMchn) | RRV_msg(s_mode, cnt),
                  &dst,
                  tag,
                  buff,
                  cnt,
                  dstMchn,
                  me,
                  dstThr);

  DBG_PRNT((stdout, "Send_init (%p): End\n", me));

  return AZQ_SUCCESS;

exception:
  XPN_print(excpn);
  return(azq_err[-excpn]);
}



      /*________________________________________________________________
     /                                                                  \
    |    psend_start                                                     |
    |                                                                    |
    |    Send the data in  "buffer"                                      |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o rqst     (Input)                                              |
    |        Persistent request filled in init_send                      |
    |                                                                    |
    |    RETURN:                                                         |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int psend_start (Rqst_t rqst) {

  int           excpn;
  static char  *where          = "send_start";

  DBG_PRNT((stdout, "\nPsend_start(%p): \n", self()));

  /* 0. Check integrity */
  if (!RQST_isPersistent(rqst))                                                {excpn = COM_E_INTERFACE;
                                                                                goto exception;}
  /* 2. Start */
  if (0 > (excpn = deal_send(rqst, COM_FOREVER)))                              goto exception;

  DBG_PRNT((stdout, "Psend_start (%p): End\n", self()));

  return(AZQ_SUCCESS);

exception:
  XPN_print(excpn);
  return(azq_err[-excpn]);
}



      /*________________________________________________________________
     /                                                                  \
    |    precv_init                                                      |
    |    Create a persistent receive communication request               |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o srcAddr  (Input)                                              |
    |        The desired source address                                  |
    |    o buffer   (Input)                                              |
    |        User buffer to store the incomming data                     |
    |    o count    (Input)                                              |
    |        Input:  Available space in "buffer" to receive the message  |
    |    o mode     (Input)                                              |
    |        Determines if reception is synchronous or asynchronous      |
    |    o tag      (Input)                                              |
    |        Desired Tag in the incoming message                         |
    |    o rqst     (Output)                                             |
    |        Relevant only to async receive requests.                    |
    |        Allows further testing (GC_test) or waiting (GC_wait)       |
    |    o timeout  (Input)                                              |
    |        Relative timeout (in milliseconds)                          |
    |                                                                    |
    |    RETURN:                                                         |
    |    = 0 : On success                                                |
    |    < 0 : On error                                                  |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int precv_init(const int srcRank, char *buf, int cnt, Tag_t tag, Rqst_t rqst) {

  Addr       src;
  Thr_t      me         = self();
  int        excpn      = AZQ_SUCCESS;
  static
  char      *where      = "recv_init";


  DBG_PRNT((stdout, "Precv_init (%p): From [%x, %d]. Tag %d. \n", me, src.Group, src.Rank, tag));

  /* 0. Check integrity */
  if (rqst == (Rqst_t)AZQ_RQST_NULL)                                           {excpn = COM_E_INTEGRITY;
                                                                                goto exception;}

  /* 1. Init */
  src.Rank  = srcRank;
  src.Group = getGroup();

  RQST_init(rqst, RQST_RECV | RQST_ASYNC | RQST_PERSISTENT,
                  &src,
                  tag,
                  buf,
                  cnt,
                  DFLT_MCHN,
                  me,
                  NULL);

  DBG_PRNT((stdout, "Precv_init (%p): End\n", me));

  return AZQ_SUCCESS;

exception:
  XPN_print(excpn);
  return(azq_err[-excpn]);
}



      /*________________________________________________________________
     /                                                                  \
    |    precv_start                                                     |
    |    Receive with a persistent receive communication request         |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o rqst     (Input)                                              |
    |        Persistent receive communication request filled in          |
    |        init_recv                                                   |
    |                                                                    |
    |    RETURN:                                                         |
    |    = 0 : On success                                                |
    |    < 0 : On error                                                  |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int precv_start (Rqst_t rqst) {

  int        excpn      = AZQ_SUCCESS;
  static
  char      *where      = "Precv_start";


  DBG_PRNT((stdout, "Precv_start (%p): Rqst %p\n", self(), rqst));

  /* 0. Check integrity */
  if(!RQST_isPersistent(rqst))                                                 {excpn = COM_E_INTERFACE;
                                                                                goto exception;}
  /* 2. Start */
  if (0 > (excpn = deal_recv(rqst)))                                           goto exception;

  DBG_PRNT((stdout, "Precv_start (%p): End\n", self()));
  return AZQ_SUCCESS;

exception:
  XPN_print(excpn);
  return(azq_err[-excpn]);
}

