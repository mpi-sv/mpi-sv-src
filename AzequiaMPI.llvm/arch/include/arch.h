/* _________________________________________________________________________
   |                                                                       |
   |  Azequia (embedded) Message Passing Interface   ( AzequiaMPI )        |
   |                                                                       |
   |  Authors: DSP Systems Group                                           |
   |           http://gsd.unex.es                                          |
   |           University of Extremadura                                   |
   |           Caceres, Spain                                              |
   |           jarico@unex.es                                              |
   |                                                                       |
   |  Date:    Sept 22, 2008                                               |
   |                                                                       |
   |  Description:                                                         |
   |                                                                       |
   |                                                                       |
   |_______________________________________________________________________| */

#ifndef _ARCH_H
#define _ARCH_H

#include <config.h>


#define  AMD64   0x00000001
#define  IA32    0x00000002
#define  IA64    0x00000004


struct cerrojo {
  volatile int Cerrojo /*__attribute__(( aligned(CACHE_LINE_SIZE) ))*/ ;
  //char         Pad[CACHE_LINE_SIZE - sizeof(int)];
};
typedef struct cerrojo cerrojo, *cerrojo_t;



#if (AZQMPI_ARCH == IA64)

#define write_barrier()   __asm__ __volatile__ ( "mf" ::: "memory" )
#define read_barrier()    __asm__ __volatile__ ( "mf" ::: "memory" )

#define wr_barrier()      __asm__ __volatile__ ( "mf" ::: "memory" )

#define CACHE_LINE_SIZE  128


#elif  (AZQMPI_ARCH == IA32 || AZQMPI_ARCH == AMD64)


#define write_barrier()   /*__asm__ __volatile__ ( "sfence" ::: "memory" )*/
#define read_barrier()    /*__asm__ __volatile__ ( "lfence" ::: "memory" )*/

#define wr_barrier()      /*__asm__ __volatile__ ( "mfence" ::: "memory" )*/

#define CACHE_LINE_SIZE  64


#else

#error "Architecture not SUPPORTED"

#endif


#define BACKOFF_MAX (1024 * 4)
//added by Herman. extern.
extern   void  lock     (int *L);
extern   int   trylock  (int *L);
extern   void  unlock   (int *L);


#endif

