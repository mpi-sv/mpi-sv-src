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
#include <dlq.h>

/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   used by this module                                          *
 *----------------------------------------------------------------*/
#if defined(__OSI)
  #include <osi.h>
#else
  #include <stdio.h>
  #include <stdlib.h>
#endif

#include <thr_dptr.h>
#include <util.h>
#include <com.h>


/*----------------------------------------------------------------*
 *   Definition of private constants                              *
 *----------------------------------------------------------------*/
#ifdef __DLQ_DEBUG
#define self()          ((Thr_t)pthread_getspecific(key))
#endif


#ifdef __COM_DEBUG
#define self()          ((Thr_t)pthread_getspecific(key))
#endif
/*----------------------------------------------------------------*
 *   Definition of private types                                  *
 *----------------------------------------------------------------*/

/* Struct for management of a double linked queue */
struct DLQ {
  Link   Queue;     /* Next of the queue of allocated elements */
  int    Count;     /* Number of elements currently in queue   */
  int   (*cmp)   (void *item, Addr_t addr, Tag_t tag, int type, int mode);
                    /* External function for comparing request */
};

/*----------------------------------------------------------------*
 *   Implementation of public function interface                  *
 *----------------------------------------------------------------*/
/*
 *  Create a DLQ struct
 *    cmp_fxn is a function (external) to compare items for finding
 */
int DLQ_create (DLQ_t *dlq, int (*cmp_fxn) (void *item, Addr_t addr, Tag_t tag, int type, int mode)) {

  DLQ_t  dlq_new;

  if (!(dlq_new = (DLQ_t)AZQ_MALLOC(sizeof(struct DLQ))))  return -1;

  dlq_new->Queue.Prev = &(dlq_new->Queue);
  dlq_new->Queue.Next = &(dlq_new->Queue);
  dlq_new->Count        = 0;
  dlq_new->cmp          = cmp_fxn;

  DBG_PRNT((stdout, "DLQ_create(%p): %p\n", self(), *dlq));

  *dlq = dlq_new;

  return 0;
}


/*
 *  Destroy the queue
 */
void DLQ_destroy (DLQ_t dlq) {
  AZQ_FREE(dlq);
}


/*
 *  Put an item at the tail of the queue
 */
int DLQ_put (DLQ_t  dlq, void *item) {

  register Link *link = (Link *)item;

  link->Next = &(dlq->Queue);
  link->Prev = dlq->Queue.Prev;

  dlq->Queue.Prev->Next = link;
  dlq->Queue.Prev       = link;

  dlq->Count++;

  DBG_PRNT((stdout, "DLQ_put(%p): dlq: %p  item: %p  (cnt: %d)\n", self(), dlq, item, dlq->Count));

  return 0;
}


/*
 *  Get an item from the head of the queue
 */
int DLQ_get (DLQ_t dlq, void **item) {

  register Link  *link;

  link             = dlq->Queue.Next;
  link->Prev->Next = link->Next;
  link->Next->Prev = link->Prev;

  dlq->Count--;

  *item = link;

  DBG_PRNT((stdout, "DLQ_get(%p): dlq: %p  item: %p  (cnt: %d)\n", self(), dlq, *item, dlq->Count));

  return 0;
}


/*
 *  Delete an item pointed by "item"
 */
void DLQ_delete (DLQ_t dlq, void *item) {

  Link *link = (Link *)item;

  link->Prev->Next = link->Next;
  link->Next->Prev = link->Prev;

  dlq->Count--;

  DBG_PRNT((stdout, "DLQ_delete(%p): dlq: %p item: %p  (cnt: %d)\n", self(), dlq, item, dlq->Count));
}


/*
 *  Find an item, throught comparing with an external function
 */
int DLQ_find (DLQ_t dlq, void **item, Addr_t addr, Tag_t tag, int type, int mode) {

  register Link *lprev;
  register Link *link;


  *item = NULL;
  if (!dlq->cmp)   return FALSE;

#ifdef __DLQ_DEBUG
  lprev = &(dlq->Queue);
  link  = dlq->Queue.Next;

  DBG_PRNT((stdout, "DLQ_find(%p): dlq: %p (cnt: %d) (rank: %d, tag: %d) (type: %x) (mode: %x)\n", self(), dlq, dlq->Count, addr->Rank, tag, type, mode));

  while (link != lprev) {

    if ((*dlq->cmp) (link, addr, tag, type, mode)) {
      DBG_PRNT((stdout, "--->>> MATCH !! %p\n", link));
    }

    link = link->Next;

  }
#endif

  if (dlq->Count == 0)  return FALSE;

  lprev = &(dlq->Queue);
  link  = dlq->Queue.Next;

  while (link != lprev) {

    if ((*dlq->cmp) (link, addr, tag, type, mode)) {
      *item = link;

      DBG_PRNT((stdout, "DLQ_find(%p): FOUND !! %p\n", self(), link));

      return TRUE;
    }

    link = link->Next;

  }

  DBG_PRNT((stdout, "DLQ_find(%p): NOT FOUND.\n", self()));

  return FALSE;
}


/*
 *  Is the queue empty?
 */
int DLQ_isEmpty (DLQ_t dlq) {

  if (dlq->Count)  return FALSE;
  return TRUE;
}


/*
 *  How many items in the queue
 */
int DLQ_count (DLQ_t dlq) {
  return (dlq->Count);
}



/*
 *  Look for an item in the queue
 */
int DLQ_lookUp (DLQ_t dlq, void *item) {

  register Link *lprev;
  register Link *link;


  lprev = &(dlq->Queue);
  link  = dlq->Queue.Next;

  while (link != lprev) {

    DBG_PRNT((stdout, "DLQ_lookUp(%p): item %p  link %p\n", self(), item, link));

    if (link == item) {
      DBG_PRNT((stdout, "DLQ_lookUp(%p): FOUND !! %p\n", self(), link));
      return TRUE;
    }

    link = link->Next;

  }

  DBG_PRNT((stdout, "DLQ_lookUp(%p): NOT FOUND  %p.\n", self(), item));

  return FALSE;
}
