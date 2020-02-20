/*-
 * Copyright (c) 2009 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */


/*

   Extended Limited Buffer.

   This module implements an extended limited buffer. Its behaviour is
   similar to a limited buffer. The items are put at the end, but can be
   retrieved from anywhere. Two functions allows get a pointer to an item
   in the buffer to be read and avoid extra copies.

 */


#ifndef _ELB_H
#define _ELB_H

/*----------------------------------------------------------------*
 *   Definition of public data types                              *
 *----------------------------------------------------------------*/
typedef struct ELB  *ELB_t;

/*----------------------------------------------------------------*
 *   Declaration of public interface                              *
 *----------------------------------------------------------------*/
int   ELB_create   (ELB_t *elb, int item_size, int item_nr, int fixed);
void  ELB_destroy  (ELB_t  elb);

int   ELB_put      (ELB_t  elb, void  *item);
int   ELB_get      (ELB_t  elb, void **item);

int   ELB_newElem  (ELB_t  elb, void **item);
void  ELB_freeElem (ELB_t  elb, void  *item);


#endif
