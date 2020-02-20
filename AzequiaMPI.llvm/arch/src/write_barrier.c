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

#include <arch.h>

  /*----------------------------------------------------------------/
 /   Declaration of types and functions used by this module        /
/----------------------------------------------------------------*/
/*
 *  write_barrier
 *    Implements a write barrier for different architectures
 */
void _write_barrier () {
 
#if (AZQMPI_ARCH == IA64)  
  
  __asm__ __volatile__ ( "mf" ::: "memory" );
  
#elif  (AZQMPI_ARCH == IA32 || AZQMPI_ARCH == AMD64)
  
  __asm__ __volatile__ ( "sfence" ::: "memory" );
  
#else
  
#error "Architecture not SUPPORTED"
  
#endif
  
}