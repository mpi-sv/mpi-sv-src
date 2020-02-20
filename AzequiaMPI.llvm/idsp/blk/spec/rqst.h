/*-
 * Copyright (c) 2009 Universidad de Extremadura
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
#include <addr.h>
#include <dlq.h>
#include <util.h>

/*----------------------------------------------------------------*
 *   Exported constants                                           *
 *----------------------------------------------------------------*/

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
#define DBG_PRNT(pmsg)    fprintf pmsg
#define self()          ((Thr_t)pthread_getspecific(key))
#else
#define DBG_PRNT(pmsg)
#endif

/*----------------------------------------------------------------*
 *   Exported types                                               *
 *----------------------------------------------------------------*/
/* Bits of "state" of a Request */
#define RQST_INITIALIZED          0x00000001
#define RQST_SATISFIED            0x00000002
#define RQST_PENDING              0x00000004
//#define RQST_FILLING              0x00000008
#define RQST_MAKES_WAITING        0x00000010
#define RQST_DESTROYED            0x00000020
#define RQST_CANCELLED            0x00000040

/* Bits of "type" argument of RQST_init */
#define RQST_RECV                 0x00000001
#define RQST_SEND                 0x00000002
#define RQST_SYNC                 0x00000004
#define RQST_ASYNC                0x00000008
#define RQST_PERSISTENT           0x00000010
#define RQST_SLM                  0x00000020
#define RQST_RRV                  0x00000040
#define RQST_ACKED                0x00000080
#define RQST_PROBE                0x00000100

#define RQST_ANY                  0x70000000
#define RQST_RRV_RQST             0x10000000
#define RQST_RRV_DATA             0x20000000
#define RQST_DATA                 0x40000000
#define RQST_MASK                 RQST_ANY

/* Mode Field of Header */
#define MODE_LAST_FRAGMENT        0x00000001
#define MODE_REMOTE               0x00000002
#define MODE_SLM                  0x00000004
#define MODE_RRV                  0x00000008
#define MODE_RRV_ACK              0x00000020
#define MODE_NO_COPY              0x00000040

#define MODE_RRV_RQST             0x10000000
#define MODE_RRV_DATA             0x20000000
#define MODE_DATA                 0x40000000

typedef struct Header Header, *Header_t;
struct Header {
  Link          Link;
  Addr          Dst;
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
  void         *SrcRqst;             /* Source request     */
  void         *DstThr;              /* Destination thread (local only)  */
};
/* For sending messages to remote machines, the last three fields in Header are not used */
#define HEADER_NET_SZ  (sizeof(Header) - (3 * (sizeof(void *))))

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
  Link                 Link;         /* Link of requests setup by asynchrounous send/recv        */
  int                  Type;         /* Type of request: SEND/RECV, SYNC/ASYNC/PRST, etc.        */
  int                  State;        /* Bits for [INITED, PENDING, SATSFED, MAKES_WAIT, DESTROY] */
  int                  WaitIdx;      /* Which rqst was satisfied on the return of waitany        */
  RQST_Status          Status;       /* Info on the sender of a satisfied receiving request      */
  void                *Owner;        /* Thread that set-up the request                           */
  void                *DstThr;       /* Send requests only: Destination Thread                   */
  char                *Buff;         /* User buffer of the request                               */
  int                  BuffSize;     /* Size of the user buffer                                  */
  char                *BuffInPtr;    /* Input pointer of the user buffer                         */
  Addr                 SrcPrst;      /* Source address to satisfied a persistent request         */
  Tag_t                TagPrst;      /* Tag to satisfied a persistent request                    */
  Header               Hdr;
  char                *RrvAddr;

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
#endif
};
typedef struct RQST_Obj  Rqst,  *Rqst_t;

/*----------------------------------------------------------------*
 *   Declaration of public interface                              *
 *----------------------------------------------------------------*/
#define    RQST_setSatisfied(rqst)   ((rqst)->State =  RQST_SATISFIED)
#define    RQST_isSatisfied(rqst)    ((rqst)->State == RQST_SATISFIED)

#define    RQST_setInactive(rqst)    ((rqst) = (Rqst_t)AZQ_RQST_NULL)
#define    RQST_isInactive(rqst)     (((rqst)->State == RQST_DESTROYED) || ((rqst)->State == RQST_INITIALIZED))

#define    RQST_isPersistent(rqst)   ((rqst)->Type & RQST_PERSISTENT)

#define    RQST_isRecv(rqst)         ((rqst)->Type & RQST_RECV)

#define    RQST_isSlm(rqst)          ((rqst)->Type & RQST_SLM)

#define RQST_setState(rqst, state) {                    \
  DBG_PRNT((stdout, "(%p): Setting State of request %p from %d to %d\n", self(), rqst, (rqst)->State, state)); \
  (rqst)->State = state;                                \
}

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
  if((status) != AZQ_STATUS_IGNORE) {                   \
    if (RQST_isCancelled(rqst)) {                       \
      (status)->Cancelled = TRUE;                       \
    } else {                                            \
      if((rqst)->Type & RQST_RECV) {                    \
        *((RQST_Status *)(status)) = (rqst)->Status;    \
      }                                                 \
    }                                                   \
  }                                                     \
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
  if ((status) != AZQ_STATUS_IGNORE) {         \
    (status)->SrcMchn   = -1;                  \
    (status)->Src.Group = ADDR_GRP_ANY;        \
    (status)->Src.Rank  = ADDR_RNK_ANY;        \
    (status)->Tag       = TAG_ANY;             \
    (status)->Count     = 0;                   \
    (status)->Cancelled = FALSE;               \
    (status)->Error     = AZQ_SUCCESS;         \
  }                                            \
}


      /*________________________________________________________________
     /                                                                  \
    |    Cancel requests                                                 |
    |                                                                    |
     \_________________________________________________________________*/

#define    RQST_setCancelled(rqst)   ((rqst)->State =  RQST_CANCELLED)
#define    RQST_isCancelled(rqst)    ((rqst)->State == RQST_CANCELLED)

/* For a request to be cancellable must happen that the request is in PENDING state and:
                   If receive request, must be empty (not partially filled)
                   If send request, always is cancellable if pending (remote case chack out of here)
 */
#define    RQST_isCancellable(rqst)  ( ( (rqst)->State == RQST_PENDING )                  &&       \
                                       ( ((rqst)->Hdr.Mode & RQST_RECV)                    ?       \
                                               ( ((rqst)->Status.Count  == 0) &&                   \
                                                 (!((rqst)->Type & RQST_ACKED))                    \
                                               )               :                                   \
                                               ( 1 )                                               \
                                       )                                                           \
                                     )

      /*________________________________________________________________
     /                                                                  \
    |    RQST_match                                                      |
    |                                                                    |
    |    Test is a request matches a "addr" and a "tag"                  |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |    o rqst         (Input)                                          |
    |        request                                                     |
    |    o addr, tag    (Input)                                          |
    |        keys                                                        |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
#define RQST_match(rqst, addr, tag, mode)                                \
(                                                                        \
  (((rqst)->State == RQST_PENDING)          ||                           \
   ((rqst)->State == RQST_MAKES_WAITING))   &&                           \
  ADDR_match(addr, &((rqst)->Status.Src))   &&                           \
  TAG_MATCH((rqst)->Status.Tag, tag)        &&                           \
  ((mode & MODE_REMOTE) ? (((rqst)->Type & RQST_RECV) ? (((rqst)->Type & RQST_MASK) & mode) : 1) : 1)  \
)

      /*________________________________________________________________
     /                                                                  \
    |    RQST_close                                                      |
    |                                                                    |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
#define RQST_destroy(rqst)       ((rqst)->State = RQST_DESTROYED)


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
  if ((rqst)->Type & RQST_SEND) {                  \
    ;                                              \
  } else {                                         \
    (rqst)->Status.Count     = 0;                  \
    (rqst)->Status.Src       = (rqst)->SrcPrst;    \
    (rqst)->Status.Tag       = (rqst)->TagPrst;    \
    (rqst)->Status.Cancelled = FALSE;              \
    (rqst)->Status.SrcMchn   = DFLT_MCHN;          \
    (rqst)->BuffInPtr        = (rqst)->Buff;       \
  }                                                \
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
#define RQST_init(rqst, type, addr, tag, buf, count, dstmchn, me, dstThr)                   \
{                                                                                           \
  Header_t   hdr;                                                                           \
                                                                                            \
  DBG_PRNT((stdout, "RQST_init(%p): Begin\n", self()));                                     \
                                                                                            \
  if((type) & RQST_RECV) {                                                                  \
                                                                                            \
    (rqst)->Owner            = me;                                                          \
    (rqst)->Status.Src       = *addr;                                                       \
    (rqst)->Status.Tag       = tag;                                                         \
    (rqst)->Status.Count     = 0;                                                           \
    (rqst)->Status.Cancelled = FALSE;                                                       \
    (rqst)->Buff             = buf;                                                         \
    (rqst)->BuffInPtr        = buf;                                                         \
    (rqst)->BuffSize         = count;                                                       \
    (rqst)->RrvAddr          = NULL;                                                        \
                                                                                            \
    if ((type) & RQST_PERSISTENT) {                                                         \
      (rqst)->SrcPrst        = *addr;                                                       \
      (rqst)->TagPrst        = tag;                                                         \
    }                                                                                       \
                                                                                            \
    (rqst)->Type   = type;                                                                  \
                                                                                            \
  } else if ((type) & RQST_SEND) {                                                          \
                                                                                            \
    (rqst)->Owner            = me;                                                          \
    (rqst)->DstThr           = dstThr;                                                      \
                                                                                            \
    if (INET_by(dstmchn)) {                                                                 \
                                                                                            \
      if ((type) & RQST_ASYNC) {                                                            \
        (rqst)->Owner            = me;                                                      \
        (rqst)->Status.Src       = *addr;                                                   \
        (rqst)->Status.Tag       = tag;                                                     \
        (rqst)->Status.Count     = 0;                                                       \
        (rqst)->Status.Cancelled = FALSE;                                                   \
      }                                                                                     \
                                                                                            \
      hdr                  = &((rqst)->Hdr);                                                \
      hdr->Mode            = MODE_REMOTE;                                                   \
                                                                                            \
      if ((type) & RQST_RRV) {                                                              \
        hdr->Mode         |= MODE_RRV;                                                      \
      }                                                                                     \
                                                                                            \
      if ((type) & RQST_ASYNC) {                                                            \
        (rqst)->Type   = type;                                                              \
      } else {                                                                              \
        (rqst)->Type   = 0;                                                                 \
      }                                                                                     \
                                                                                            \
    } else {                                                                                \
                                                                                            \
      if((type) & RQST_SLM) {                                                               \
        hdr                  = &(((Thr_t)me)->SlmSet->Hdr[((Thr_t)me)->SlmSet->Turno]);     \
        hdr->Mode            = MODE_LAST_FRAGMENT | MODE_SLM;                               \
        (rqst)->Type   = RQST_SLM;                                                          \
      } else {                                                                              \
        hdr                  = &(rqst->Hdr);                                                \
        hdr->Mode            = MODE_LAST_FRAGMENT;                                          \
        (rqst)->Type   = 0;                                                                 \
      }                                                                                     \
                                                                                            \
    }                                                                                       \
                                                                                            \
    if((type) & RQST_PERSISTENT)                                                            \
      (rqst)->Type |= RQST_PERSISTENT;                                                      \
                                                                                            \
    hdr->Dst               = *addr;                                                         \
    hdr->Src               = *getAddr();                                                    \
    hdr->SrcThr            = (void *)me;                                                    \
    hdr->SrcMchn           = INET_getCpuId();                                               \
    hdr->DstMchn           = dstmchn;                                                       \
    hdr->SrcRqst           = (void *)(rqst);                                                \
    hdr->Tag               = tag;                                                           \
    hdr->Payload           = buf;                                                           \
    hdr->PayloadSize       = count;                                                         \
    hdr->MessageSize       = count;                                                         \
    hdr->RrvDstAddr        = NULL;                                                          \
                                                                                            \
  }                                                                                         \
                                                                                            \
  (rqst)->State  = RQST_INITIALIZED;                                                        \
                                                                                            \
  DBG_PRNT((stdout, "RQST_init(%p): rqst %p has state %x\n",                                \
           self(), rqst, (rqst)->State));                                                   \
                                                                                            \
}


/*----------------------------------------------------------------*
 *   Declaration of public interface implemented by this module   *
 *----------------------------------------------------------------*/
extern int   RQST_ready      (void *dstThr, Rqst_t  rqst);
extern int   RQST_feed       (Rqst_t rqst, Header_t hdr);


#endif
