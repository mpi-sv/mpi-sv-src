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
   |  Description: Implementation of LFS                                   |
   |               LFS is a package that provides a lock-free stack        |
   |                                                                       |
   |_______________________________________________________________________| */

#include <lfs.h>
#include <config.h>
#include <stdlib.h>
#define ATOMIC_BOOL_CMPXCHG(ptr, old_value, new_value) \
        __sync_bool_compare_and_swap((volatile long long * const) (ptr), (const long long) (old_value), (const long long) (new_value))
//#define ATOMIC_BOOL_CMPXCHG(ptr, old_value, new_value) \
//        __sync_bool_compare_and_swap((volatile long * const) (ptr), (const long) (old_value), (const long) (new_value)) 

//#include <stdio.h>
//#define AT()  printf ("at %s:%d in %s()...\n",  __FILE__, __LINE__, __FUNCTION__)
//#define self()        ((Thr_t)pthread_getspecific(key))


      /*________________________________________________________________
     /                                                                  \
    |    LFS_init                                                        |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
void LFS_iNit(LFS_t stack)
{
//fprintf(stdout, "LFS_init(%p): Stack %p. BEGIN\n", self(), stack); fflush(stdout);
  stack->Top = NULL;
//fprintf(stdout, "LFS_init(%p): Stack %p. BEGIN\n", self(), stack); fflush(stdout);
  return;
}


      /*________________________________________________________________
     /                                                                  \
    |    LFS_push                                                        |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
void LFS_pUsh(LFS_t stack, LFS_Link_t link)
{
  LFS_Link_t old;

//fprintf(stdout, "LFS_push(%p): Item %p in Stack %p. BEGIN\n", self(), link, stack); fflush(stdout);
  do {
    old = stack->Top;
    link->Next = old;
  } while (!ATOMIC_BOOL_CMPXCHG(&stack->Top, old, link));

//fprintf(stdout, "LFS_push(%p): END\n", self()); fflush(stdout);
  return;
}


      /*________________________________________________________________
     /                                                                  \
    |    LFS_pop                                                         |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int LFS_pOp(LFS_t stack, LFS_Link_t *link)
{
  LFS_Link_t old, new;

//fprintf(stdout, "LFS_pop(%p): Stack %p. BEGIN\n", self(), stack); fflush(stdout);
  if(stack->Top == NULL)     
    return -1;

  do {
    old = stack->Top;
    new = old->Next;
  } while(!ATOMIC_BOOL_CMPXCHG(&stack->Top, old, new));
  *link = old;
//fprintf(stdout, "LFS_pop(%p): Poped item %p from Stack %p. END\n", self(), *link, stack); fflush(stdout);
  return 0;
}


void LFS_poP(LFS_t stack, LFS_Link_t *link)
{
  LFS_Link_t old, new;

//fprintf(stdout, "LFS_pop(%p): Stack %p. BEGIN\n", self(), stack); fflush(stdout);
  if(stack->Top == NULL)     
    *link = NULL;
  else {
    do {
      old = stack->Top;
      new = old->Next;
    } while(!ATOMIC_BOOL_CMPXCHG(&stack->Top, old, new));
    *link = old;
  }
//fprintf(stdout, "LFS_pop(%p): Poped item %p from Stack %p. END\n", self(), *link, stack); fflush(stdout);
  return;
}




