/*-
 * Copyright (c) 2009 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */


#ifndef _THR_DPTR_H_
#define _THR_DPTR_H_

/*----------------------------------------------------------------*
 *   Declaration of types used by this module                     *
 *----------------------------------------------------------------*/
#include <pmi_interface.h>

#if defined (__OSI)
  #include <osi.h>
#else
  #include <pthread.h>
#endif

#include <addr.h>
#include <rqst.h>
#include <dlq.h>

/*----------------------------------------------------------------*
 *   1. Constants                                                 *
 *----------------------------------------------------------------*/
#define ARGV_NR                  8      /* Maximum number of strings in argv */
#define ARGV_SIZE_MAX           32      /* Maximum size (bytes) of all string in argv */

/* Bits of "State" field */
#define ALLOCATED       0x00000001
#define SLM_ENABLED     0x00000002
#define STARTED         0x00000004
#define SIGNALED        0x00000008
#define SIGPEND         0x00000010
#define SIGMASK         0x00000020
#define ZOMBIE          0x00000040


/* Buffers of Short Local Messages */
#define SLM_BUFFER_SIZE     (16 * 1024)   /* What it is considered a "small" message.
                                             Up to SLM_MAX bytes */
#define SLM_BUFFERS_PER_THREAD      16    /* Each thread willing to use Short Local
                                             Messages (SLM) can allocate a Set of buffers.
                                             The Set has SLM_BUFFER_MAX Buffers. */
#define SLM_SET_ALLOCATED   0x00000001
#define SLM_SET_FULL        0x00000002

struct SlmSet {
  int              State;
  int              Turno;
  volatile int     Cnt;
  Header           Hdr[SLM_BUFFERS_PER_THREAD];
  char             Buffer[SLM_BUFFERS_PER_THREAD][SLM_BUFFER_SIZE];
};
typedef struct SlmSet SlmSet, *SlmSet_t;


/*----------------------------------------------------------------*
 *   2. Data types                                                *
 *----------------------------------------------------------------*/

/* Thread descriptor */
#define CACHE_LINE_SIZE 64
typedef struct Thr Thr, *Thr_t;

#define ThrSize (sizeof (pthread_t)              + \
                 sizeof (pthread_mutex_t)        + \
                 sizeof (pthread_cond_t)         + \
                 sizeof (pthread_attr_t)         + \
                 sizeof (Rqst_t *)               + \
                 sizeof (Rqst)                   + \
             3 * sizeof (DLQ_t)                  + \
                 sizeof (SlmSet_t)               + \
                 sizeof (void *)                 + \
                 sizeof (Placement_t)            + \
                 sizeof (int (*)(int, char **))  + \
            10 * sizeof (int)                    + \
                 sizeof (char**)                 + \
                 sizeof (int)                    + \
             2 * sizeof (Addr)                   + \
                 sizeof (int)                      \
               )


struct Thr {
  pthread_t         Self;
  pthread_mutex_t   Lock;
  pthread_cond_t    Ready;
  pthread_attr_t    Attr;
  Rqst_t           *WaitAnyRqstVector;
  Rqst              SyncRqst;
  DLQ_t             RecvPendReg;
  DLQ_t             AsendPendReg;
  DLQ_t             MailBox;
  SlmSet_t          SlmSet;
  void             *GrpInfo;
  Placement_t       Placement;
  int             (*Body_Fxn)(int, char **);
  int               Stack_Size;
  int               ExitCode;
  int               LocalRank;
  int               WaitRqstLeftCnt;
  int               WaitAnyRqstCnt;
  int               RqstAsyncCnt;
  int               SatisfiedRqst;
  int               State;
  volatile int      SpinCounter;
  int               Argc;
  char            **Argv;
  int               ClientMchn;            /* Only           */
  Addr              ClientAddr;            /* for servers    */
  Addr              Address;
  char              Pad[(ThrSize%CACHE_LINE_SIZE ? CACHE_LINE_SIZE - (ThrSize%CACHE_LINE_SIZE) : 0)];
};
//Herman commented. What's this? seems that it do not make any sense.
//char a[sizeof(Thr) % CACHE_LINE_SIZE ? -1 : CACHE_LINE_SIZE];


#endif
