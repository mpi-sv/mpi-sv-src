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
#define self()        ((Thr_t)pthread_getspecific(key))


      /*________________________________________________________________
     /                                                                  \
    |    timed_probe                                                     |
    |    Test if given src address & tag, a matching whole message       |
    |    (all chunks) is found at thread maibox, else block.             |
    |                                                                    |
    |    RETURNS: 0 if OK                                                |
    |    PARAMETERS:                                                     |
    |    o src (Input)                                                   |
    |        The tested message src address                              |
    |    o tag (Input)                                                   |
    |        Tested message tag                                          |
    |    o status (Output)                                               |
    |        Status of pending receive communication                     |
    |    o timeout  (Input)                                              |
    |        Relative timeout (in milliseconds)                          |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int timed_probe (const int srcRank, Tag_t tag, Status *status, unsigned timeout) {

  Addr      src;
  Thr_t     me      = self();
  Rqst     *rqst;
  Header_t  hdr;


  DBG_PRNT((stdout, "\ntimed_probe(%p): From [%x, %d]. Tag %d. Timeout %x\n", self(), src.Group, src.Rank, tag, timeout));

  src.Rank  = srcRank;
  src.Group = getGroup();

  LOCK(me);

  /* 1. Find a message in mailbox */
  if (MBX_lookUp(me->MailBox, &hdr, &src, tag)) {

    /* Whole hdr found, fill status */
    if (status != AZQ_STATUS_IGNORE) {
      status->Src    = hdr->Src;
      status->Tag    = hdr->Tag;
      status->Count  = hdr->MessageSize;
    }

    DBG_PRNT((stdout, "\ntimed_probe(%p): Found in MBX from [%d, %d]. Tag %d. Count %d\n", self(),
                               status->Src.Group, status->Src.Rank, status->Tag, status->Count));

    UNLOCK(me);
    return AZQ_SUCCESS;
  }

  DBG_PRNT((stdout, "\ntimed_probe(%p): No request found in MBX. Waiting ... \n", self()));

  /* 2. No mesage found. Wait for one. */
  rqst = &me->SyncRqst;
  RQST_init(rqst, RQST_RECV | RQST_SYNC | RQST_PROBE | MODE_DATA | MODE_RRV_RQST,
                  &src,
                  tag,
                  NULL,
                  0,
                  DFLT_MCHN,
                  me,
                  NULL);

  /* 3. Register the request. No call to deal_recv(), because not neccessary look up in mailbox. */
  DBG_PRNT((stdout, "\ntimed_probe(%p): Registering %p on RPQ %p\n", me, rqst, me->RecvPendReg));

  
  RPQ_put(me->RecvPendReg, rqst);
  rqst->State = RQST_PENDING;

  UNLOCK(me);

  /* 3. Wait */
  timed_wait(&rqst, status, COM_FOREVER);


  DBG_PRNT((stdout, "timed_probe(%p): End\n", me));
  return AZQ_SUCCESS;
}



