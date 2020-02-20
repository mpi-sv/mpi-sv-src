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
inline void lock(int *L) 
{
  int  old, new;
  int  i, delay = 1;
  do {
    for (i = 0; i < delay; i++);
    if(delay < BACKOFF_MAX)
      delay = delay * 2;
    old = 0;
    new = 1;
  } while(!__sync_bool_compare_and_swap(L, old, new));
  return;
}


inline int trylock(int *L) 
{
  int  old, new, delay = 1, i;
  old = 0;
  new = 1;
  if(!__sync_bool_compare_and_swap(L, old, new))
    return -1;
  return 0;
}

inline void unlock(int *L) 
{
  *L = 0;
}
