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
#include <elb.h>

/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   used by this module                                          *
 *----------------------------------------------------------------*/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>


/* Debug pourpose only */
#ifdef __DEBUG_MALLOC
extern  void  *INET_MALLOC            (unsigned int size);
extern  void   INET_FREE              (void *ptr);
#else
#define INET_MALLOC malloc
#define INET_FREE   free
#endif

/*----------------------------------------------------------------*
 *   Definition of private types                                  *
 *----------------------------------------------------------------*/
/* Double linked list of items in the extended limited buffer */
typedef struct BUCKET {
  struct BUCKET    *Next;
  struct BUCKET    *Prev;
} BUCKET;

/* Struct for management of buffer */
struct ELB {
  char             *Buffer;      /* Static storage */

  BUCKET            A_Queue;     /* Head of the queue of allocated elements */
  int               A_Count;     /* Number of elements currently in queue */

  BUCKET           *H_Queue;     /* Head of the queue of free elements */
  int               H_Count;     /* Number of elements currently not allocated */

  int               Item_Size;   /* Size of each item in queue */
  int               Item_Count;  /* Number of items allocated */
  int               Fixed;       /* Not dinamic grow of items in buffer */

  /* FOR TESTING:                                                                */
  /* Synchronization for accessing the queue of allocated and free items:        */
  /*  - Condition variables avoid buffer overflow, stopping the calling          */
  /*    thread if buffer full. If buffer politic allow lost of messages          */
  /*    (eg. soft realtime) only a mutex would be needed                         */
  pthread_mutex_t   A_Mtx;
  pthread_cond_t    A_CvEmpty;

  pthread_mutex_t   H_Mtx;
  pthread_cond_t    H_CvEmpty;

};


/*----------------------------------------------------------------*
 *   Declaration of private functions                             *
 *----------------------------------------------------------------*/
static int init_queues (ELB_t p);

/*----------------------------------------------------------------*
 *   Implementation of private functions                          *
 *----------------------------------------------------------------*/

/* Malloc of static storage and initialize queues */
static int init_queues (ELB_t p) {

  BUCKET  *b,
          *b1;
  int      i;


  if (!(b = (BUCKET *)INET_MALLOC(p->Item_Size * p->Item_Count)))
   return -1;

  p->H_Queue = (BUCKET *)((char *)b);
  p->H_Count = p->Item_Count;
  p->Buffer  = (char *)b;

  b1 = (BUCKET *)((char *)b + p->Item_Size);
  for (i = 0; i < p->Item_Count - 1; i++) {
    b->Prev = b1;
    b       = b1;
    b1      = (BUCKET *)((char *)b + p->Item_Size);
  }

  b->Prev = NULL;

  return 0;
}


/*----------------------------------------------------------------*
 *   Implementation of public function interface                  *
 *----------------------------------------------------------------*/

     /*__________________________________________________________________
    /                                                                    \
    |    ELB_create                                                      |
    |                                                                    |
    |    Create an Extended Limited Buffer                               |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |                                                                    |
    |     o  elb        (IN/OUT)                                         |
    |          Pointer address for the new buffer                        |
    |     o  item_size  (IN)                                             |
    |          Size of each item in the static storage                   |
    |     o  item_count (IN)                                             |
    |          Number of item in the static storage                      |
    |     o  fixed      (IN)                                             |
    |          TRUE if buffer size fixed at init, FALSE if dynamic       |
    |          allocation of new items allowed                           |
    |                                                                    |
    |    RETURN:                                                         |
    |     = 0 : On success                                               |
    |     < 0 : In other case                                            |
    |                                                                    |
    \_____________/  ___________________________________________________/
                 / _/
                /_/
               */
int ELB_create (ELB_t *elb, int item_size, int item_count, int fixed) {

  ELB_t  p;

  if ((item_size <= 0) || (item_count <= 1))                                   goto exception_0;

  if (!(p = (ELB_t)INET_MALLOC(sizeof(struct ELB))))             goto exception_0;

  /* Init the allocated items and holes queue */
  p->A_Count      = 0;
  p->A_Queue.Prev = p->A_Queue.Next = &(p->A_Queue);
  p->H_Queue      = NULL;

  /* Item_size must be a multiple of 8 */
  if (item_size % 0x07)
    p->Item_Size = (item_size + 7) >> 3 << 3;
  else
    p->Item_Size = item_size;

  /* Init synchronization */
  if (pthread_mutex_init(&p->A_Mtx,     NULL))                                 goto exception_1;
  if (pthread_cond_init (&p->A_CvEmpty, NULL))                                 goto exception_2;

  if (pthread_mutex_init(&p->H_Mtx,     NULL))                                 goto exception_3;
  if (pthread_cond_init (&p->H_CvEmpty, NULL))                                 goto exception_4;

  p->Item_Size += sizeof(BUCKET);
  p->Item_Count = item_count;
  p->Fixed      = fixed;

  /* Malloc static storage */
  if (0 > init_queues(p))                                                      goto exception_5;

  *elb = p;

  return 0;

exception_5:
  pthread_cond_destroy (&p->H_CvEmpty);
exception_4:
  pthread_mutex_destroy(&p->H_Mtx);
exception_3:
  pthread_cond_destroy (&p->A_CvEmpty);
exception_2:
  pthread_mutex_destroy(&p->A_Mtx);
exception_1:
  INET_FREE(p);
exception_0:
  return -1;
}


     /*__________________________________________________________________
    /                                                                    \
    |    ELB_destroy                                                     |
    |                                                                    |
    |    Destroy an Extended Limited Buffer                              |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |                                                                    |
    |     o  elb        (IN)                                             |
    |          Pointer address for the buffer                            |
    |                                                                    |
    |    RETURN:                                                         |
    |       none                                                         |
    |                                                                    |
    \_____________/  ___________________________________________________/
                 / _/
                /_/
               */
void ELB_destroy (ELB_t elb) {

  /* 1. Free condition variables and mutex */
  pthread_mutex_destroy(&elb->H_Mtx);
  pthread_cond_destroy (&elb->H_CvEmpty);

  pthread_mutex_destroy(&elb->A_Mtx);
  pthread_cond_destroy (&elb->A_CvEmpty);

  /* 2. Free memory reserved for packets */
  INET_FREE(elb->Buffer);
  INET_FREE(elb);
}


     /*__________________________________________________________________
    /                                                                    \
    |    ELB_newElem                                                     |
    |                                                                    |
    |    Get a pointer to a non allocated item                           |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |                                                                    |
    |     o  elb        (IN)                                             |
    |          Pointer to a buffer                                       |
    |     o  item       (IN/OUT)                                         |
    |          Pointer address of the returned item                      |
    |                                                                    |
    |    RETURN:                                                         |
    |     = 0 : On success                                               |
    |     < 0 : In other case                                            |
    |                                                                    |
    \_____________/  ___________________________________________________/
                 / _/
                /_/
               */
int ELB_newElem (ELB_t elb, void **item) {

  BUCKET           *elem;

  pthread_mutex_lock(&elb->H_Mtx);
  while(elb->H_Count == 0)
    pthread_cond_wait(&elb->H_CvEmpty, &elb->H_Mtx);

  elem         = elb->H_Queue;
  elb->H_Queue = elb->H_Queue->Prev;
  elem->Prev   = NULL;

  elb->H_Count--;

  pthread_mutex_unlock(&elb->H_Mtx);

  /* 3. Return pointer to user space (saving BUCKET) */
  *item = (void *)(elem + 1);
  return 0;
}


     /*__________________________________________________________________
    /                                                                    \
    |    ELB_freeElem                                                    |
    |                                                                    |
    |    Return an item to the holes queue                               |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |                                                                    |
    |     o  elb        (IN)                                             |
    |          Pointer to a buffer                                       |
    |     o  item       (IN)                                             |
    |          Pointer of the item                                       |
    |                                                                    |
    |    RETURN:                                                         |
    |      none                                                          |
    |                                                                    |
    \_____________/  ___________________________________________________/
                 / _/
                /_/
               */
void ELB_freeElem (ELB_t elb, void *item) {

  BUCKET *elem;

  pthread_mutex_lock(&elb->H_Mtx);

  elem         = (BUCKET *)item - 1;
  elem->Prev   = elb->H_Queue;
  elb->H_Queue = elem;

  elb->H_Count++;

  pthread_cond_signal(&elb->H_CvEmpty);
  pthread_mutex_unlock(&elb->H_Mtx);

  return;
}


     /*__________________________________________________________________
    /                                                                    \
    |    ELB_put                                                         |
    |                                                                    |
    |    Put an item in the allocated queue                              |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |                                                                    |
    |     o  elb        (IN)                                             |
    |          Pointer to a buffer                                       |
    |     o  item       (IN)                                             |
    |          Pointer of the item                                       |
    |                                                                    |
    |    RETURN:                                                         |
    |     = 0 : On success                                               |
    |     < 0 : In other case                                            |
    |                                                                    |
    \_____________/  ___________________________________________________/
                 / _/
                /_/
               */
int ELB_put (ELB_t elb, void  *item) {

  BUCKET *elem;

  pthread_mutex_lock(&elb->A_Mtx);

  elem       = (BUCKET *)item - 1;
  elem->Next = &(elb->A_Queue);
  elem->Prev = elb->A_Queue.Prev;

  elb->A_Queue.Prev->Next = elem;
  elb->A_Queue.Prev       = elem;

  elb->A_Count++;

  pthread_cond_signal(&elb->A_CvEmpty);
  pthread_mutex_unlock(&elb->A_Mtx);

  return 0;
}


     /*__________________________________________________________________
    /                                                                    \
    |    ELB_get                                                         |
    |                                                                    |
    |    Get an item from the allocated queue                            |
    |                                                                    |
    |    PARAMETERS:                                                     |
    |                                                                    |
    |     o  elb        (IN)                                             |
    |          Pointer to a buffer                                       |
    |     o  item       (IN)                                             |
    |          Pointer address of the returned item                      |
    |                                                                    |
    |    RETURN:                                                         |
    |     = 0 : On success                                               |
    |     < 0 : In other case                                            |
    |                                                                    |
    \_____________/  ___________________________________________________/
                 / _/
                /_/
               */
int ELB_get (ELB_t elb, void **item) {

  BUCKET *elem;

  pthread_mutex_lock(&elb->A_Mtx);

  while(elb->A_Count == 0) {
    pthread_cond_wait(&elb->A_CvEmpty, &elb->A_Mtx);
  }

  elem = elb->A_Queue.Next;
  elem->Prev->Next = elem->Next;
  elem->Next->Prev = elem->Prev;

  elb->A_Count--;

  pthread_mutex_unlock(&elb->A_Mtx);

  *item = (void *)(elem + 1);
  return 0;
}


