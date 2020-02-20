/*-
 * Copyright (c) 2009 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */


#ifndef _COM_H_
#define _COM_H_

/*----------------------------------------------------------------*
 *   Declaration of types used by this module                     *
 *----------------------------------------------------------------*/
#if defined (__OSI)
  #include <osi.h>
#else
  #include <pthread.h>
#endif

#include <addr.h>
#include <inet.h>
#include <rqst.h>
#include <thr_dptr.h>

/*----------------------------------------------------------------*
 *   Constants                                                    *
 *----------------------------------------------------------------*/
/* No timeout */
#define COM_FOREVER       0xFFFFFFFF

/* Communication modes:
   Blocking send uses by default (if enable in thread) SLM for local messages and RRV only for large messages
   Blocking synchronous send uses by default NO SLM and always RRV
*/
#define RRV_MODE          RQST_RRV
#define SLM_MODE          RQST_SLM

/* Remote Rendevouz Protocol packet thresold size in bytes */
#define RRV_THRESHOLD     (128 * 1024)

/* Protocol argument */
#define COM_PROTOCOL      ((Protocol_t)0x0000)

/* Wait and test */
#define NONE_SATISFIED    (-1)

    /*----------------------------------------------------------------*
     *   Exported exceptions                                          *
     *----------------------------------------------------------------*/
#define COM_E_OK           0
#define COM_E_EXHAUST     (COM_E_OK          - 1)
#define COM_E_INTEGRITY   (COM_E_EXHAUST     - 1)
#define COM_E_TIMEOUT     (COM_E_INTEGRITY   - 1)
#define COM_E_INTERFACE   (COM_E_TIMEOUT     - 1)
#define COM_E_SYSTEM      (COM_E_INTERFACE   - 1)
#define COM_E_SIGNALED    (COM_E_SYSTEM      - 1)
#define COM_E_DEADPART    (COM_E_SIGNALED    - 1)

/* Error field of Status on failed multi-request wait operations */
#define AZQ_WAIT_SUCCESS          0
#define AZQ_WAIT_ERR_PENDING      7

/* Status argument of multi-request wait operations */
#define AZQ_STATUS_IGNORE         (NULL)
#define AZQ_STATUSES_IGNORE       (NULL)

/* Outcnt output argument of waitsome operation when all the requests are NULL */
#define AZQ_UNDEFINED             (-1)

/* rqst[i] output value (deallocated request) of multi-request wait operations */
#define AZQ_RQST_NULL             (NULL)

/* Error codes */
extern int azq_err[10];

/* Debug messages */
#ifdef __COM_DEBUG
#define DBG_PRNT(pmsg)  fprintf pmsg
#else
#define DBG_PRNT(pmsg)
#endif

/*----------------------------------------------------------------*
 *   Types                                                        *
 *----------------------------------------------------------------*/
struct Status {
  int   SrcMchn;
  Addr  Src;
  Tag_t Tag;
  int   Count;
  int   Cancelled;
  int   Error;
};
typedef struct Status Status;

/*----------------------------------------------------------------*
 *   3. Declaration of PRIVATE EXPORTED interface                 *
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------*
 *   SYNCHRONIZATION private interface                            *
 *----------------------------------------------------------------*/
/* Specifica data for thread descriptor access */
extern pthread_key_t  key;

#define LOCK(thr)           {                                                     \
                                if(pthread_mutex_lock(&((thr)->Lock)))            \
                                  panic("LOCK");                                  \
                            }

#define UNLOCK(thr)         {                                                     \
                                if(pthread_mutex_unlock(&((thr)->Lock)))          \
                                  panic("LOCK");                                  \
                            }

#define SIGNAL(thr)         {                                                     \
                              if(pthread_cond_signal(&((thr)->Ready)))            \
                                panic("SIGNAL");                                  \
                            }

/*----------------------------------------------------------------*
 *   SLM_SET private interface                                    *
 *----------------------------------------------------------------*/
#define SLM_SET_full(set)                        \
(                                                \
  ((set)->State & SLM_SET_FULL) ? TRUE : FALSE   \
)


#define SLM_SET_free(set)                        \
{                                                \
  (set)->Cnt--;                                  \
  if((set)->Cnt == 0)                            \
    (set)->State &= ~SLM_SET_FULL;               \
}


#define SLM_SET_msg(s_mode, me, cnt, dstMchn)    \
    (                                            \
      ( ( s_mode      & SLM_MODE)          &&    \
        ( (me)->State & SLM_ENABLED)       &&    \
        ( cnt        <= SLM_BUFFER_SIZE)   &&    \
        (!SLM_SET_full((me)->SlmSet))            \
      )                                          \
              ? RQST_SLM : 0                     \
    )

/*----------------------------------------------------------------*
 *   RRV private interface                                        *
 *----------------------------------------------------------------*/
#define RRV_msg(s_mode, cnt)                     \
    (                                            \
      ( ( s_mode     &  RRV_MODE)          ||    \
        ( cnt        >= RRV_THRESHOLD)           \
      )                                          \
              ? RQST_RRV : 0                     \
    )

/*----------------------------------------------------------------*
 *   SEND/RECV private interface                                  *
 *----------------------------------------------------------------*/
extern int    deal_send           (Rqst_t rqst, unsigned timeout);
extern int    deal_recv           (Rqst_t rqst);
extern int    deliver             (INET_iovec *iov, int last_frgmt, int *success);
extern int    TIMEDWAIT           (Thr_t me, Thr_t dstThr, unsigned timeout);


extern int    COM_init            ();
extern void   COM_finalize        (void);
extern void   COM_setLoc          (int (*f)(void *srcThr, Addr_t dstAddr, int *mchn, void *thr));


/*----------------------------------------------------------------*
 *   3. Declaration of EXPORTED interface                         *
 *----------------------------------------------------------------*/
extern Addr_t getAddr          (void);
extern int    getCpuId         (void);
#define       getGroup()       ((getAddr())->Group)
#define       getRank()        ((getAddr())->Rank)
extern double getAbsTime       (void);

/* SYNCHRONOUS interface */

#define       AZQ_send(      dst, buf, cnt, tag)      timed_send(    (dst),  (buf), (cnt), (tag), (SLM_MODE), COM_FOREVER)
#define       AZQ_ssend(     dst, buf, cnt, tag)      timed_send(    (dst),  (buf), (cnt), (tag), (RRV_MODE), COM_FOREVER)
#define       AZQ_recv(      src, buf, cnt, tag, st)  timed_recv(    (src),  (buf), (cnt), (tag), (st),       COM_FOREVER)
#define       AZQ_probe(     src, tag, st)            timed_probe(   (src),  (tag), (st),                     COM_FOREVER)
#define           send(      dst, buf, cnt, tag)      timed_send(    (dst),  (buf), (cnt), (tag), (SLM_MODE), COM_FOREVER)
#define           ssend(     dst, buf, cnt, tag)      timed_send(    (dst),  (buf), (cnt), (tag), (RRV_MODE), COM_FOREVER)
#define           recv(      src, buf, cnt, tag, st)  timed_recv(    (src),  (buf), (cnt), (tag), (st),       COM_FOREVER)
#define           probe(     src, tag, st)            timed_probe(   (src),  (tag), (st),                     COM_FOREVER)

extern int    timed_send       (const int dstRank, char *buf, int cnt, Tag_t tag, int mode,       unsigned  timeout);
extern int    timed_recv       (const int srcRank, char *buf, int cnt, Tag_t tag, Status *status, unsigned  timeout);
extern int    timed_probe      (const int srcRank,                     Tag_t tag, Status *status, unsigned  timeout);


/* ASYNCHRONOUS interface */

#define       AZQ_asend(     dst, buf, cnt, tag, rqst) _asend(    (dst),  (buf), (cnt), (tag), (0),        (rqst))
#define       AZQ_assend(    dst, buf, cnt, tag, rqst) _asend(    (dst),  (buf), (cnt), (tag), (RRV_MODE), (rqst))
#define           asend(     dst, buf, cnt, tag, rqst) _asend(    (dst),  (buf), (cnt), (tag), (0),        (rqst))
#define           assend(    dst, buf, cnt, tag, rqst) _asend(    (dst),  (buf), (cnt), (tag), (RRV_MODE), (rqst))
extern int    _asend           (const int dstRank, char *buf, int cnt, Tag_t tag, int mode,  Rqst_t rqst);
extern int    arecv            (const int srcRank, char *buf, int cnt, Tag_t tag,            Rqst_t rqst);
extern int    aprobe           (const int srcRank, Tag_t tag, int *flag, Status *status);

#define       AZQ_waitone(   rqst, st)                  timed_wait(     (rqst), (st),                       COM_FOREVER)
#define       AZQ_waitany(   rqst, cnt, idx, st)        timed_waitany(  (rqst), (cnt), (idx), (st),         COM_FOREVER)
#define       AZQ_waitall(   rqst, cnt,      st)        timed_waitall(  (rqst), (cnt),        (st),         COM_FOREVER)
#define       AZQ_waitsome(  rqst, cnt, idx, st, ocnt)  timed_waitsome( (rqst), (cnt), (idx), (st), (ocnt), COM_FOREVER)
#define           waitone(   rqst, st)                  timed_wait(     (rqst), (st),                       COM_FOREVER)
#define           waitany(   rqst, cnt, idx, st)        timed_waitany(  (rqst), (cnt), (idx), (st),         COM_FOREVER)
#define           waitall(   rqst, cnt,      st)        timed_waitall(  (rqst), (cnt),        (st),         COM_FOREVER)
#define           waitsome(  rqst, cnt, idx, st, ocnt)  timed_waitsome( (rqst), (cnt), (idx), (st), (ocnt), COM_FOREVER)

extern int    timed_wait       (Rqst_t *rqst,                      Status *status,                unsigned  timeout);
extern int    timed_waitany    (Rqst_t *rqst, int cnt, int *index, Status *status,                unsigned  timeout);
extern int    timed_waitall    (Rqst_t *rqst, int cnt,             Status *status,                unsigned  timeout);
extern int    timed_waitsome   (Rqst_t *rqst, int cnt, int *index, Status *status,  int *outcnt,  unsigned  timeout);

extern int    cancel           (Rqst_t rqst);

extern int    test             (Rqst_t *rqst,                          int *flag,    Status *status);
extern int    testany          (Rqst_t *rqst,  int cnt,   int *index,  int *flag,    Status *status);
extern int    testall          (Rqst_t *rqst,  int cnt,                int *flag,    Status *status);
extern int    testsome         (Rqst_t *rqst,  int incnt, int *outcnt, int *indices, Status *status);


/* PERSISTENT interface */

#define       AZQ_psend_init(  dst, buf, cnt, tag, rqst)  send_init( (dst), (buf), (cnt), (tag), 0,          (rqst))
#define       AZQ_pssend_init( dst, buf, cnt, tag, rqst)  send_init( (dst), (buf), (cnt), (tag), (RRV_MODE), (rqst))
#define           psend_init(  dst, buf, cnt, tag, rqst)  send_init( (dst), (buf), (cnt), (tag), 0,          (rqst))
#define           pssend_init( dst, buf, cnt, tag, rqst)  send_init( (dst), (buf), (cnt), (tag), (RRV_MODE), (rqst))
extern int    send_init        (const int dstRank, char *buf, int cnt, Tag_t tag, int s_mode, Rqst_t rqst);
extern int    psend_start      (Rqst_t rqst);
extern int    precv_init       (const int srcRank, char *buf, int cnt, Tag_t tag,             Rqst_t rqst);
extern int    precv_start      (Rqst_t rqst);


#endif
