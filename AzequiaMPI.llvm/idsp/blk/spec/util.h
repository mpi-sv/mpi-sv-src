/*-
 * Copyright (c) 2009 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */


#ifndef _UTIL_H_
#define _UTIL_H_

/*----------------------------------------------------------------*
 *   Declaration of types used by this module                     *
 *----------------------------------------------------------------*/
#if defined (__OSI)
  #include <osi.h>
#else
  #include <pthread.h>
#endif

#include <atomic.h>
#include <config.h>

/*----------------------------------------------------------------*
 *   Exported function interface                                  *
 *----------------------------------------------------------------*/

extern void mmx_memcpy (void *to, const void *from, size_t len);
extern void memcpy_16  (void *to, const void *from, size_t len);
extern void ntcopy     (void *to, const void *from, size_t len);

#if (AZQMPI_ARCH == IA32 || AZQMPI_ARCH == AMD64)


#define small_memcpy(to,from,n)                                   \
{                                                                 \
    register unsigned long int dummy;                             \
	void *_dst = (to);                                            \
    const void *_src = (from);                                    \
    __asm__ __volatile__( "rep; movsb"                            \
                          :"=&D"(_dst), "=&S"(_src), "=&c"(dummy) \
                          :"0" (_dst), "1" (_src),"2" (n)         \
                          : "memory");                            \
}


//#define MEMCPY(DST, SRC, BLENGTH)   small_memcpy( (DST), (SRC), (BLENGTH) )

/*
#define MEMCPY( DST, SRC, BLENGTH ) \
   { \
    if( (BLENGTH) <= (128) ) { \
      small_memcpy( (DST), (SRC), (BLENGTH) ); \
    } \
    else if ((BLENGTH) <= (16 * 1024) ) {\
      memcpy( (DST), (SRC), (BLENGTH) ); \
    }  \
    else { \
      small_memcpy( (DST), (SRC), (BLENGTH) ); \
    } \
  } 
*/
#define MEMCPY( DST, SRC, BLENGTH ) \
{ \
    if ((BLENGTH) <= (16 * 1024) ) \
      memcpy( (DST), (SRC), (BLENGTH) ); \
    else  \
      small_memcpy( (DST), (SRC), (BLENGTH) ); \
}

#elif (AZQMPI_ARCH == IA64)

#define MEMCPY memcpy

#else

#error "Architecture not supported!!"

#endif


#ifdef __DEBUG_MALLOC
extern  void  *AZQ_MALLOC            (unsigned int size);
extern  void   AZQ_FREE              (void *ptr);
#else
#define AZQ_MALLOC malloc
#define AZQ_FREE   free
#endif

extern  void   milliRel2posixAbs (unsigned milliseconds, struct timespec *posix);
extern  void   panic             (char *where);

#endif
