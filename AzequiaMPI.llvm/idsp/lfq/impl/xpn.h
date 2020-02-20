/*-
 * Copyright (c) 2009 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */


#ifndef _XPN_H_
#define _XPN_H_

/*----------------------------------------------------------------*
 *   1. Declaration of types and functionsused by this module     *
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------*
 *   2. Definition of exported constants                          *
 *----------------------------------------------------------------*/
#if defined (__OSI)
  #include <osi.h>
#else
  #include <stdio.h>
#endif

#ifdef  __XPN_PRINT
#  include <thr.h>
#  define XPN_print(excpn)  \
                          { \
                            if(excpn < 8) { \
                              fprintf(stdout, "\t[Machine %d] >>> Exception %s\n", getCpuId(), e_names[-(excpn)]); \
                              fprintf(stdout, "\t                 Raised by thread %p in function %s \n", THR_self(), where); \
                              fflush(stdout); \
                            } \
                          }

#else
#  define XPN_print(excpn)
#endif

#endif

