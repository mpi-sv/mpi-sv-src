#ifndef _LFS_H_
#define _LFS_H_

/* _________________________________________________________________________
   |                                                                       |
   |  AzequiaMPI                                                           |
   |                                                                       |
   |  Authors: DSP Systems Group                                           |
   |           http://gsd.unex.es                                          |
   |           University of Extremadura                                   |
   |           Caceres, Spain                                              |
   |           juancarl@unex.es                                            |
   |                                                                       |
   |  Date:    june 18, 2011                                               |
   |                                                                       |
   |  Description: Specification of LFS                                    |
   |               LFS is a package that provides a lock-free stack        |
   |                                                                       |
   |_______________________________________________________________________| */

#include <arch.h>
#include <config.h>
#include <atomic.h>


struct LFS_Link {
  struct LFS_Link  *Next;
  int               Index;  
};
typedef struct LFS_Link LFS_Link, *LFS_Link_t;

#ifndef CACHE_LINE_SIZE
#error "Must define CACHE_LINE_SIZE"
#endif

#define LFSSize (1 * sizeof (LFS_Link_t))
struct LFS {
  LFS_Link_t  Top __attribute__(( aligned(CACHE_LINE_SIZE) )); 
  char        Pad[(LFSSize % CACHE_LINE_SIZE ? CACHE_LINE_SIZE - (LFSSize % CACHE_LINE_SIZE) : 0)];
};
typedef struct LFS LFS, *LFS_t;


/*----------------------------------------------------------------*
 *   Declaration of public interface                              *
 *----------------------------------------------------------------*/

#define LFS_init(stack) \
{ \
  (stack)->Top = NULL; \
}


#define LFS_push(stack, link) \
{ \
  LFS_Link_t old; \
  \
  do { \
    old = (stack)->Top; \
    (link)->Next = old; \
  } while (!ATOMIC_BOOL_CMPXCHG(&(stack)->Top, old, (link))); \
}


#define LFS_pop(stack, link)  \
{ \
  LFS_Link_t old, new; \
 \
  if((stack)->Top == NULL)      \
    *(link) = NULL; \
  else { \
    do { \
      old = (stack)->Top; \
      new = old->Next; \
    } while(!ATOMIC_BOOL_CMPXCHG(&(stack)->Top, old, new)); \
    *(link) = old; \
  } \
}



#endif
