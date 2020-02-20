/*
 * Copyright (c) 2010 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */
/*
   Lock-Free Queue

   This module implements a lock-free queue of one reader and multiple writers
   Being lock-free, by its nature this module is thread-safe.
   The items can be of any struct type, but REQUIRE a Link field in the
   first position of the struct
 */

#ifndef _LFQ_H
#define _LFQ_H

#include <atomic.h>
#include <arch.h>

/*----------------------------------------------------------------*
 *   Definition of public data types                              *
 *----------------------------------------------------------------*/

/* Single linked list of items in the queue */
typedef struct LFQ_Link LFQ_Link, *LFQ_Link_t;
struct LFQ_Link {
  LFQ_Link_t  Next;
  LFQ_Link_t  Prev;
};

#ifndef CACHE_LINE_SIZE
#error "Must define CACHE_LINE_SIZE"
#endif

typedef struct LFQ  *LFQ_t;
#define LFQSize (2 * sizeof (LFQ_Link_t))
struct LFQ {
  LFQ_Link_t  Next  __attribute__(( aligned(CACHE_LINE_SIZE) ));
  LFQ_Link_t  Prev;
  char        Pad[(LFQSize % CACHE_LINE_SIZE ? CACHE_LINE_SIZE - (LFQSize % CACHE_LINE_SIZE) : 0)];
};
typedef struct LFQ LFQ;

/*----------------------------------------------------------------*
 *   Declaration of public interface                              *
 *----------------------------------------------------------------*/

/*
 * LFQ_isEmpty
 */
#define LFQ_isEmpty(lfq)  ((lfq)->Next ? 0 : 1)

/*
 * LFQ_deq
 */
#define  LFQ_deq(lfq, item__)                                  \
{                                                              \
  volatile LFQ_Link_t  item;                                   \
                                                               \
  item = (lfq)->Next;                                          \
  if(item->Next != NULL)                                       \
    (lfq)->Next = item->Next;                                  \
  else {                                                       \
    (lfq)->Next = NULL;                                        \
    if(!ATOMIC_BOOL_CMPXCHG(&(lfq)->Prev, (item), NULL)) {     \
      while(!item->Next);                                      \
      (lfq)->Next = item->Next;                                \
    }                                                          \
  }                                                            \
  *(item__) = (typeof(*item__))(item);                         \
}

/*
 * LFQ_enq
 */
#define  LFQ_enq(lfq, item)                                    \
{                                                              \
  LFQ_Link_t  prev;                                            \
                                                               \
  (item)->Next = NULL;                                         \
  prev = (LFQ_Link_t) ATOMIC_XCHG(&(lfq)->Prev, (item));       \
  if (prev == NULL)                                            \
    (lfq)->Next = (item);                                      \
  else                                                         \
    prev->Next = (item);                                       \
}

#endif
