/*-
 * Copyright (c) 2009 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */


#ifndef _ADDR_HDDN_H_
#define _ADDR_HDDN_H_

/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   used by this module                                          *
 *----------------------------------------------------------------*/
#include <addr.h>

/*----------------------------------------------------------------*
 *   1. Declaration of public constants                           *
 *----------------------------------------------------------------*/


/*----------------------------------------------------------------*
 *   2. Declaration of public types                               *
 *----------------------------------------------------------------*/


/*----------------------------------------------------------------*
 *   3. Declaration of public macro interface                     *
 *----------------------------------------------------------------*/
#define ADDR_match(addr, dst) (                                                                         \
                                /*(((dst)->Group == ADDR_GRP_ANY) || ((dst)->Group == (addr)->Group)) && */ \
                                (((dst)->Rank  == ADDR_RNK_ANY) || ((dst)->Rank  == (addr)->Rank))      \
                              )
#define ADDR_equal(addr_1, addr_2) (                                          \
                                     ((addr_1)->Group == (addr_2)->Group) &&  \
                                     ((addr_1)->Rank  == (addr_2)->Rank)      \
                                   )

#endif
