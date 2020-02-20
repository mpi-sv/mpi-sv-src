/*-
 * Copyright (c) 2009-2011 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */


#ifndef _RPC_RQST_H_
#define _RPC_RQST_H_


/*----------------------------------------------------------------*
 *   Declaration of types used by this module                     *
 *----------------------------------------------------------------*/
#include <config.h>
#include <rqst.h>
#include <addr.h>
#include <util.h>



/*----------------------------------------------------------------*
 *   Exported constants                                           *
 *----------------------------------------------------------------*/
#define  __XPN_PRINT


      /*________________________________________________________________
     /                                                                  \
    |    RQST_match                                                      |
    |                                                                    |
    |    Test if a request matches a "addr" and a "tag"                  |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o rqst         (Input)                                          |
    |        request                                                     |
    |    o keyRank, tag (Input)                                          |
    |        keys                                                        |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
#define old_RQST_matchRpc(rqst, keyAddr, keyTag, mode)             \
(                                                           \
  ADDR_match((keyAddr), &(rqst)->Status.Src)   &&       \
  TAG_MATCH((rqst)->Status.Tag, (keyTag))        &&         \
  (((mode) & MODE_REMOTE) ? (((rqst)->Type & RQST_RECV) ? (((rqst)->Type & RQST_MASK) & (mode)) : 1) : 1)  \
)


#define RQST_matchRpc(rqst, keyAddr, keyTag, mode)             \
(                                                           \
  ADDR_match((keyAddr), &(rqst)->Status.Src)   &&       \
  TAG_MATCH((rqst)->Status.Tag, (keyTag))        &&         \
  ((rqst)->Hdr.Mode & (mode))                               \
)




      /*________________________________________________________________
     /                                                                  \
    |    RQST_initRpcSend                                                |
    |                                                                    |
    |    Open a request                                                  |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o rqst        (Input/Output)                                    |
    |        The request to mark as OPEN                                 |
    |    o opnMode     (Input)                                           |
    |        Kind and mode of request                                    |
    |    o addr, tag   (Input)                                           |
    |        Search keys                                                 |
    |    o buf, count  (Input)                                           |
    |        Data involved                                               |
    |    o me          (Input)                                           |
    |        Thread self                                                 |
    |    o dstThr      (Input)                                           |
    |        Thread destination                                          |
    |                                                                    |
    |    RETURN:                                                         |
    |    = 1 : if rqst found                                             |
    |    = 0 : if not found                                              |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
extern  int  AZQ_cpuId;


#define old_RQST_initRpcSend(rqst, type, addr,    myAddr, tag, buf, count, dstmchn, me, dstThr)  \
{                                                                                           \
DBG_PRNT((stdout, "RQST_initRpcSend(%p): Begin\n", self()));                             \
if (dstmchn == AZQ_cpuId) {                                                             \
(rqst)->Hdr.Mode      = MODE_LAST_FRAGMENT;                                           \
(rqst)->Type   = (type);                                                              \
}                                                                                       \
else {                                                                                  \
(rqst)->Hdr.Dst             = *(addr);                                                \
(rqst)->Hdr.Mode            = MODE_REMOTE;                                            \
(rqst)->Hdr.DstMchn         = dstmchn;                                                \
(rqst)->Hdr.MessageSize     = count;                                                  \
(rqst)->Hdr.SrcMchn         = AZQ_cpuId;                                              \
(rqst)->Hdr.RrvDstAddr      = NULL;                                                   \
if ((type) & RQST_ASYNC) {                                                            \
(rqst)->Status.Src.Rank   = addr->Rank;                                                   \
(rqst)->Status.Tag       = tag;                                                     \
(rqst)->Status.Count     = 0;                                                       \
(rqst)->Status.Cancelled = FALSE;                                                   \
(rqst)->Type   = type;                                                              \
}                                                                                     \
else {                                                                                \
(rqst)->Type   = 0;                                                                 \
}                                                                                     \
if ((type) & RQST_RRV) {                                                              \
(rqst)->Hdr.Mode           |= MODE_RRV_RQST;                                             \
} else {                                                                                    \
(rqst)->Hdr.Mode           |= MODE_EAGER;                                             \
}\
}                                                                                       \
(rqst)->Hdr.SrcThr            = (void *)(me);                                           \
(rqst)->Hdr.DstThr            = dstThr;                                                 \
(rqst)->Hdr.Src               = *(myAddr);                                         \
(rqst)->Hdr.Tag               = tag;                                                    \
(rqst)->Hdr.Payload           = buf;                                                    \
(rqst)->Hdr.PayloadSize       = count;                                                  \
(rqst)->Owner                 = (void *)(me);                                           \
(rqst)->State                 = RQST_PENDING;                                           \
DBG_PRNT((stdout, "RQST_initRpcSend(%p): rqst %p has state %x. End\n",                \
self(), (rqst), (rqst)->State));                                               \
}


#define RQST_initRpcSend(rqst, addr,    myAddr, tag, buf, count, dstmchn, me, dstThr)  \
{                                                                                           \
  DBG_PRNT((stdout, "RQST_initRpcSend(%p): Begin\n", self()));                             \
  if (dstmchn == AZQ_cpuId) {                                                             \
    (rqst)->Hdr.Mode      = MODE_LAST_FRAGMENT;                                           \
  }                                                                                       \
  else {                                                                                  \
    (rqst)->Hdr.Dst             = *(addr);                                                \
    (rqst)->Hdr.Mode            = MODE_REMOTE;                                            \
    (rqst)->Hdr.DstMchn         = dstmchn;                                                \
    (rqst)->Hdr.MessageSize     = count;                                                  \
    (rqst)->Hdr.SrcMchn         = AZQ_cpuId;                                              \
    (rqst)->Hdr.RrvDstAddr      = NULL;                                                   \
  }                                                                                       \
  (rqst)->Hdr.Mode             |= MODE_RRV_RQST | MODE_LAST_FRAGMENT;                     \
  (rqst)->Hdr.SrcThr            = (void *)(me);                                           \
  (rqst)->Hdr.DstThr            = dstThr;                                                 \
  (rqst)->Hdr.Src               = *(myAddr);                                         \
  (rqst)->Hdr.Tag               = tag;                                                    \
  (rqst)->Hdr.Payload           = buf;                                                    \
  (rqst)->Hdr.PayloadSize       = count;                                                  \
  (rqst)->Owner                 = (void *)(me);                                           \
  (rqst)->State                 = RQST_PENDING;                                           \
  (rqst)->Type        = RQST_SEND | RQST_SYNC;                                             \
  DBG_PRNT((stdout, "RQST_initRpcSend(%p): rqst %p has state %x. End\n",                \
           self(), (rqst), (rqst)->State));                                               \
}




#define RQST_initRpcRecv(rqst, keyAddr, myAddr, keyTag, buf, count, dstmchn, me, dstThr)           \
{                                                                                           \
  DBG_PRNT((stdout, "RQST_initRpcRecv(%p): Begin\n", self()));                                     \
  /*fprintf(stdout, "RQST_initRpcRecv(%p): Rqst %p. Begin\n", self(), (rqst)); fflush(stdout);*/   \
  (rqst)->Status.Src       = *(keyAddr);                                            \
  (rqst)->Status.Tag       = (keyTag);                                                         \
  (rqst)->Status.Count     = 0;                                                           \
  (rqst)->Status.Cancelled = FALSE;                                                       \
  (rqst)->Buff             = buf;                                                         \
  (rqst)->BuffInPtr        = buf;                                                         \
  (rqst)->BuffSize         = count;                                                       \
  (rqst)->RrvAddr          = NULL;                                                        \
  (rqst)->Type             = RQST_RECV | RQST_SYNC;                                       \
  (rqst)->Hdr.Mode         = MODE_ANY;                                                    \
  (rqst)->State            = RQST_PENDING;                                                \
  DBG_PRNT((stdout, "RQST_initRpcRecv(%p): rqst %p has state %x. End\n",                     \
           self(), (rqst), (rqst)->State));                                               \
}






/*----------------------------------------------------------------*
 *   Declaration of public interface implemented by this module   *
 *----------------------------------------------------------------*/
extern int   RQST_ready       (void *dstThr, Rqst_t  rqst);
#endif
