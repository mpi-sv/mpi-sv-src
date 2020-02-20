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
#include <config.h>
#include <util.h>
#include <string.h>

/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   used by this module                                          *
 *----------------------------------------------------------------*/
#if defined (__OSI)
  #include <osi.h>
#else
  #include <pthread.h>
  #include <errno.h>
  #include <stdio.h>
  #include <stdlib.h>
//added by Herman
#endif

#if defined (HAVE_CLOCK_GETTIME)
  #include <time.h>
#elif defined (HAVE_GETTIMEOFDAY)
  #include <sys/time.h>
#else
#error "Need to define a TIMER. Allowed kinds are: HAVE_CLOCK_GETTIME, HAVE_GETTIMEOFDAY"
#endif

#include <timer.h>
#include <thr_dptr.h>

/*----------------------------------------------------------------*
 *   Definition of private constants                              *
 *----------------------------------------------------------------*/


/*----------------------------------------------------------------*
 *   Definition of private data                                   *
 *----------------------------------------------------------------*/
extern pthread_key_t  key;

#ifdef __DEBUG_MALLOC
static unsigned int totsize = 0;
#endif

/*----------------------------------------------------------------*
 *   Implementation of private functions                          *
 *----------------------------------------------------------------*/
#define self()        ((Thr_t)pthread_getspecific(key))

/*----------------------------------------------------------------*
 *   Implementation of exported interface                         *
 *----------------------------------------------------------------*/

#if (AZQMPI_ARCH == IA32 || AZQMPI_ARCH == AMD64)

void mmx_memcpy (void *to, const void *from, size_t len) {

  void *p;
  int i;

  p = to;
  i = len >> 6; /* len/64 */

  for(; i>0; i--)
  {
	__asm__ __volatile__ (
						  "2:  movq (%0), %%mm0\n"
						  "  movq 8(%0), %%mm1\n"
						  "  movq 16(%0), %%mm2\n"
						  "  movq 24(%0), %%mm3\n"
						  "  movq %%mm0, (%1)\n"
						  "  movq %%mm1, 8(%1)\n"
						  "  movq %%mm2, 16(%1)\n"
						  "  movq %%mm3, 24(%1)\n"
						  "  movq 32(%0), %%mm0\n"
						  "  movq 40(%0), %%mm1\n"
						  "  movq 48(%0), %%mm2\n"
						  "  movq 56(%0), %%mm3\n"
						  "  movq %%mm0, 32(%1)\n"
						  "  movq %%mm1, 40(%1)\n"
						  "  movq %%mm2, 48(%1)\n"
						  "  movq %%mm3, 56(%1)\n"
						  : : "r" (from), "r" (to) : "memory");
	from+=64;
	to+=64;
  }

  /*
   *Now do the tail of the block
   */
  memcpy(to, from, len&63);
}

/*
#if defined (_INTEL)
#include <xmmintrin.h>
#else
#include "xmmintrin.h"
#endif
*/
#define BLOCKSIZE  131072     /* Needs to be divisible by 16 */
#define NUMPERPAGE 512        /* Number of elements fit in a page */
#define ALIGNMENT  16

void memcpy_16(void *destination, const void *source, size_t nbytes)
{
  int nb_b4, nb_after;
  char *dest = (char *)destination, *src = (char *)source;

  nb_b4 = 16 - ((int) dest % 16);
  if (nb_b4 != 16 && nb_b4 <= nbytes)
  {
    memcpy(dest, src, nb_b4);
    src += nb_b4;
    dest += nb_b4;
    nbytes -= nb_b4;
  }

  /*memcpy(dest, src, nbytes);  */
  nb_after = nbytes % 16;
  nbytes -= nb_after;

  if ( nbytes > 0) {
    memcpy(dest, src, nbytes);
  }

  if( nb_after > 0 ) {
    src += nbytes;
    dest += nbytes;
    memcpy( dest, src, nb_after );
  }
}

//#if defined(_INTEL)


#elif (AZQMPI_ARCH == IA64)

#else

#error "Architecture not supported!!"

#endif


#ifdef __DEBUG_MALLOC

void *AZQ_MALLOC (unsigned int size) {

  void *ptr;

  ptr = malloc(size);
  totsize += size;
  fprintf(stdout, "Reserved %d (acum: %d) bytes at %p\n", size, totsize, ptr);

  return (ptr);
}

void AZQ_FREE (void *ptr) {
  fprintf(stdout, "Freeing memory at %p\n", ptr);
  free(ptr);
}

#endif



     /*¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯\
    |    milliRel2posixAbs                                               |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
void milliRel2posixAbs(unsigned milliseconds, struct timespec *posix) {

  int seconds;

#if defined (HAVE_CLOCK_GETTIME)

  clock_gettime(CLOCK_REALTIME, posix);

#elif defined (HAVE_GETTIMEOFDAY)

  struct timeval tm;

  gettimeofday(&tm, NULL);

  posix->tv_sec  = tm.tv_sec;
  posix->tv_nsec = tm.tv_usec / 1.0e3;

#endif

  seconds       = milliseconds / 1000;
  milliseconds  = milliseconds % 1000;
#ifdef __UTIL_DEBUG
  fprintf(stdout, "REL:   %12ld seconds\n",     (unsigned long)seconds);
  fprintf(stdout, "REL:   %12ld milliseconds\n",(unsigned long)milliseconds);
  fprintf(stdout, "NOW:   %12ld seconds\n",     (unsigned long)(posix->tv_sec));
  fprintf(stdout, "NOW:   %12ld nanoseconds\n",                 posix->tv_nsec);
#endif
  posix->tv_sec  += seconds;
  posix->tv_nsec += milliseconds * 1.0e6;
  if(posix->tv_nsec >= 1.0e9) {
    posix->tv_nsec -= 1.0e9;
    posix->tv_sec  += 1;
  }
#ifdef __UTIL_DEBUG
  fprintf(stdout, "LATER: %12ld seconds\n",     (unsigned long)(posix->tv_sec));
  fprintf(stdout, "LATER: %12ld nanoseconds\n", posix->tv_nsec);
#endif

  return;
}



      /*________________________________________________________________
     /                                                                  \
    |    panic                                                           |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
void panic(char *where) {

  fprintf(stdout, "Panic on %s (thread %p) !!\n", where, self());
  exit(1);
  return;
}
