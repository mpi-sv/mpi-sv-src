/*-
 * Copyright (c) 2009 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */


#ifndef _ADDR_H_
#define _ADDR_H_

/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   used by this module                                          *
 *----------------------------------------------------------------*/
#include <config.h>

/*----------------------------------------------------------------*
 *   1. Declaration of public constants                           *
 *----------------------------------------------------------------*/
#define ADDR_GRP_ANY        0x7FFFFFFF
#define ADDR_RNK_ANY        0x7FFFFFFF

#define DFLT_MCHN           ((Mchn_t)0xFFFF)

/*----------------------------------------------------------------*
 *   2. Declaration of public types                               *
 *----------------------------------------------------------------*/
typedef int Tag_t;

struct Addr {
  int Group;
  int Rank;
};
typedef struct Addr Addr, *Addr_t;

#define ADDR_setAny(addr)                             \
                   (addr).Group = ADDR_GRP_ANY;       \
                   (addr).Rank  = ADDR_RNK_ANY;


#define ADDR_isAny(addr)                              \
                  (((addr)->Group == ADDR_GRP_ANY) || \
                   ((addr)->Rank  == ADDR_RNK_ANY))

/* Support for AzequiaMPI tag match. In AzqMPI the tag is a bit-field
   and need to define TAG_ANY accordingly. For Azequia without MPI support
   the tag can be a full 32-bit integer
 */
//#define AZQ_MPI
#ifdef AZQ_MPI

#define TAG_ANY             0x00008000

#define TAG_MATCH(tag, tagref)                                              \
                  (                                                         \
                   ((((tag) & 0x0000FFFF) == ((tagref) & 0x0000FFFF)) ||    \
                    ((((tag) & 0x0000FFFF) == TAG_ANY))               &&    \
                    (((tagref) & 0x0000FFFF) < ((tag) & 0x0000FFFF))) &&    \
                   (((tag) & 0xFFFF0000) == ((tagref) & 0xFFFF0000))        \
                  )

#else

#define TAG_ANY             0x7FFFFFFF

#define TAG_MATCH(tag, tagref)                               \
                  (                                          \
                   ((tag) == (tagref)) || ((tag) == TAG_ANY) \
                  )

#endif


#endif
