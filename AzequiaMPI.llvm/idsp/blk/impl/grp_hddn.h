/*-
 * Copyright (c) 2009 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */


#ifndef _GRP_HDDN_H_
#define _GRP_HDDN_H_

/* Hidden interface of GRP module */

/*----------------------------------------------------------------*
 *   Declaration of types used by this module                     *
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------*
 *   1. Constants                                                 *
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------*
 *   2. Data                                                      *
 *----------------------------------------------------------------*/


/*----------------------------------------------------------------*
 *   3. Declaration of public interface                           *
 *----------------------------------------------------------------*/
extern int GRP_server (int *gix, void (*bodyFxn), int port, int stackSize);


#endif
