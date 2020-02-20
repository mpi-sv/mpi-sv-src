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
#define self()        ((Thr_t)pthread_getspecific(key))



      /*________________________________________________________________
     /                                                                  \
    |    aprobe                                                          |
    |    Test if given src address & tag, a matching whole message       |
    |    (all chunks) is found at thread maibox                          |
    |                                                                    |
    |    RETURNS: 0 if OK                                                |
    |    PARAMETERS:                                                     |
    |    o src (Input)                                                   |
    |        The tested message src address                              |
    |    o tag (Input)                                                   |
    |        Tested message tag                                          |
    |    o flag (Output)                                                 |
    |        TRUE  if the whole hdr was received                         |
    |        FALSE if the whole hdr was not received                     |
    |    o status (Output)                                               |
    |        Status of pending receive communication                     |
    |                                                                    |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int aprobe(int srcRank, Tag_t tag, int *flag, Status *status) {

  Addr      src;
  Thr_t     me     = self();
  Header_t  hdr;

  //DBG_PRNT((stdout, "\naprobe(%p): From [%x, %d]. Tag %d\n", me, src->Group, src->Rank, tag));
  DBG_PRNT((stdout, "\naprobe(%p): From [%x, %d]. Tag %d\n", me, src.Group, src.Rank, tag));
  *flag = FALSE;

  src.Rank  = srcRank;
  src.Group = getGroup();

  LOCK(me);

  if (MBX_lookUp(me->MailBox, &hdr, &src, tag)) {
    /* Whole hdr found, fill status */
    *flag = TRUE;
    if (status != AZQ_STATUS_IGNORE) {
      status->Src    = hdr->Src;
      status->Tag    = hdr->Tag;
      status->Count  = hdr->MessageSize;
    }
  }

  UNLOCK(me);

  DBG_PRNT((stdout, "aprobe(%p): End\n", me));

  return AZQ_SUCCESS;
}



