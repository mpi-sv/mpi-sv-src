/*-
 * Copyright (c) 2009-2011 Universidad de Extremadura
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
#include <config.h>
#include <pmi_interface.h>

#if defined (__OSI)
  #include <osi.h>
#else
  #include <pthread.h>
#endif

#include <addr.h>
#include <rqst.h>
#include <dlq.h>
#include <lfq.h>

/*----------------------------------------------------------------*
 *   1. Constants                                                 *
 *----------------------------------------------------------------*/
#define ARGV_NR                  8      /* Maximum number of strings in argv */
#define ARGV_SIZE_MAX           32      /* Maximum size (bytes) of all string in argv */

/* Bits of "State" field */
#define ALLOCATED       0x00000001
#define STARTED         0x00000004
#define SIGNALED        0x00000008
#define SIGPEND         0x00000010
#define SIGMASK         0x00000020
#define ZOMBIE          0x00000040


/*----------------------------------------------------------------*
 *   2. Data types                                                *
 *----------------------------------------------------------------*/
typedef struct Thr Thr, *Thr_t;

/* MchnThr_t should not be delared here, but in grp.c  
 * Performance made us to commit this crime against beauty and power of software architecture
 * Stuttgart. July 20, 2010  
 */
#define MchnThrSize (                           \
                      3 * sizeof(int)         + \
                          sizeof(Thr_t)       + \
                          sizeof(Placement_t)   \
                    )

struct MchnThr {
  int          Mchn;
  int          Sigmask;
  int          ExitCode;
  Thr_t        Thr;
  Placement_t  Placement;
};
typedef struct MchnThr MchnThr, *MchnThr_t;


/* FastBox */
#ifdef USE_FASTBOXES                /* See rqst.h */
#define FBOX_BUF_MAX (1024)
#define FBOX_MAX       16  

struct fastBox {
//volatile
  int   Turn                  __attribute__(( aligned(CACHE_LINE_SIZE) ));
  int   Tag;
  int   SenderGlobalRank;    /* Global rank of Rank sending here */  
  int   SenderLocalRank;     /* Local  rank of Rank sending here */  
  int   SeqNr;
  int   MessageSize;
  char  Payload[FBOX_BUF_MAX]  ;
};
typedef struct fastBox fastBox, *fastBox_t;
#endif


struct Thr {
  DLQ               RecvPendReg        ;
  DLQ               MailBox            ;
  LFQ               PubMailBox         ;
  Rqst              SyncRqst           ;
  DLQ               AsendPendReg       ;
#ifdef USE_FASTBOXES
  fastBox           FastBox  [FBOX_MAX];
  int               SendSeqNr[FBOX_MAX];
  int               LastRecvSeqNr[FBOX_MAX];
  int               LocalGroupSize;
#endif
  MchnThr_t         GrpInfo;
  int               LocalRank;
  Addr              Address;
  pthread_t         Self               ;
  pthread_mutex_t   Lock               ;
  pthread_cond_t    Ready              ;
  pthread_attr_t    Attr               ;
  Placement_t       Placement;
  int             (*Body_Fxn)(int, char **);
  int               Stack_Size;
  int               ExitCode;
  int               State;
  int               ClientMchn;             /* Only           */
  Addr              ClientAddr;             /* for servers    */
//volatile int      SpinCounter        ;
  int               Argc;
  char            **Argv;
  //RqstTable        *RqstTable;
  void             *RqstTable;
};


#endif
