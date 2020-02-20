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

#ifndef _OBJECT_H
#define _OBJECT_H

  /*----------------------------------------------------------------/
 /   Declaration of types and functions used by this module        /
/----------------------------------------------------------------*/
#include <config.h>

#include <p_config.h>
#include <lfs.h>

  /*-------------------------------------------------------/
 /                    Public types                        /
/-------------------------------------------------------*/
struct Object_Link {
  void            *Next;
  int              Index;
};
typedef struct Object_Link Object_Link, *Object_Link_t;

/* An object reprenting an opaque MPI object */
struct Object {
  int              Initialized;
  int              Item_Size;    /* Size of an item */
  LFS              Avail; 
  void           *(*Heap)[];
  int              Heap_Size;    /* Max of blocks that can be created */
  int              Heap_Count;   /* How many current blocks used */
  int              Block_Size;   /* Number of items in a block */
};
typedef struct Object Object, *Object_t;

  /*-------------------------------------------------------/
 /         Utility macros                                 /
/-------------------------------------------------------*/
extern void  *AZQMPI_blockAdd(Object_t object, int item_cnt, int item_sz);

/**
 *  blockAdd
 *
 *    Adding a block to the heap
 */

//void AZQMPI_blockAdd(Object_t object, int item_cnt, int item_sz) 
#define AZQMPI_blockAdd(object, item_cnt, item_sz)                               \
do {                                                                             \
  void         *block0;                                                          \
  void         *block2;                                                          \
  int           index;                                                           \
                                                                                 \
  if ((object)->Heap_Count == (object)->Heap_Size) {                             \
    /*printf("AZQMPI_blockAdd(%p): Heap full\n", PCS_self());                    */  \
    break;                                                                       \
  }                                                                              \
  if (posix_memalign(&block0, CACHE_LINE_SIZE, (item_cnt) * (item_sz))) break;   \
  index = (object)->Heap_Count * (item_cnt);                                     \
  {                                                                              \
    int          i;                                                              \
    LFS_Link_t   addr = (LFS_Link_t)block0;                                      \
    for (i = 0; i < (item_cnt); i++) {                                           \
      addr->Index = index + i;                                                   \
      LFS_push(&(object)->Avail, addr);                                          \
      block2 = (void *)addr;                                                     \
      block2 += (item_sz);                                                       \
      addr = (LFS_Link_t )block2;                                                \
    }                                                                            \
  }                                                                              \
  (*(object)->Heap)[(object)->Heap_Count] = block0;                              \
  (object)->Heap_Count++;                                                        \
} while(0);



   /*-------------------------------------------------------/
  /         Declaration of exported functions              /
 /        (Implemented as macros for speed)               /
/-------------------------------------------------------*/
/*
extern void   objInit     (Object_t *object, int max_blk, int item_sz, int prealloc_item_cnt, int block_size, int *error);
extern void   objFinalize (Object_t *object);
extern void   objGet      (Object_t  object, int index,   void **item); 
extern void   objAlloc    (Object_t  object, void **item, int *index);    Implemented as macros for speed
extern void   objFree     (Object_t  object, void **item, int  index);
*/



//int objInit (Object_t *object, int max_blk, int item_sz, int prealloc_item_cnt, int block_size) 
#define objInit(object, max_blk, item_sz, prealloc_item_cnt, block_size, error)             \
do {                                                                                        \
  Object *obj;                                                                              \
                                                                                            \
  *error = 0;                                                                               \
  if (posix_memalign((void *)&obj, CACHE_LINE_SIZE, sizeof(Object))) {                      \
    *error = 1;                                                                             \
    break;                                                                                  \
  }                                                                                         \
  obj->Heap_Count = 0;                                                                      \
  obj->Heap_Size  = (max_blk);                                                              \
  LFS_init(&obj->Avail);                                                                    \
  if (posix_memalign((void *)&(obj->Heap), CACHE_LINE_SIZE, (max_blk) * sizeof(void *))) {  \
    free(obj);                                                                              \
    *error = 1;                                                                             \
    break;                                                                                  \
  }                                                                                         \
  if ((prealloc_item_cnt) > 0) {                                                            \
    AZQMPI_blockAdd(obj, (prealloc_item_cnt), (item_sz));                                   \
  }                                                                                         \
  obj->Initialized  = TRUE;                                                                 \
  obj->Item_Size    = (item_sz);                                                            \
  obj->Block_Size   = (block_size);                                                         \
                                                                                            \
  *(object) = obj;                                                                          \
} while(0);


/**
 *  objFinalize
 */
//void objFinalize (Object_t *object) 
#define objFinalize(object)              \
{                                        \
  Object_t obj = *(object);              \
  int      i;                            \
                                         \
  for (i = 0; i < obj->Heap_Count; i++)  \
    free((*obj->Heap)[i]);               \
  free(obj->Heap);                       \
  free(obj);                             \
}





/**
 *  objAlloc
 */
#define  objAlloc(object, item, index)                                      \
{                                                                           \
  LFS_Link_t link;                                                          \
                                                                            \
  while(1) {                                                                \
    LFS_pop(&(object)->Avail, &link);                                       \
    if(NULL == link) {                                                      \
      AZQMPI_blockAdd((object), (object)->Block_Size, (object)->Item_Size); \
      continue;                                                             \
    }                                                                       \
    *(item) = link;                                                         \
    *(index) = link->Index;                                                 \
    break;                                                                  \
  }                                                                         \
}





/**
 *  objFree
 */
#define objFree(object, item, index)                                       \
{                                                                          \
  ((Object_Link_t)(*(item)))->Index = (index);                             \
  LFS_push(&(object)->Avail, (Object_Link_t)(*(item)));                    \
}


  //objGet(commtab->Comms, index, &comm);

/** 
 *  objGet
 *
 *    Access to an object by Index in Heap/Block
 */
#define objGet(object, index, item)                     \
do {                                                    \
  int    in_heap;                                       \
  int    in_block;                                      \
  char  *block;                                         \
                                                        \
  if ((index) < 0) {                                    \
    *(item) = NULL;                                     \
    break;                                              \
  }                                                     \
  in_heap  = (index) / (object)->Heap_Size;             \
  in_block = (index) % (object)->Block_Size;            \
  block = (*(object)->Heap)[in_heap];                   \
  *(item) = block + ((object)->Item_Size * in_block);   \
} while(0);


#endif
