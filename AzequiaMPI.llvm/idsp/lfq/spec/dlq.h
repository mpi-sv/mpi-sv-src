/*-
 * Copyright (c) 2009 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */


/*
   Double Linked Queue

   This module implements a doubled linked queue. The items are put at the end, but can be
   retrieved from anywhere (by comparing throught a external function.
   This structure is not thread-safe.
   The items can be of any struct type, but REQUIRE a Link field in the
   first position of the struct.
 */

#ifndef _DLQ_H
#define _DLQ_H

#include <addr.h>

#ifndef TRUE
#define FALSE (0)
#define TRUE  (1)
#endif

/* Debug messages */
#ifdef __DLQ_DEBUG
#define DBG_PRNT(pmsg) \
{ \
   fprintf pmsg \
   ; fflush(stdout); \
}
#else
#define DBG_PRNT(pmsg)
#endif

/*----------------------------------------------------------------*
 *   Definition of public data types                              *
 *----------------------------------------------------------------*/

/* Double linked list of items in the queue */
struct DLQ_Link {
  struct DLQ_Link  *Next;
  struct DLQ_Link  *Prev;
};
typedef struct DLQ_Link DLQ_Link, *DLQ_Link_t;

struct DLQ {
  DLQ_Link   Queue;     /* Next of the queue of allocated elements */
//int        Count;     /* Number of elements currently in queue   */
//int        (*cmp)   (void *item, Addr_t addr, Tag_t tag, int type, int mode);
                        /* External function for comparing request */
};
typedef struct DLQ  DLQ, *DLQ_t;


/*----------------------------------------------------------------*
 *   Macros implementing the interface                            *
 *----------------------------------------------------------------*/
#define DLQ_create(dlq, cmp_fxn)       \
{                                      \
  (dlq)->Queue.Prev = &((dlq)->Queue); \
  (dlq)->Queue.Next = &((dlq)->Queue); \
  /*(dlq)->cmp          = (cmp_fxn); */    \
}



#define DLQ_put(dlq, item)                      \
{                                               \
  ((DLQ_Link_t)(item))->Next = &((dlq)->Queue);     \
  ((DLQ_Link_t)(item))->Prev = (dlq)->Queue.Prev;     \
  (dlq)->Queue.Prev->Next = ((DLQ_Link_t)(item));   \
  (dlq)->Queue.Prev       = ((DLQ_Link_t)(item));   \
}



#define DLQ_get(dlq, item) {                                   \
  (*(DLQ_Link_t *)(item))             = (dlq)->Queue.Next;         \
  (*(DLQ_Link_t *)(item))             = (dlq)->Queue.Next;         \
  (*(DLQ_Link_t *)(item))->Prev->Next = (*(DLQ_Link_t *)(item))->Next; \
  (*(DLQ_Link_t *)(item))->Next->Prev = (*(DLQ_Link_t *)(item))->Prev; \
}



#define DLQ_isEmpty(dlq) (                                     \
  ((dlq)->Queue.Prev == &((dlq)->Queue) ? 1 : 0)               \
)



#define DLQ_delete(dlq, item) {                                \
  ((DLQ_Link_t)(item))->Prev->Next = ((DLQ_Link_t)(item))->Next;       \
  ((DLQ_Link_t)(item))->Next->Prev = ((DLQ_Link_t)(item))->Prev;       \
}



#define old_DLQ_findRPQ(dlq, item, keyRank, keyTag, type, mode, success)    \
do {                                                           \
  *(success) = FALSE;                                 \
\
   DLQ_Link_t         lprev;                     \
   DLQ_Link_t         link;                   \
                                                               \
  if (DLQ_isEmpty(dlq)) {                                      \
    DBG_PRNT((stdout, "DLQ_find: Cola vacía.\n"));       \
        break;                                                     \
  }                                                            \
  lprev = &((dlq)->Queue);                     \
  link  = (dlq)->Queue.Next;                   \
  *(success) = FALSE;                                   \
  while (link != lprev) {                                      \
    fprintf(stdout, "Link = 0x%x\n", link); fflush(stdout); \
    if (RQST_match((Rqst_t)link, (keyRank), (keyTag), (mode))) { \
      *(item)    = (Rqst_t)link;                              \
      *(success) = TRUE;                          \
      fprintf(stdout, "DLQ_find: ¡Found!\n"); fflush(stdout);   \
      break;                                                   \
    }                                                          \
    \
    \
    else { \
      fprintf(stdout, "DLQ_find: NOT MATCH  [%d  %d  %x]  con  [%d  %d  %x]\n", \
         keyRank, keyTag, mode, ((Rqst_t)link)->Status.Src.Rank, ((Rqst_t)link)->Status.Tag); fflush(stdout);   \
    } \
    link = link->Next;                                         \
  }                                                            \
} while (0);


#define DLQ_findRPQ(dlq, item, keyRank, keyTag, mode, success)    \
{                                                           \
  *(success) = FALSE;                                 \
  if (!DLQ_isEmpty(dlq)) {                                      \
    DBG_PRNT((stdout, "DLQ_findRPQ (%p): RPQ not empty\n", self()));   \
    DLQ_Link_t   lprev = &((dlq)->Queue);                     \
    DLQ_Link_t   link  = (dlq)->Queue.Next;                   \
    do {                                      \
      DBG_PRNT((stdout, "DLQ_findRPQ (%p): RQST_match link [%d  %d  %x] with [%d  %d  %x]\n", self(), ((Rqst_t)(link))->Status.Src.Rank, ((Rqst_t)(link))->Status.Tag, ((Rqst_t)(link))->Hdr.Mode, (keyRank), (keyTag), (mode)));  \
      if (RQST_match((Rqst_t)link, (keyRank), (keyTag), (mode))) { \
        *(item)    = (Rqst_t)link;                              \
        *(success) = TRUE;                          \
        DBG_PRNT((stdout, "DLQ_findRPQ (%p): ¡Found!\n", self()));   \
        break;                                                   \
      }                                                           \
    link = link->Next;                                         \
    } while (link != lprev);                                                           \
  } \
  DBG_PRNT((stdout, "DLQ_findRPQ (%p):  Encontrada? : %d\n", self(), *success));  \
}


#define DLQ_findLocalRPQ(dlq, item, keyRank, keyTag, success)    \
{                                                           \
  *(success) = FALSE;                                 \
  if (!DLQ_isEmpty(dlq)) {                                      \
    DBG_PRNT((stdout, "DLQ_findLocalRPQ (%p): RPQ not empty\n", self()));   \
    DLQ_Link_t   lprev = &((dlq)->Queue);                     \
    DLQ_Link_t   link  = (dlq)->Queue.Next;                   \
    do {                                      \
      DBG_PRNT((stdout, "DLQ_findLocalRPQ (%p): RQST_match link [%d  %d  %x] with [%d  %d  -1]\n", self(), ((Rqst_t)(link))->Status.Src.Rank, ((Rqst_t)(link))->Status.Tag, ((Rqst_t)(link))->Hdr.Mode, (keyRank), (keyTag)));  \
      if (RQST_matchLocal((Rqst_t)link, (keyRank), (keyTag))) { \
        *(item)    = (Rqst_t)link;                              \
        *(success) = TRUE;                          \
        DBG_PRNT((stdout, "DLQ_findLocalRPQ (%p): ¡Found!\n", self()));   \
        break;                                                   \
      }                                                          \
      link = link->Next;                                         \
    } while (link != lprev);                                                           \
    DBG_PRNT((stdout, "DLQ_findLocalRPQ (%p):  Encontrada? : %d\n", self(), *success));  \
  } \
}




#define DLQ_findMBX(dlq, item, keyRank, keyTag, mode, success)    \
{                                                           \
  /*fprintf(stdout, "DLQ_findMBX: wanted Rank = %x\n", (keyRank)); fflush(stdout);*/ \
  /*fprintf(stdout, "DLQ_findMBX: wanted Tag  = %x\n", (keyTag));  fflush(stdout); */\
  *(success) = FALSE;                                 \
  if (!DLQ_isEmpty(dlq)) {                                      \
    DBG_PRNT((stdout, "DLQ_findMBX (%p): MBX not empty\n", self()));   \
    DLQ_Link_t         lprev  = &((dlq)->Queue);                   \
    DLQ_Link_t         link   = (dlq)->Queue.Next;                \
    do {    \
       DBG_PRNT((stdout, "DLQ_findMBX (%p): HDR_Match link [%d  %d  %x] with [%d  %d  %x]\n", self(), ((Header_t)(link))->Src.Rank, ((Header_t)(link))->Tag, ((Header_t)(link))->Mode, (keyRank), (keyTag), (mode)));  \
      /*fprintf(stdout, "DLQ_findMBX: Link = 0x%x\n", link); fflush(stdout); */\
      /*if(RANK_match( ((Header_t)link)->Src.Rank, (keyRank))) fprintf(stdout, "DLQ_findMBX: RANK_match\n"); fflush(stdout);*/ \
      /*if(TAG_MATCH (keyTag, ((Header_t)link)->Tag))          fprintf(stdout, "DLQ_findMBX: TAG_match\n");  fflush(stdout);*/ \
      if  \
            ( (RANK_match( ((Header_t)link)->Src.Rank, (keyRank))) && \
              (TAG_MATCH(keyTag, ((Header_t)link)->Tag))    && \
              (((Header_t)link)->Mode & (mode)) \
         ) { \
        *(item)    = (Rqst_t)link;     \
        *(success) = TRUE;             \
        /*fprintf(stdout, "DLQ_findMBX: ¡Encontrada!\n"); fflush(stdout);*/   \
        DBG_PRNT((stdout, "DLQ_findMBX (%p): ¡Found!\n", self()));   \
        break;                                                   \
      }                                                          \
      link = link->Next;                     \
    } while (link != lprev);                                     \
    DBG_PRNT((stdout, "DLQ_findMBX (%p):  Encontrada? : %d\n", self(), *success));  \
  }                                                            \
}

#define old_DLQ_findMBX(dlq, item, keyRank, keyTag, type, mode, success)    \
{                                                           \
/*fprintf(stdout, "DLQ_findMBX: wanted Rank = %x\n", (keyRank)); fflush(stdout);*/ \
/*fprintf(stdout, "DLQ_findMBX: wanted Tag  = %x\n", (keyTag));  fflush(stdout); */\
*(success) = FALSE;                                 \
if (!DLQ_isEmpty(dlq)) {                                      \
DLQ_Link_t         lprev  = &((dlq)->Queue);                   \
DLQ_Link_t         link   = (dlq)->Queue.Next;                \
do {    \
/*fprintf(stdout, "DLQ_findMBX: Link = 0x%x\n", link); fflush(stdout); */\
/*if(RANK_match( ((Header_t)link)->Src.Rank, (keyRank))) fprintf(stdout, "DLQ_findMBX: RANK_match\n"); fflush(stdout);*/ \
/*if(TAG_MATCH (keyTag, ((Header_t)link)->Tag))          fprintf(stdout, "DLQ_findMBX: TAG_match\n");  fflush(stdout);*/ \
if (( \
( (RANK_match( ((Header_t)link)->Src.Rank, (keyRank))) && \
(TAG_MATCH(keyTag, ((Header_t)link)->Tag))    && \
(HDR_isLocal((Header_t)link) ? 1 : ((type & RQST_MASK) & (((Header_t)link)->Mode))) \
) \
? 1 : 0 \
) \
) { \
*(item)    = (Rqst_t)link;     \
*(success) = TRUE;             \
/*fprintf(stdout, "DLQ_findMBX: ¡Encontrada!\n"); fflush(stdout);*/   \
break;                                                   \
}                                                          \
link = link->Next;                     \
} while (link != lprev);                                     \
}                                                            \
}


#endif
