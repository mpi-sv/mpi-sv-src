/*-
 * Copyright (c) 2009-2011 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */


#ifndef _RQST_H_
#define _RQST_H_


/*----------------------------------------------------------------*
 *   Declaration of types used by this module                     *
 *----------------------------------------------------------------*/
#include <config.h>
#include <arch.h>
#include <addr.h>
#include <util.h>


/*----------------------------------------------------------------*
 *   Exported constants                                           *
 *----------------------------------------------------------------*/

#ifdef USE_FASTBOXES
  #define TURN_SEND 0 
  #define TURN_RECV 1 
#endif

#define  __XPN_PRINT
#define SCHED_YIELD_PERIOD (32 * 1024 * 1024)  /* Must be power of 2 */

    /*----------------------------------------------------------------*
     *   Exported exceptions                                          *
     *----------------------------------------------------------------*/
#define RQST_E_OK          0
#define RQST_E_EXHAUST    (RQST_E_OK          - 1)
#define RQST_E_INTEGRITY  (RQST_E_EXHAUST     - 1)
#define RQST_E_TIMEOUT    (RQST_E_INTEGRITY   - 1)
#define RQST_E_INTERFACE  (RQST_E_TIMEOUT     - 1)
#define RQST_E_SYSTEM     (RQST_E_INTERFACE   - 1)
#define RQST_E_SIGNALED   (RQST_E_SYSTEM      - 1)
#define RQST_E_DEADPART   (RQST_E_SIGNALED    - 1)


#ifdef __RQST_DEBUG
#define DBG_PRNT(pmsg) \
  { \
    fprintf pmsg \
    ; fflush(stdout); \
  }
#else
  #define DBG_PRNT(pmsg)
#endif

/*----------------------------------------------------------------*
 *   Exported types                                               *
 *----------------------------------------------------------------*/
/* Bits of "state" of a Request */
#define RQST_PENDING              0x00000000
#define RQST_SATISFIED            0x00000002
#define RQST_FEEDING              0x00000004
#define RQST_CANCELLED            0x00000008
#define RQST_MAKES_WAITING        0x00000010
#define RQST_INACTIVE             0x00000020
#define RQST_WAITING              0x00000040

/* Bits of "type" of a request */
#define RQST_RECV                 0x00000001
#define RQST_SEND                 0x00000002
#define RQST_SYNC                 0x00000004
#define RQST_ASYNC                0x00000008
#define RQST_PERSISTENT           0x00000010
#define RQST_SPLITCOPY            0x00000020
#define RQST_RRV                  0x00000040
#define RQST_ACKED                0x00000080
#define RQST_PROBE                0x00000100
#define RQST_DATA_CARRIER         0x00000200


/* Bits of "mode" field of a Header */
#define MODE_LAST_FRAGMENT        0x00000001
#define MODE_REMOTE               0x00000002
#define MODE_NO_COPY              0x00000004

#define MODE_ANY                  0x00000F00
#define MODE_EAGER                0x00000100
#define MODE_RRV_RQST             0x00000200
#define MODE_RRV_ACK              0x00000400
#define MODE_RRV_DATA             0x00000800



/* Double linked list of items in the queue */
struct Link {
  struct Link  *Next;
  struct Link  *Prev;
};
typedef struct Link Link, *Link_t;


typedef struct Header Header, *Header_t;
struct Header {
  Link          Link  __attribute__(( aligned(CACHE_LINE_SIZE) )) ;
  Addr          Dst   /*__attribute__(( aligned(CACHE_LINE_SIZE) )) */;
  Addr          Src;
  short         SrcMchn;
  short         DstMchn;
  Tag_t         Tag;
  int           Mode;                /* Control flags for different types of messages */
  void         *Payload;             /* Pointer to data */
  unsigned int  PayloadSize;         /* Size in bytes of the whole message if local, and the size of fragment if remote */
  unsigned int  MessageSize;         /* Size of whole message */
  char         *RrvDstAddr;

  void         *SrcThr;              /* Source thread      */
  void         *DstThr;              /* Destination thread (local only)  */
};
/* For sending messages to remote machines, the last two fields in Header are not used */
#define HEADER_NET_SZ  (sizeof(Header) - (2 * (sizeof(void *))))

struct RQST_Status {
  int   SrcMchn;
  Addr  Src;
  Tag_t Tag;
  int   Count;
  int   Cancelled;
  int   Error;
};
typedef struct RQST_Status RQST_Status, *RQST_Status_t;

typedef struct RQST_PendReg RQST_PendReg, *RQST_PendReg_t;

struct RQST_Obj {
  Header               Hdr;
  volatile int         State;   
  int                  Type          __attribute__(( aligned(CACHE_LINE_SIZE) )) ;    /* Do no remove the aligned attribute */
                                     /* Type of request: SEND/RECV, SYNC/ASYNC/PRST, etc.        */
  int                  SeqNr;
                                     /* Info on the sender of a satisfied receiving request      */
  void                *Owner;        /* Thread that set-up the request                           */
  char                *Buff;         /* User buffer of the request                               */
  int                  BuffSize;     /* Size of the user buffer                                  */
  char                *BuffInPtr;    /* Input pointer of the user buffer                         */
  int                  RankPrst;     /* Source address to satisfied a persistent request         */
  Tag_t                TagPrst;      /* Tag to satisfied a persistent request                    */
  char                *RrvAddr;
  int                  PersistentWait;
  int                  AsyncInWait;
  char                 CarriedData[1024];
/* AzqMPI support */
#ifdef AZQ_MPI
  unsigned int         MpiType;
  char                *PackedBuffer; /* Packed buffer if needed. For buffered always is used,    */
  int                  Count;        /*        for non-blocking is used if datatype is not       */
  void                *Datatype;     /*        contig., for persistent is ...                    */
  char                *OrigBuffer;   /* Only used in persistent request pointing original data   */
  void                *Comm;         /* Communicator associated to the request                   */
  int                  Index;        /* Integer (now index in the object structure) for supporting
                                        Fortran to/from C interface in MPI library */
  void                *Pool;
#endif
  RQST_Status          Status;     
  volatile int         SendBufferSwitched /*__attribute__(( aligned(CACHE_LINE_SIZE) ))*/ ;
  volatile int         SendCounter;  
  void                *SendBuffer;  
}; 
typedef struct RQST_Obj  Rqst,  *Rqst_t;

/*----------------------------------------------------------------*
 *   Declaration of public interface                              *
 *----------------------------------------------------------------*/
#define    RQST_setWaiting(rqst)     ((rqst)->AsyncInWait =  1)
#define    RQST_unsetWaiting(rqst)   ((rqst)->AsyncInWait =  0)
#define    RQST_isWaiting(rqst)      ((rqst)->AsyncInWait == 1)

#define    RQST_setSatisfied(rqst)   ((rqst)->State =  RQST_SATISFIED)
#define    RQST_isSatisfied(rqst)    ((rqst)->State == RQST_SATISFIED)

#define    RQST_setInactive(rqst)    ((rqst)->State =  RQST_INACTIVE)
#define    RQST_isInactive(rqst)     ((rqst)->State == RQST_INACTIVE)

#define    RQST_isPersistent(rqst)   ((rqst)->Type & RQST_PERSISTENT)

#define    RQST_setSplitCopy(rqst)   ( (rqst)->Type |= RQST_SPLITCOPY ) 
#define    RQST_isSplitCopy(rqst)    ( (rqst)->Type &  RQST_SPLITCOPY ) 

#define    RQST_setDataCarrier(rqst) ( (rqst)->Type |= RQST_DATA_CARRIER ) 
#define    RQST_isDataCarrier(rqst)  ( (rqst)->Type &  RQST_DATA_CARRIER ) 

#define    RQST_isRecv(rqst)         ((rqst)->Type & RQST_RECV )
#define    RQST_isSend(rqst)         ((rqst)->Type & RQST_SEND )

#define    RQST_isSync(rqst)         ( (rqst)->Type & RQST_SYNC )
#define    RQST_isAsync(rqst)        ( (rqst)->Type & RQST_ASYNC) 

#define    HDR_isSync(hdr)           ( ((Rqst_t)(hdr))->Type & RQST_SYNC )
#define    HDR_isAsync(hdr)          ( ((Rqst_t)(hdr))->Type & RQST_ASYNC )

#define    RQST_isRemote(rqst)       ( (rqst)->Hdr.Mode & MODE_REMOTE)   
#define    RQST_isLocal(rqst)        (!(RQST_isRemote(rqst)))  

#define    HDR_isRemote(hdr)         ((hdr)->Mode & MODE_REMOTE)   
#define    HDR_isLocal(hdr)          (!(HDR_isRemote(hdr)))   

#define    RQST_setState(rqst, state) { (rqst)->State = (state); }

#define    RQST_setSeqNr(rqst, seqNr) { (rqst)->SeqNr = (seqNr); }
#define    RQST_setOwner(rqst, thr)   { (rqst)->Owner = (thr); }

#define    RQST_isSendBufferSwitched(rqst)   ((rqst)->SendBufferSwitched)
#define    RQST_setSendBufferSwitched(rqst)  ((rqst)->SendBufferSwitched = 1)


      /*________________________________________________________________
     /                                                                  \
    |    RQST_getStatus                                                  |
    |                                                                    |
    |    Set status from a request stisfied                              |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o rqst         (Input)                                          |
    |        request                                                     |
    |    o status         (Input / Output)                               |
    |        Status                                                      |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
#define RQST_getStatus(rqst, status)  {                 \
    *((RQST_Status *)(status)) = (rqst)->Status;        \
}


      /*________________________________________________________________
     /                                                                  \
    |    RQST_setEmptyStatus                                             |
    |                                                                    |
    |    Set status to default values. Wait/test for a NULL requests     |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o status       (Input / Output)                                 |
    |        Status                                                      |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
#define    RQST_setEmptyStatus(status)         \
{                                              \
    (status)->SrcMchn   = -1;                  \
    (status)->Src.Rank  = ADDR_RNK_ANY;        \
    (status)->Tag       = TAG_ANY;             \
    (status)->Count     = 0;                   \
    (status)->Cancelled = FALSE;               \
    (status)->Error     = AZQ_SUCCESS;         \
}


      /*________________________________________________________________
     /                                                                  \
    |    Cancel requests                                                 |
    |                                                                    |
     \_________________________________________________________________*/

#define    RQST_setCancelled(rqst)   ((rqst)->State =  RQST_CANCELLED)
#define    RQST_isCancelled(rqst)    ((rqst)->State == RQST_CANCELLED)


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
#define old_RQST_match(rqst, keyRank, keyTag, mode)             \
(                                                           \
  RANK_match((keyRank), (rqst)->Status.Src.Rank)   &&       \
  TAG_MATCH((rqst)->Status.Tag, (keyTag))        &&         \
  (((mode) & MODE_REMOTE) ? (((rqst)->Type & RQST_RECV) ? (((rqst)->Type & RQST_MASK) & (mode)) : 1) : 1)  \
)

#define RQST_match(rqst, keyRank, keyTag, mode)             \
(                                                           \
  RANK_match((keyRank), (rqst)->Status.Src.Rank)   &&       \
  TAG_MATCH((rqst)->Status.Tag, (keyTag))        &&         \
  ((rqst)->Hdr.Mode & (mode))                               \
)


#define RQST_matchLocal(rqst, keyRank, keyTag)    \
(                                                       \
  RANK_match((keyRank), (rqst)->Status.Src.Rank)   &&   \
  TAG_MATCH((rqst)->Status.Tag, (keyTag))               \
)



      /*________________________________________________________________
     /                                                                  \
    |    RQST_clear                                                      |
    |                                                                    |
    |    Clear an asynchronous communicaton PERSISTENT request           |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o rqst (Input)                                                  |
    |        The request                                                 |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
#define RQST_clear(rqst)                           \
{                                                  \
  (rqst)->Status.Count     = 0;                  \
    (rqst)->Status.Src.Rank  = (rqst)->RankPrst;    \
  (rqst)->Status.Tag       = (rqst)->TagPrst;    \
  (rqst)->Status.Cancelled = FALSE;              \
  (rqst)->Status.SrcMchn   = DFLT_MCHN;          \
  (rqst)->BuffInPtr        = (rqst)->Buff;       \
  (rqst)->State = RQST_INITIALIZED;                \
}


      /*________________________________________________________________
     /                                                                  \
    |    RQST_init                                                       |
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

#define RQST_initSendLocal(rqst, myRank, tag, buf, count, me, seqNr)                        \
{                                                                                           \
  (rqst)->Hdr.Mode              = MODE_EAGER;                                               \
  (rqst)->Hdr.Src.Rank          = (myRank);                                                 \
  (rqst)->Hdr.Tag               = (tag);                                                    \
  (rqst)->Hdr.Payload           = (buf);                                                    \
  (rqst)->Hdr.MessageSize       = (count);                                                  \
  (rqst)->Owner                 = (me);                                                     \
  (rqst)->Status.Cancelled      = FALSE;                                                    \
  (rqst)->SeqNr                 = (seqNr);                                                  \
  (rqst)->State                 = RQST_PENDING;                                             \
  (rqst)->Type                  = RQST_SEND | RQST_SYNC;                                    \
  (rqst)->SendCounter           = 1;                                                        \
  (rqst)->SendBufferSwitched    = 0;                                                        \
}


#define RQST_initAsendLocal(rqst, myRank, tag, buf, count, me, seqNr)                       \
{                                                                                           \
  DBG_PRNT((stdout, "RQST_init(%p): Begin\n", self()));                                     \
  /*fprintf(stdout, "RQST_init(%p): Rqst %p. Begin\n", self(), (rqst)); fflush(stdout);*/   \
  (rqst)->Hdr.Mode              = MODE_EAGER | MODE_LAST_FRAGMENT;                          \
  (rqst)->Type                  = RQST_SEND | RQST_ASYNC;                                   \
  (rqst)->Hdr.Src.Rank          = (myRank);                                                 \
  (rqst)->Hdr.Tag               = (tag);                                                    \
  (rqst)->Hdr.Payload           = (buf);                                                    \
  (rqst)->Hdr.MessageSize       = (count);                                                  \
  (rqst)->Owner                 = (me);                                                     \
  (rqst)->Status.Cancelled      = FALSE;                                                    \
  (rqst)->SeqNr                 = (seqNr);                                                  \
  (rqst)->State                 = RQST_PENDING;                                             \
  (rqst)->SendCounter           = 1;                                                        \
  DBG_PRNT((stdout, "RQST_init(%p): rqst %p has state %x. End\n",                           \
           self(), (rqst), (rqst)->State));                                                 \
}


#ifdef USE_FASTBOXES

#define RQST_initSendAsync(rqst, type, dstAddr, myAddr, tag, buf, count, dstmchn, me, dstThr, seqNr)       \
{                                                                                           \
  DBG_PRNT((stdout, "RQST_init(%p): Begin\n", self()));                                     \
  /*fprintf(stdout, "RQST_init(%p): Rqst %p. Begin\n", self(), (rqst)); fflush(stdout);*/   \
  if (dstmchn == AZQ_cpuId) {                                                             \
    (rqst)->Hdr.Mode  = MODE_EAGER | MODE_LAST_FRAGMENT;                                               \
    (rqst)->Type      = (type);                                                           \
  }                                                                                       \
  else {                                                                                  \
    (rqst)->Hdr.Dst             = *(dstAddr);                                             \
    (rqst)->Hdr.Mode            = MODE_REMOTE;                                            \
    (rqst)->Hdr.DstMchn         = dstmchn;                                                \
    (rqst)->Hdr.MessageSize     = count;                                                  \
    (rqst)->Hdr.SrcMchn         = AZQ_cpuId;                                              \
    (rqst)->Hdr.RrvDstAddr      = NULL;                                                   \
    (rqst)->Status.Src.Rank      = (myAddr)->Rank;                                        \
    (rqst)->Status.Tag          = (tag);                                                  \
    (rqst)->Status.Count        = 0;                                                      \
    (rqst)->Status.Cancelled    = FALSE;                                                  \
    (rqst)->Type                = type;                                                   \
    (rqst)->SeqNr               = (seqNr);                                                \
                                                                                          \
    if ((count) >= RRV_THRESHOLD) {                                                              \
      (rqst)->Hdr.Mode           |= MODE_RRV_RQST;                                             \
    }                                                                                     \
  }                                                                                       \
  (rqst)->Hdr.Src               = *(myAddr);                                              \
  (rqst)->Hdr.Tag               = tag;                                                    \
  (rqst)->Hdr.Payload           = buf;                                                    \
  (rqst)->Hdr.PayloadSize       = count;                                                  \
  (rqst)->Owner                 = me;                                                     \
  (rqst)->Status.Cancelled      = FALSE;                                                  \
  (rqst)->State                 = RQST_PENDING;                                           \
  DBG_PRNT((stdout, "RQST_init(%p): rqst %p has state %x. End\n",                         \
           self(), (rqst), (rqst)->State));                                               \
}







#else
/* No fast boxes */


#define RQST_initSendAsync(rqst, type, dstAddr, myAddr, tag, buf, count, dstmchn, me, dstThr)   \
{                                                                                               \
  DBG_PRNT((stdout, "RQST_init(%p): Begin\n", self()));                                         \
  /*fprintf(stdout, "RQST_init(%p): Rqst %p. Begin\n", self(), (rqst)); fflush(stdout);*/       \
  if (dstmchn == AZQ_cpuId) {                                                             \
    (rqst)->Hdr.Mode  = MODE_EAGER | MODE_LAST_FRAGMENT;                                  \
    (rqst)->Type      = (type);                                                           \
  }                                                                                       \
  else {                                                                                  \
    (rqst)->Hdr.Dst             = *(dstAddr);                                             \
    (rqst)->Hdr.Mode            = MODE_REMOTE;                                            \
    (rqst)->Hdr.DstMchn         = dstmchn;                                                \
    (rqst)->Hdr.SrcMchn         = AZQ_cpuId;                                              \
    (rqst)->Hdr.RrvDstAddr      = NULL;                                                   \
    (rqst)->Status.Src.Rank     = (myAddr)->Rank;                                         \
    (rqst)->Status.Tag          = (tag);                                                  \
    (rqst)->Status.Count        = 0;                                                      \
    (rqst)->Status.Cancelled    = FALSE;                                                  \
    (rqst)->Type                = type;                                                   \
                                                                                          \
    if ((count) >= RRV_THRESHOLD) {                                                       \
      (rqst)->Hdr.Mode           |= MODE_RRV_RQST;                                        \
    }                                                                                     \
  }                                                                                       \
  (rqst)->Hdr.Src               = *(myAddr);                                              \
  (rqst)->Hdr.Tag               = tag;                                                    \
  (rqst)->Hdr.Payload           = buf;                                                    \
  (rqst)->Hdr.PayloadSize       = count;                                                  \
  (rqst)->Hdr.MessageSize       = count;                                                  \
  (rqst)->Owner                 = me;                                                     \
  (rqst)->Status.Cancelled      = FALSE;                                                  \
  (rqst)->State                 = RQST_PENDING;                                           \
  DBG_PRNT((stdout, "RQST_init(%p): rqst %p has state %x. End\n",                         \
           self(), (rqst), (rqst)->State));                                               \
}


#endif



#define RQST_initSsendRemote(rqst, type, dstRank, myAddr, tag, buf, count, dstmchn, me, dstThr)       \
{                                                                                           \
  DBG_PRNT((stdout, "RQST_init(%p): Begin\n", self()));                                     \
  /*fprintf(stdout, "RQST_init(%p): Rqst %p. Begin\n", self(), (rqst)); fflush(stdout);*/   \
  if (dstmchn != AZQ_cpuId) {                                                             \
    (rqst)->Status.Src          = *(myAddr);                                              \
    (rqst)->Status.Tag          = tag;                                                    \
    (rqst)->Status.Count        = 0;                                                      \
    (rqst)->Status.Cancelled    = FALSE;                                                  \
    (rqst)->Type                = type;                                                   \
    (rqst)->Hdr.Dst.Rank        = dstRank;                                                \
    (rqst)->Hdr.Dst.Group       = (myAddr)->Group;                                        \
    (rqst)->Hdr.Mode            = MODE_REMOTE;                                            \
    (rqst)->Hdr.DstMchn         = dstmchn;                                                \
    (rqst)->Hdr.SrcMchn         = AZQ_cpuId;                                              \
    (rqst)->Hdr.RrvDstAddr      = NULL;                                                   \
    /*if ((type) & RQST_RRV) {*/                                                          \
      (rqst)->Hdr.Mode           |= MODE_RRV_RQST;                                        \
    /*} */                                                                                \
  }                                                                                       \
  (rqst)->Hdr.Src               = *(myAddr);                                              \
  (rqst)->Hdr.Tag               = tag;                                                    \
  (rqst)->Hdr.Payload           = buf;                                                    \
  (rqst)->Hdr.PayloadSize       = count;                                                  \
  (rqst)->Hdr.MessageSize       = count;                                                  \
  (rqst)->State                 = RQST_PENDING;                                           \
  DBG_PRNT((stdout, "RQST_init(%p): rqst %p has state %x. End\n",                         \
           self(), (rqst), (rqst)->State));                                               \
}


#define RQST_initSendPersistent(rqst, type, dstAddr, myAddr, tag, buf, count, dstmchn, me, dstThr)       \
{                                                                                           \
  DBG_PRNT((stdout, "RQST_init(%p): Begin\n", self()));                                     \
  /*fprintf(stdout, "RQST_init(%p): Rqst %p. Begin\n", self(), (rqst)); fflush(stdout);*/   \
  if (dstmchn == AZQ_cpuId) {                                                             \
    (rqst)->Hdr.Mode      = MODE_EAGER | MODE_LAST_FRAGMENT;                                                  \
    (rqst)->Type   = (type);                                                              \
  }                                                                                       \
  else {                                                                                  \
    (rqst)->Hdr.Dst            = *(dstAddr);                                                      \
    (rqst)->Hdr.Mode           = MODE_REMOTE;                                                   \
    if ((type) & RQST_ASYNC) {                                                            \
      (rqst)->Status.Src.Rank   = (myAddr)->Rank;                                                   \
      (rqst)->Status.Tag       = tag;                                                     \
      (rqst)->Status.Count     = 0;                                                       \
      (rqst)->Status.Cancelled = FALSE;                                                   \
      (rqst)->Type   = type;                                                              \
    }                                                                                     \
    else {                                                                                     \
      (rqst)->Type   = 0;                                                                 \
    }                                                                                     \
    if ((count) >= RRV_THRESHOLD) {                                                              \
      (rqst)->Hdr.Mode           |= MODE_RRV_RQST;                                                    \
    }                                                                                     \
  }                                                                                       \
  (rqst)->Hdr.DstThr            = (dstThr);                                                      \
  (rqst)->Hdr.Src               = *(myAddr);                                                     \
  (rqst)->Hdr.SrcMchn           = AZQ_cpuId;                                                     \
  (rqst)->Hdr.DstMchn           = dstmchn;                                                       \
  (rqst)->Hdr.Tag               = tag;                                                           \
  (rqst)->Hdr.Payload           = buf;                                                           \
  (rqst)->Hdr.PayloadSize       = count;                                                         \
  (rqst)->Hdr.MessageSize       = count;                                                         \
  (rqst)->Hdr.RrvDstAddr        = NULL;                                                          \
  (rqst)->Owner                 = me;                                                          \
  (rqst)->Type |= RQST_PERSISTENT;                                                      \
  (rqst)->PersistentWait = FALSE;                                                           \
  (rqst)->State  = RQST_PENDING;                                                            \
  DBG_PRNT((stdout, "RQST_init(%p): rqst %p has state %x. End\n",                           \
           self(), (rqst), (rqst)->State));                                                 \
}




#define RQST_initRecvPersistent(rqst, type, keyRank, myAddr, keyTag, buf, count, dstmchn, me, dstThr)           \
{                                                                                           \
  DBG_PRNT((stdout, "RQST_init(%p): Begin\n", self()));                                     \
  /*fprintf(stdout, "RQST_init(%p): Rqst %p. Begin\n", self(), (rqst)); fflush(stdout);*/   \
  (rqst)->Owner            = me;                                                          \
  (rqst)->Status.Src.Rank  = (keyRank);                                                       \
  (rqst)->Status.Tag       = keyTag;                                                         \
  (rqst)->Status.Count     = 0;                                                           \
  (rqst)->Status.Cancelled = FALSE;                                                       \
  (rqst)->Buff             = buf;                                                         \
  (rqst)->BuffInPtr        = buf;                                                         \
  (rqst)->BuffSize         = count;                                                       \
  (rqst)->RrvAddr          = NULL;                                                        \
  (rqst)->Type             = type;                                                                  \
  (rqst)->RankPrst         = keyRank;                                                       \
  (rqst)->TagPrst          = keyTag;                                                         \
  (rqst)->PersistentWait   = FALSE;                                                           \
  (rqst)->State            = RQST_PENDING;                                                            \
  (rqst)->Hdr.Mode             = MODE_ANY;                                                        \
  DBG_PRNT((stdout, "RQST_initRecv(%p): rqst %p has state %x. End\n",                           \
           self(), (rqst), (rqst)->State));                                                 \
}



      /*________________________________________________________________
     /                                                                  \
    |    RQST_feed                                                       |
    |                                                                    |
    |      A fragment of a message, described by "hdr", has arrived      |
    |    that matches the "dstRqst" request. Copy the data of this       |
    |    fragment to the user buffer associated to "dstRqst".            |
    |      Info on the message and its origin is built inside "dstRqst". |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o dstRqst  (Input/Output)                                       |
    |        Target request                                              |
    |    o hdr      (Input/Output)                                       |
    |        message descriptor                                          |
    |                                                                    |
    |    RETURN:                                                         |
    |    = 0  : On success                                               |
    |    <  0 : On error                                                 |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
#define RQST_feed(rqst, hdr) \
{ \
  if (!((hdr)->Mode & MODE_NO_COPY)) {\
    if( (hdr)->PayloadSize) \
      MEMCPY((rqst)->BuffInPtr, (hdr)->Payload, (hdr)->PayloadSize); \
  } \
  (rqst)->Status.Count += (hdr)->PayloadSize; \
  (rqst)->Status.Src    = (hdr)->Src; \
  (rqst)->Status.Tag    = (hdr)->Tag; \
  if((hdr)->Mode & MODE_LAST_FRAGMENT) { \
    (rqst)->Status.SrcMchn = (hdr)->SrcMchn; \
  } else { \
    (rqst)->BuffInPtr += (hdr)->PayloadSize; \
  } \
}



#define RQST_feedLocal(rqst, hdr) \
{ \
  MEMCPY((rqst)->BuffInPtr, (hdr)->Payload, (hdr)->MessageSize); \
  (rqst)->Status.Count    = (hdr)->MessageSize; \
  (rqst)->Status.Src.Rank = (hdr)->Src.Rank; \
  (rqst)->Status.Tag      = (hdr)->Tag; \
}



#define RQST_halfFeedLocal(rqst, srcBuf, wholeSize, srcRank, srcTag) \
{ \
  MEMCPY((rqst)->BuffInPtr + (wholeSize>>1), (srcBuf) + (wholeSize>>1), (1 & wholeSize ? (wholeSize>>1) + 1 : (wholeSize>>1))); \
  (rqst)->Status.Count    = wholeSize; \
  (rqst)->Status.Src.Rank = srcRank; \
  (rqst)->Status.Tag      = srcTag; \
}



#define RQST_feedFromFBox(rqst, fBox) \
{ \
  MEMCPY((rqst)->BuffInPtr, (fBox)->Payload, (fBox)->MessageSize); \
  (rqst)->Status.Count    = (fBox)->MessageSize; \
  (rqst)->Status.Src.Rank = (fBox)->SenderGlobalRank; \
  (rqst)->Status.Tag      = (fBox)->Tag; \
}






#define RQST_initSendRemote(rqst, dstRank, tag, buf, count, me)                         \
{                                                                                       \
  DBG_PRNT((stdout, "RQST_initSendRemote(%p): Rqst %p  (start)\n", self(), (rqst)));    \
  (rqst)->Status.Src          = (me)->Address;                                          \
  (rqst)->Status.Tag          = tag;                                                    \
  (rqst)->Status.Count        = 0;                                                      \
  (rqst)->Status.Cancelled    = FALSE;                                                  \
  (rqst)->Type                = RQST_SEND | RQST_SYNC;                                  \
  (rqst)->Hdr.Dst.Rank        = (dstRank);                                              \
  /********* RICO *********/    (rqst)->Hdr.Dst.Group       = me->Address.Group;        \
  /********* RICO *********/    (rqst)->Hdr.Src = (me)->Address;                        \
  (rqst)->Hdr.Mode            = MODE_REMOTE;                                            \
  (rqst)->Hdr.DstMchn         = ((me)->GrpInfo)[(dstRank)].Mchn;                        \
  (rqst)->Hdr.MessageSize     = count;                                                  \
  (rqst)->Hdr.SrcMchn         = AZQ_cpuId;                                              \
  (rqst)->Hdr.RrvDstAddr      = NULL;                                                   \
  if ((count) >= RRV_THRESHOLD) {                                                       \
    (rqst)->Hdr.Mode           |= MODE_RRV_RQST;                                        \
  } else {                                                                              \
    (rqst)->Hdr.Mode           |= MODE_EAGER;                                           \
  }                                                                                     \
  (rqst)->Hdr.Tag               = tag;                                                  \
  (rqst)->Hdr.Payload           = buf;                                                  \
  (rqst)->Hdr.PayloadSize       = count;                                                \
  (rqst)->State                 = RQST_PENDING;                                         \
  (rqst)->Owner                 = me;                                                   \
  DBG_PRNT((stdout, "RQST_initSendRemote(%p): Rqst %p  (end)\n", self(), (rqst)));      \
}


#define RQST_initASendRemote_2(rqst, dstRank, tag, buf, count, me)                      \
{                                                                                       \
  DBG_PRNT((stdout, "RQST_initASendRemote(%p): Rqst %p  (start)\n", self(), (rqst)));   \
  (rqst)->Status.Src          = (me)->Address;                                          \
  (rqst)->Status.Tag          = tag;                                                    \
  (rqst)->Status.Count        = 0;                                                      \
  (rqst)->Status.Cancelled    = FALSE;                                                  \
  (rqst)->Type                = RQST_SEND | RQST_ASYNC;                                 \
  (rqst)->Hdr.Dst.Rank        = (dstRank);                                              \
  /********* RICO *********/    (rqst)->Hdr.Dst.Group       = me->Address.Group;        \
  /********* RICO *********/    (rqst)->Hdr.Src = (me)->Address;                        \
  (rqst)->Hdr.Mode            = MODE_REMOTE;                                            \
  (rqst)->Hdr.DstMchn         = ((me)->GrpInfo)[(dstRank)].Mchn;                        \
  (rqst)->Hdr.MessageSize     = count;                                                  \
  (rqst)->Hdr.SrcMchn         = AZQ_cpuId;                                              \
  (rqst)->Hdr.RrvDstAddr      = NULL;                                                   \
  if ((count) >= RRV_THRESHOLD) {                                                       \
    (rqst)->Hdr.Mode           |= MODE_RRV_RQST;                                        \
  } else {                                                                              \
    (rqst)->Hdr.Mode           |= MODE_EAGER;                                           \
  }                                                                                     \
  (rqst)->Hdr.Tag               = tag;                                                  \
  (rqst)->Hdr.Payload           = buf;                                                  \
  (rqst)->Hdr.PayloadSize       = count;                                                \
  (rqst)->State                 = RQST_PENDING;                                         \
  (rqst)->Owner                 = me;                                                   \
  DBG_PRNT((stdout, "RQST_initASendRemote(%p): Rqst %p  (end)\n", self(), (rqst)));     \
}


#define RQST_initRecv(rqst, srcRank, keyTag, buf, count, me)           \
{                                                                                           \
  DBG_PRNT((stdout, "RQST_initRecv(%p): Rqst %p (start)\n", self(), (rqst)));   \
  (rqst)->Owner            = (me);                                                          \
  (rqst)->Status.Src.Rank  = (srcRank);                                            \
  /********* RICO *********/  (rqst)->Status.Src.Group  = me->Address.Group;              \
  (rqst)->Status.Tag       = (keyTag);                                             \
  (rqst)->Status.Count     = 0;                                                           \
  (rqst)->Status.Cancelled = FALSE;                                                       \
  (rqst)->Buff             = (buf);                                                         \
  (rqst)->BuffInPtr        = (buf);                                                         \
  (rqst)->BuffSize         = (count);                                                       \
  (rqst)->RrvAddr          = NULL;                                                        \
  (rqst)->Type             = RQST_RECV | RQST_SYNC;                             \
  (rqst)->State            = RQST_PENDING;                                                \
  (rqst)->Hdr.Mode         = MODE_ANY; \
  DBG_PRNT((stdout, "RQST_initRecv(%p): Rqst %p (end)\n", self(), (rqst)));   \
}


#define RQST_initProbeRecv(rqst, srcRank, keyTag, buf, count, me)           \
{                                                                                           \
DBG_PRNT((stdout, "RQST_initRecv(%p): Rqst %p (start)\n", self(), (rqst)));   \
(rqst)->Owner            = (me);                                                          \
(rqst)->Status.Src.Rank  = (srcRank);                                            \
/********* RICO *********/  (rqst)->Status.Src.Group  = me->Address.Group;              \
(rqst)->Status.Tag       = (keyTag);                                             \
(rqst)->Status.Count     = 0;                                                           \
(rqst)->Status.Cancelled = FALSE;                                                       \
(rqst)->Buff             = (buf);                                                         \
(rqst)->BuffInPtr        = (buf);                                                         \
(rqst)->BuffSize         = (count);                                                       \
(rqst)->RrvAddr          = NULL;                                                        \
(rqst)->Type             = RQST_RECV | RQST_SYNC | RQST_PROBE;                             \
(rqst)->State            = RQST_PENDING;                                                \
(rqst)->Hdr.Mode         = MODE_ANY; \
DBG_PRNT((stdout, "RQST_initRecv(%p): Rqst %p (end)\n", self(), (rqst)));   \
}


#define RQST_initARecv(rqst, srcRank, keyTag, buf, count, me)           \
{                                                                                           \
DBG_PRNT((stdout, "RQST_initARecv(%p): Rqst %p (start)\n", self(), (rqst)));   \
(rqst)->Owner            = (me);                                                          \
(rqst)->Status.Src.Rank  = (srcRank);                                            \
/********* RICO *********/  (rqst)->Status.Src.Group  = me->Address.Group;                                            \
(rqst)->Status.Tag       = (keyTag);                                                         \
(rqst)->Status.Count     = 0;                                                           \
(rqst)->Status.Cancelled = FALSE;                                                       \
(rqst)->Buff             = (buf);                                                         \
(rqst)->BuffInPtr        = (buf);                                                         \
(rqst)->BuffSize         = (count);                                                       \
(rqst)->RrvAddr          = NULL;                                                        \
(rqst)->Type             = RQST_RECV | RQST_ASYNC;                             \
(rqst)->State            = RQST_PENDING;                                                \
(rqst)->Hdr.Mode         = MODE_ANY; \
DBG_PRNT((stdout, "RQST_initARecv(%p): Rqst %p (end)\n", self(), (rqst)));   \
}







/*----------------------------------------------------------------*
 *   Declaration of public interface implemented by this module   *
 *----------------------------------------------------------------*/
extern void  RQST_printHdr    (Header_t hdr);


#endif
