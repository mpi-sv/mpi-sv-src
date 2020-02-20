/*-
 * Copyright (c) 2009-2011 Universidad de Extremadura
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

#include <config.h>

#include <addr.h>
#include <rpq.h>
#include <mbx.h>
#include <inet.h>
#include <rqst.h>
#include <thr_dptr.h>



/*----------------------------------------------------------------*
 *   Constants                                                    *
 *----------------------------------------------------------------*/
/* No timeout */
#define COM_FOREVER       0xFFFFFFFF

/* Communication modes:
   Blocking send             uses by default RRV only for large messages
   Blocking synchronous send uses by default always RRV
*/
//#define RRV_MODE          RQST_RRV

/* Remote Rendevouz Protocol packet thresold size in bytes */
#define RRV_THRESHOLD     (64 * 1024)

/* Protocol argument */
#define COM_PROTOCOL      ((Protocol_t)0x0000)

/* Wait and test */
#define NONE_SATISFIED    (-1)

/* Message size from which split copy is worth */
#define SPLIT_COPY_THRESHOLD  (2048*4)
//#define SPLIT_COPY_THRESHOLD  (1000000000)


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
#define DBG_PRNT(pmsg) \
  { \
    fprintf pmsg \
    ; fflush(stdout); \
  }
#else
  #define DBG_PRNT(pmsg)
#endif


/*----------------------------------------------------------------*
 *   Types                                                        *
 *----------------------------------------------------------------*/
typedef RQST_Status Status;


/*----------------------------------------------------------------*
 *   3. Declaration of PRIVATE EXPORTED interface                 *
 *----------------------------------------------------------------*/


/*----------------------------------------------------------------*
 *   SYNCHRONIZATION private interface                            *
 *----------------------------------------------------------------*/
/* Specific data for thread descriptor access */
extern pthread_key_t  key;
#define self()        ((Thr_t)pthread_getspecific(key))


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
extern void   AZQ_deal_recv       (Rqst_t rqst);
extern int    deliver             (INET_iovec *iov, int last_frgmt, int *success);

extern int    COM_init            ();
extern void   COM_finalize        (void);
extern void   COM_setLoc          (int (*f)(void *srcThr, Addr_t dst, int *mchn, void *thr));


/*----------------------------------------------------------------*
 *   3. Declaration of EXPORTED interface                         *
 *----------------------------------------------------------------*/
extern Addr_t getAddr          (void);
extern int    getCpuId         (void);
#define       getGroup()       ((getAddr())->Group)
#define       getRank()        ((getAddr())->Rank)
extern double getAbsTime       (void);



/*----------------------------------------------------------------*
 *   4. Aux Macros                                                *
 *----------------------------------------------------------------*/
#define makeUncancellable(srcRqst, cancelled) \
{                          \
  int state;               \
                           \
  *(cancelled) = 0;        \
  if(RQST_PENDING != (state = __sync_val_compare_and_swap(&(srcRqst)->State, RQST_PENDING, RQST_FEEDING))) { \
    *(cancelled) = 1;      \
  }                        \
}



   /*________________________________________________________________
  /                                                                  \
 |    deliverFromLfq                                                  |
 |                                                                    |
 |    Deliver a send request extracted from the receiver LFQ          |
 |                                                                    |
 |    PARAMETERS:                                                     |
 |    o srcRqst     (input/output)                                    |
 |        Send request                                                |
 |                                                                    |
  \____________/  ___________________________________________________/
              / _/
             /_/
            */
#define deliverLocalFromLfq(me, srcRqst)                                                       \
do {                                                                                           \
    Rqst_t   __restrict__      dstRqst;                                                                    \
    int            success;                                                                    \
                                                                                               \
    /*fprintf(stdout, "deliverLocalFromLfq(%p): BEGIN \n", self());    fflush(stdout); */      \
                                                                                               \
    DLQ_findLocalRPQ(&(me)->RecvPendReg, &dstRqst, ((Header_t)(srcRqst))->Src.Rank,            \
                                                   ((Header_t)(srcRqst))->Tag, &success );     \
    if(success) {                                                                              \
      Thr_t   srcThr       = (Thr_t)((srcRqst)->Owner);                                        \
      int     srcLocalRank = srcThr->LocalRank;                                                \
                                                                                               \
      if( (srcRqst)->SeqNr == (1 + (me)->LastRecvSeqNr[srcLocalRank])) {                       \
        int number = 0;                                                                        \
        if(RQST_isAsync(srcRqst)) {                                                            \
          int alreadyCancelled;                                                                \
          makeUncancellable(srcRqst, &alreadyCancelled);                                       \
          if(alreadyCancelled) {                                                               \
            if(!RQST_isPersistent(srcRqst)) {                                                  \
              LFQ_t   lfq    =  &srcThr->PubMailBox;                                           \
              LFQ_enq(lfq, (LFQ_Link_t)(srcRqst));                                             \
            }                                                                                  \
            else                                                                               \
              srcRqst->PersistentWait = FALSE;                                                 \
            (me)->LastRecvSeqNr[srcLocalRank]++;                                               \
            break;                                                                             \
          }                                                                                    \
        }                                                                                      \
        else {                                                                                 \
          number = ATOMIC_ADD(&srcRqst->SendCounter, 1);                                       \
          switch(number) {                                                                     \
            case 1:                                                                            \
              break;                                                                           \
            case 2:                                                                            \
              while(!RQST_isSendBufferSwitched(srcRqst));                                      \
              break;                                                                           \
            default:                                                                           \
              panic("deliverLocalFromLfq: Bad Number");                                        \
          }                                                                                    \
        }                                                                                      \
                                                                                               \
        RPQ_remove(&(me)->RecvPendReg, dstRqst);                                               \
        if (dstRqst->Type & RQST_PROBE) {                                                      \
          dstRqst->Status.Src.Rank = ((Header_t)(srcRqst))->Src.Rank;                          \
          dstRqst->Status.Tag      = ((Header_t)(srcRqst))->Tag;                               \
          dstRqst->Status.Count    = ((Header_t)(srcRqst))->MessageSize;                       \
          RQST_setState(dstRqst, RQST_SATISFIED);                                              \
          MBX_put(&(me)->MailBox, ((Header_t)(srcRqst)));                                      \
          srcRqst->Type |= RQST_PROBE;                                                         \
          break ;                                                                              \
        }                                                                                      \
                                                                                               \
        if((srcRqst)->Hdr.MessageSize >= SPLIT_COPY_THRESHOLD && (RQST_isWaiting(srcRqst) )) { \
          Thr_t   sender      = (Thr_t)(srcRqst)->Owner;                                       \
          LFQ_t   lfq         = &sender->PubMailBox;                                           \
                                                                                               \
          (srcRqst)->BuffInPtr = dstRqst;                                                      \
          RQST_setSplitCopy(srcRqst);                                                          \
          LFQ_enq(lfq, (LFQ_Link_t)(srcRqst));                                                 \
          RQST_halfFeedLocal(dstRqst, (srcRqst)->Hdr.Payload,  (srcRqst)->Hdr.MessageSize,     \
                                      (srcRqst)->Hdr.Src.Rank, (srcRqst)->Hdr.Tag);            \
          ATOMIC_ADD(&(srcRqst)->State, (RQST_isSync(srcRqst) ? 1 : -1 ));                     \
          ATOMIC_ADD(&(dstRqst)->State, 1);                                                    \
        }                                                                                      \
        else {                                                                                 \
          if(((Header_t)(srcRqst))->MessageSize > dstRqst->BuffSize) panic("Too big Message"); \
          RQST_feedLocal(dstRqst, ((Header_t)(srcRqst)));                                      \
          RQST_setState( dstRqst,  RQST_SATISFIED);                                            \
          RQST_setState((srcRqst), RQST_SATISFIED);                                            \
          __asm__ __volatile__ ( "mfence" ::: "memory" );                                      \
        }                                                                                      \
        if(number == 2) {                                                                      \
          if((srcRqst)->SendBuffer)                                                            \
            free((srcRqst)->SendBuffer);                                                       \
          rqstFree(srcThr->RqstTable, &(srcRqst));                                             \
        }                                                                                      \
        (me)->LastRecvSeqNr[srcLocalRank]++;                                                   \
        break;                                                                                 \
      }                                                                                        \
    }                                                                                          \
    MBX_put(&(me)->MailBox, ((Header_t)(srcRqst)));                                            \
} while(0);


#ifdef USE_FASTBOXES 

   /*________________________________________________________________
  /                                                                  \
 |    deliverFromFbox                                                 |
 |                                                                    |
 |    Deliver a send request extracted from the receiver LFQ          |
 |                                                                    |
 |    PARAMETERS:                                                     |
 |    o srcRqst     (input/output)                                    |
 |        Send request                                                |
 |                                                                    |
  \____________/  ___________________________________________________/
              / _/
             /_/
            */
#define deliverFromFbox(me, fBox) \
do { \
  Rqst_t         dstRqst; \
  int            success; \
\
  /*fprintf(stdout, "deliverFromFbox(%p). Source Fbox %p. BEGIN\n", self(), (fBox));  
    fprintf(stdout, "deliverFromFbox(%p). Key Rank %d, Key Tag %x \n", self(), (fBox)->SenderGlobalRank, (fBox)->Tag ); fflush(stdout); */ \
 \
  DLQ_findLocalRPQ(&(me)->RecvPendReg, &dstRqst, (fBox)->SenderGlobalRank, (fBox)->Tag, &success ); \
  if(success) { \
    /*fprintf(stdout, "deliverFromFbox(%p): Match found in RPQ !! \n", self());    fflush(stdout); */ \
\
    RPQ_remove(&(me)->RecvPendReg, dstRqst); \
    if (dstRqst->Type & RQST_PROBE) {   \
        dstRqst->Status.Src.Rank = (fBox)->SenderGlobalRank; \
        dstRqst->Status.Tag      = (fBox)->Tag; \
        dstRqst->Status.Count    = (fBox)->MessageSize; \
        RQST_setState(dstRqst, RQST_SATISFIED);  \
      /*fprintf(stdout, "deliverFromFbox(%p): Match RQST_PROBE in RPQ !! fBoxSeqNr %d END\n", self(), fBox->SeqNr); fflush(stdout); */ \
        break; \
    } \
\
    /*fprintf(stdout, "deliverFromFbox(%p): Match found in RPQ !! fBoxSeqNr %d\n", self(), fBox->SeqNr);    fflush(stdout); */ \
    RQST_feedFromFBox(dstRqst, (fBox)); \
    me->LastRecvSeqNr[(fBox)->SenderLocalRank]++; \
    (fBox)->Turn = TURN_SEND; \
    RQST_setState(dstRqst, RQST_SATISFIED); \
    __asm__ __volatile__ ( "sfence" ::: "memory" ); \
  }  \
  else {\
  /*fprintf(stdout, "deliverFromFbox(%p): Match Not found in RPQ :(\n", self());    fflush(stdout); */ \
  } \
  /*fprintf(stdout, "deliverFromFbox(%p): END\n", self());    fflush(stdout); */ \
} while(0);



#else 
  /* No fast boxes */





#define BUENO_deliverRemoteFromLfq(me, srcHdr)   \
do { \
    \
    Rqst_t      dstRqst;\
    int         success_local, excpn;\
    Header_t    hdr2;\
    INET_iovec  iov[2]; \
    \
\
\
  DBG_PRNT((stdout, "deliverRemoteFromLfq(%p). Header %p Mode: %s  (start)\n", me, ((Header_t)(srcHdr)), (((Header_t)(srcHdr))->Mode & MODE_RRV_RQST) ? "MODE_RRV_RQST" : (((Header_t)(srcHdr))->Mode & MODE_RRV_ACK) ? "MODE_RRV_ACK" : (((Header_t)(srcHdr))->Mode & MODE_RRV_DATA) ? "MODE_RRV_DATA" : "MODE_EAGER")); \
  DBG_PRNT((stdout, "\t Looking up request in RPQ from [%x, %d] and tag: %x \n", self(), ((Header_t)(srcHdr))->Src.Group, ((Header_t)(srcHdr))->Src.Rank, ((Header_t)(srcHdr))->Tag)); \
  DLQ_findRPQ(&(me)->RecvPendReg, &dstRqst, ((Header_t)(srcHdr))->Src.Rank, ((Header_t)(srcHdr))->Tag, ((Header_t)(srcHdr))->Mode, &success_local); \
    DBG_PRNT((stdout, "deliverRemoteFromLfq(%p). SUCCESS: %d\n", self(), success_local)); \
    if(success_local) { \
      if (((Header_t)(srcHdr))->Mode & MODE_RRV_RQST) { \
        DBG_PRNT((stdout, "deliverRemoteFromLfq(%p): MODE_RRV_RQST message. Mark rqst %p for receiving only from [%d] Rag: %d\n", ((Thr_t) dstRqst->Owner), dstRqst, ((Header_t)(srcHdr))->Src.Rank, ((Header_t)(srcHdr))->Tag)); \
        /* Mark the request to ONLY receive all message fragments from this source (deberia ser solamente para el caso de recibir de ANY) */ \
        dstRqst->Status.Src  = ((Header_t)(srcHdr))->Src; \
        dstRqst->Status.Tag  = ((Header_t)(srcHdr))->Tag; \
        dstRqst->Hdr.Mode   &= ~MODE_ANY; \
        dstRqst->Hdr.Mode   |= MODE_RRV_DATA; \
        \
        /* Send "ready-to-receive" message to sender */\
        sendRrvAck(((Header_t)(srcHdr)), dstRqst);\
        \
        /* Message received. Safely erase it from low-level buffer */\
        DBG_PRNT((stdout, "deliverRemoteFromLfq(%p): INET_recv of message header %p\n", self(), srcHdr)); \
        INET_recv((char *)(srcHdr));\
      \
      } else if (((Header_t)(srcHdr))->Mode & MODE_RRV_ACK) { \
        /* Remove send request from the queue */ \
        DBG_PRNT((stdout, "deliverRemoteFromLfq(%p). Founded! Remove %p from RPQ\n", self(), dstRqst)); \
        RPQ_remove(&(me)->RecvPendReg, dstRqst); \
       \
        /* Build message and send data */ \
        hdr2             = &(dstRqst->Hdr); \
        hdr2->RrvDstAddr = ((Header_t)(srcHdr))->RrvDstAddr; \
		hdr2->Mode      &= ~MODE_RRV_ACK; \
        hdr2->Mode      |= MODE_RRV_DATA; \
       \
        iov[0].Data     = (char *)hdr2; \
        iov[0].Size     = HEADER_NET_SZ; \
        iov[1].Data     = (char *)hdr2->Payload; \
        iov[1].Size     = hdr2->PayloadSize; \
       \
        DBG_PRNT((stdout, "deliverRemoteFromLfq(%p). Sending DATA to [%x, %d]\n", self(), ((Header_t)(srcHdr))->Src.Group, ((Header_t)(srcHdr))->Src.Rank)); \
        INET_send(iov, hdr2->DstMchn, COM_PROTOCOL); \
       \
        /* Send request is satisfied */ \
        RQST_setState(dstRqst, RQST_SATISFIED);\
       \
        /* Message received, erase it from low-level buffer */ \
        DBG_PRNT((stdout, "deliverRemoteFromLfq(%p): INET_recv of message header %p\n", self(), ((Header_t)(srcHdr)))); \
        INET_recv((char *)srcHdr); \
	   \
        return COM_E_OK;\
       \
      } else {\
        /* Mark the request to ONLY receive all message fragments from this source (deberia ser solamente para el caso de recibir de ANY) */\
        if (((Header_t)(srcHdr))->Mode & MODE_EAGER) { /* Aqui vienen MODE_RRV_DATA (ya he marcado el fuente) y MODE_DATA (todavia no lo he marcado)*/ \
          DBG_PRNT((stdout, "deliverRemoteFromLfq(%p): MODE_EAGER message. Mark rqst %p for receiving only from [%d] Tag: %d\n", ((Thr_t) dstRqst->Owner), dstRqst, ((Header_t)(srcHdr))->Src.Rank, ((Header_t)(srcHdr))->Tag)); \
          dstRqst->Status.Src  = ((Header_t)(srcHdr))->Src;\
          dstRqst->Status.Tag  = ((Header_t)(srcHdr))->Tag;      /* solamente se debe hacer para el primer fragmento y en el caso de recibir de ANY */ \
          dstRqst->Hdr.Mode   &= ~MODE_ANY; \
          dstRqst->Hdr.Mode   |= MODE_EAGER; \
        }\
        \
        /* Feed the destination request */\
        DBG_PRNT((stdout, "deliverRemoteFromLfq(%p): Feeding rqst %p with header %p\n", ((Thr_t)me), dstRqst, ((Header_t)(srcHdr)))); \
        RQST_feed(dstRqst, ((Header_t)(srcHdr)));\
        \
        /* Last fragment arrived completes the destination request */\
        if (((Header_t)(srcHdr))->Mode & MODE_LAST_FRAGMENT) {\
          RPQ_remove(&(me)->RecvPendReg, dstRqst);\
          RQST_setState(dstRqst, RQST_SATISFIED);\
          DBG_PRNT((stdout, "deliverRemoteFromLfq(%p): Delete rqst %p from RPQ. SATISFIED \n", ((Thr_t)me), dstRqst)); \
        }\
        \
        /* Message received, erase it from low-level buffer */\
        DBG_PRNT((stdout, "deliverRemoteFromLfq(%p): INET_recv of message header %p\n", self(), ((Header_t)(srcHdr)))); \
        INET_recv((char *)(srcHdr));\
        \
      }\
    } else {\
      /* Receiver not waiting. Put message in mailbox */\
      DBG_PRNT((stdout, "deliverRemoteFromLfq(%p): Put rqst %p in MBX \n", ((Thr_t)me), ((Header_t)(srcHdr)))); \
      MBX_put(&(me)->MailBox, ((Header_t)(srcHdr)));\
    }\
    DBG_PRNT((stdout, "deliverRemoteFromLfq(%p). (end) \n", ((Thr_t)me))); \
   \
} while(0); 


#endif /* #ifdef USE_FASTBOXES */




#define recvFromFbox(me, srcRank, buff, cnt, tag, status, result) \
do { \
\
  fastBox_t fBox; \
\
  *(result) = 0; \
  if((srcRank) != ADDR_RNK_ANY) { \
    /* 1. Map dstRank to the fastBox in which looking up by tag */ \
    Thr_t srcThr       = ((me)->GrpInfo)[(srcRank)].Thr; \
    int   srcLocalRank = srcThr->LocalRank; \
    fBox               = &(me)->FastBox[srcLocalRank]; \
\
    if( fBox->Turn == TURN_RECV ) { \
      /*fprintf(stdout, "recvFromFbox(%p): Recibo de fbox %d\n", self(), THR_getLocalRank(srcThr)); */ \
      /* 2. Test the tag */ \
      if(TAG_MATCH((tag), fBox->Tag)) { \
        /*fprintf(stdout, "recvFromFbox(%p): Tag %x and fBox->Tag %x MATCH!!. Expected seqNr %d\n", 
                    self(), tag, fBox->Tag, (1 + (me)->LastRecvSeqNr[srcLocalRank])); */ \
        if(fBox->SeqNr == (1 + (me)->LastRecvSeqNr[srcLocalRank])) { \
          /*fprintf(stdout, "recvFromFbox(%p): Match found !! fBoxSeqNr %d\n", self(), fBox->SeqNr);    fflush(stdout); */ \
          \
          MEMCPY((buff),  (fBox)->Payload, (fBox)->MessageSize); \
          if((status)) { \
            (status)->Count    = (fBox)->MessageSize; \
            (status)->Src.Rank = (fBox)->SenderGlobalRank; \
            (status)->Tag      = (fBox)->Tag; \
          } \
          \
          (me)->LastRecvSeqNr[srcLocalRank]++; \
          fBox->Turn = TURN_SEND; \
          __asm__ __volatile__ ( "mfence" ::: "memory" ); \
          *(result) = 1; \
          /*fprintf(stdout, "recvFromFbox(%p): Got from FastBox. END\n", self()); fflush(stdout); */ \
          break; \
        } \
      } \
    } \
  } \
  else { \
    int lRank; \
    /* Sweep all the fast boxes */ \
    for(lRank = 0; lRank < (me)->LocalGroupSize; lRank++) { \
      fBox = &(me)->FastBox[lRank]; \
      if( fBox->Turn == TURN_RECV ) { \
        if(TAG_MATCH(tag, fBox->Tag)) { \
          if(fBox->SeqNr == (1 + (me)->LastRecvSeqNr[lRank])) { \
            /*fprintf(stdout, "recvFromFbox(%p): Match found in RQST !! fBoxSeqNr %d\n", self(), fBox->SeqNr);    fflush(stdout); */ \
\
            MEMCPY((buff),  (fBox)->Payload, (fBox)->MessageSize); \
            if((status)) { \
              (status)->Count = (fBox)->MessageSize; \
              (status)->Src.Rank  = (fBox)->SenderGlobalRank; \
              (status)->Tag   = (fBox)->Tag; \
            } \
\
            (me)->LastRecvSeqNr[lRank]++; \
            fBox->Turn = TURN_SEND; \
            __asm__ __volatile__ ( "mfence" ::: "memory" ); \
            *result = 1; \
            /*fprintf(stdout, "recvFromFbox(%p): Got from FastBox. End\n", self()); fflush(stdout);*/ \
            break; \
          } \
        } \
      } \
    } \
  } \
} while(0);


 

/* ------------------------------------------------------------------------------------------------
 * SYNCHRONOUS interface
 * ------------------------------------------------------------------------------------------------
 */
#define       send(  dst,  buff, cnt, tag)             AZQ_send(  (dst), (buff), (cnt), (tag))
#define       asend( dst, buff, cnt, tag, rqst)        AZQ_asend( (dst), (buff), (cnt), (tag), (rqst)) 
#define       ssend( dst, buff, cnt, tag)              AZQ_ssend( (dst),  (buf), (cnt), (tag))
#define       assend(dst, buff, cnt, tag, rqst)        AZQ_assend((dst),  (buf), (cnt), (tag), (rqst))
#define       recv(  dst,  buff, cnt, tag, status)     AZQ_recv(  (dst), (buff), (cnt), (tag), (status))
#define       arecv( dst, buff, cnt, tag, rqst)        AZQ_arecv( (dst), (buff), (cnt), (tag), (rqst)) 
#define       probe( src, tag, st)                     AZQ_probe( (src),  (tag), (st)        )
#define       cancel(rqst)                             AZQ_cancel((rqst))



extern int    AZQ_send  (const int dstRank, const char *buff, const int cnt, const Tag_t tag);
extern int    AZQ_asend (const int dstRank, const char *buff, const int cnt, const Tag_t tag, Rqst_t rqst); 
extern int    AZQ_ssend (const int dstRank, const char *buff, const int cnt, const Tag_t tag);
extern int    AZQ_assend(const int dstRank, const char *buff, const int cnt, const Tag_t tag, Rqst_t rqst); 
extern int    AZQ_recv  (const int dstRank, const char *buff, const int cnt, const Tag_t tag, RQST_Status *status);
extern int    AZQ_arecv (const int dstRank, const char *buff, const int cnt, const Tag_t tag, Rqst_t rqst);
extern int    aprobe    (const int srcRank, const Tag_t tag, int *flag, RQST_Status *status);
extern int    AZQ_cancel(Rqst_t rqst);


#define       waitone(rqst,           st)             AZQ_waitone  ( (rqst),               (st))
#define       waitany(rqst, cnt, idx, st)             AZQ_waitany  ( (rqst), (cnt), (idx), (st))
#define       waitall(rqst, cnt,      st)             AZQ_waitall  ( (rqst), (cnt),        (st))
#define       waitsome(rqst, cnt, idx, st, ocnt)      AZQ_waitsome((rqst), (cnt), (idx), (st), (ocnt))
extern int    AZQ_waitany      (Rqst_t *rqst, int cnt, int *index, RQST_Status *status);
extern int    AZQ_waitone      (Rqst_t *rqst,                      RQST_Status *status);
extern int    AZQ_waitall      (Rqst_t *rqst, int cnt,             RQST_Status *status);
extern int    AZQ_waitsome     (Rqst_t *rqst, int cnt, int *index, RQST_Status *status, int *outcount);



extern int    test             (Rqst_t *rqst,                        int *flag,    RQST_Status *status);
extern int    testany          (Rqst_t *rqst,  int cnt, int *index,  int *flag,    RQST_Status *status);
extern int    testall          (Rqst_t *rqst,  int cnt,              int *flag,    RQST_Status *status);
extern int    testsome         (Rqst_t *rqst,  int cnt, int *index, RQST_Status *status, int *outcnt);


/* PERSISTENT interface */
#define       psend_init(  dst, buf, cnt, tag, rqst)  AZQ_psend_init( (dst), (buf), (cnt), (tag), 0,               (rqst))
#define       pssend_init( dst, buf, cnt, tag, rqst)  AZQ_psend_init( (dst), (buf), (cnt), (tag), 0/*(RRV_MODE)*/, (rqst))
#define       precv_init(  dst, buf, cnt, tag, rqst)  AZQ_precv_init( (dst), (buf), (cnt), (tag),                  (rqst))
#define       psend_start(rqst)                       AZQ_psend_start( (rqst))
#define       precv_init(  dst, buf, cnt, tag, rqst)  AZQ_precv_init( (dst), (buf), (cnt), (tag),                  (rqst))
#define       precv_start(rqst)                       AZQ_precv_start( (rqst))
extern int    AZQ_psend_init   (const int dst, char *buf, int cnt, Tag_t tag, int s_mode, Rqst_t rqst);
extern int    AZQ_psend_start  (Rqst_t rqst);
extern int    AZQ_precv_init   (const int src, char *buf, int cnt, Tag_t tag,             Rqst_t rqst);
extern int    AZQ_precv_start  (Rqst_t rqst);




#endif
