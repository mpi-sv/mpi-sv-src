/*-
 * Copyright (c) 2009 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */


/*

   Double Linked Queue

   This module implements a dinamic queue doubled linked. Its behaviour is
   similar to a limited buffer. The items are put at the end, but can be
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
#define DBG_PRNT(pmsg)  fprintf pmsg
#else
#define DBG_PRNT(pmsg)
#endif

/*----------------------------------------------------------------*
 *   Definition of public data types                              *
 *----------------------------------------------------------------*/
typedef struct DLQ  *DLQ_t;

/* Double linked list of items in the queue */
struct Link {
  struct Link  *Next;
  struct Link  *Prev;
};
typedef struct Link Link, *Link_t;

/*----------------------------------------------------------------*
 *   Declaration of public interface                              *
 *----------------------------------------------------------------*/
extern int   DLQ_create   (DLQ_t *dlq, int (*cmp_fxn) (void *item, Addr_t addr, Tag_t tag, int type, int mode));
extern void  DLQ_destroy  (DLQ_t  dlq);

extern int   DLQ_put      (DLQ_t  dlq, void   *item);
extern int   DLQ_get      (DLQ_t  dlq, void  **item);

extern int   DLQ_find     (DLQ_t  dlq, void  **item, Addr_t addr, Tag_t tag, int type, int mode);
extern void  DLQ_delete   (DLQ_t  dlq, void   *item);

extern int   DLQ_isEmpty  (DLQ_t  dlq);
extern int   DLQ_count    (DLQ_t  dlq);

extern int   DLQ_lookUp   (DLQ_t dlq, void *item);

#endif
