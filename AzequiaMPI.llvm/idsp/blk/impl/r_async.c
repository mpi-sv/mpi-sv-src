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
    |    arecv                                                           |
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
int arecv(const int srcRank, char *buff, int cnt, Tag_t tag, Rqst_t rqst) {

  Addr       src;
  Thr_t      me         = self();
  int        excpn;
  static
  char      *where      = "arecv";


  //DBG_PRNT((stdout, "\narecv(%p): From [%x, %d]. Tag %d  Rqst: %p. \n", me, src->Group, src->Rank, tag, rqst));
  DBG_PRNT((stdout, "\narecv(%p): From [%x, %d]. Tag %d  Rqst: %p. \n", me, src.Group, src.Rank, tag, rqst));
  /* 0. Check integrity */
  if(rqst == (Rqst_t)AZQ_RQST_NULL)                                            {excpn = COM_E_INTEGRITY;
                                                                                goto exception;}
  /* 1. Init */
  src.Rank  = srcRank;
  src.Group = getGroup();

  RQST_init(rqst, RQST_RECV | RQST_ASYNC | RQST_ANY,
                  &src,
                  tag,
                  buff,
                  cnt,
                  DFLT_MCHN,
                  me,
                  NULL);

  /* 2. Start */
  if (0 > (excpn = deal_recv(rqst)))                                           goto exception;

  DBG_PRNT((stdout, "arecv(%p): End\n", me));

  return AZQ_SUCCESS;

exception:
  XPN_print(excpn);
  return(azq_err[-excpn]);
}

