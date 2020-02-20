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
#include <rpq.h>
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



      /*________________________________________________________________
     /                                                                  \
    |    cancel                                                          |
    |    Try to cancel a non-blocking or persistent request              |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o rqst     (Input / Output)                                     |
    |        Request to cancel                                           |
    |                                                                    |
    |    RETURN:                                                         |
    |    0  : On success                                                 |
    |    < 0: On other case                                              |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int cancel(Rqst_t rqst) {

  int      excpn;
  Thr_t    me          = self();
  Thr_t    dstThr;
  static
  char    *where       = "Cancel";

  DBG_PRNT((stdout, "\nCancel(%p): \n", me));

  if(rqst == (Rqst_t)AZQ_RQST_NULL)                                             {excpn = AZQ_E_REQUEST;
                                                                                goto retorno;}

  if (RQST_isRecv(rqst)) {

    LOCK(me);
    DBG_PRNT((stdout, "\nCancel(%p): receive request %p \n", me, rqst));
    if (RQST_isCancellable(rqst)) {
      RPQ_remove(me->RecvPendReg, rqst);
      RQST_setCancelled(rqst);
    }
    UNLOCK(me);

  } else {

    /* A remote send request is not cancellable */
    if (rqst->Hdr.Mode & MODE_REMOTE)  return AZQ_SUCCESS;

    dstThr = rqst->DstThr;

    LOCK(dstThr);
    DBG_PRNT((stdout, "\nCancel(%p): send request %p \n", me, rqst));
    if (RQST_isCancellable(rqst)) {
      if (DLQ_lookUp(dstThr->MailBox, &(rqst->Hdr))) {
        MBX_remove(dstThr->MailBox, &(rqst->Hdr));
        RQST_setCancelled(rqst);
      }
    }
    UNLOCK(dstThr);

  }

  DBG_PRNT((stdout, "Cancel(%p): End\n", me));
  return AZQ_SUCCESS;

retorno:
  XPN_print(excpn);
  return(azq_err[-excpn]);
}


