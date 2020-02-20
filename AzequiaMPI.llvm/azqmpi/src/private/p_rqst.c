/******************************/  /*-
 * Copyright (c) 2009-2011 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */

  /*----------------------------------------------------------------/
 /   Declaration of public functions implemented by this module    /
/----------------------------------------------------------------*/
#include <p_rqst.h>

  /*----------------------------------------------------------------/
 /   Declaration of public functions used by this module           /
/----------------------------------------------------------------*/
#if defined (__OSI)
  #include <osi.h>
#else
  #include <stdio.h>
  #include <string.h>
  #include <pthread.h>
#endif

#include <azq_types.h>
#include <com.h>
#include <rqst.h>

#include <object.h>
#include <env.h>
#include <p_config.h>

#include <assert.h>
  /*----------------------------------------------------------------/
 /             Definition of macros and constants                  /
/----------------------------------------------------------------*/
/* To avoid too much internal fragmentation */
#define  MIN_SEGMENT_SIZE           (0 + INFO_SEGMENT_SIZE)
#define  MPI_RQST_ALLOCATED          0x00010000


  /*----------------------------------------------------------------/
 /                  Definition of opaque types                     /
/----------------------------------------------------------------*/
struct RqstTable {
/* User buffer support */
  char           *AttachedBuf;
  int             Length;           /* Lenth in bytes of the user buffer */
  SegmentInfo_t   Avail;            /* Segments avail for using */
  SegmentInfo_t   Active;           /* Segments currently used */

/* User Requests Entry Table */
  Object_t        Rqst;
};


  /*-------------------------------------------------------/
 /         Declaration of private interface               /
/-------------------------------------------------------*/
static inline  int      RQST_CompleteBsendRqsts  (RqstTable *rqsttab);

#ifdef DEBUG_MODE
PRIVATE  int            printRequests            (RqstTable *rqsttab);
#endif

static inline int rqstFreeSegment (RqstTable *rqsttab, SegmentInfo *segment);


  /*-------------------------------------------------------/
 /                   Private interface                    /
/-------------------------------------------------------*/
/*
 *  RQST_completeBsendRqsts
 *    Free completed BSEND_RQST_TYPE requests in the active list.
 */
static inline int RQST_CompleteBsendRqsts (RqstTable *rqsttab) 
{
  SegmentInfo   *active = rqsttab->Active;
  SegmentInfo   *active_next;
  Mpi_P_Request *rqst;
  int            flag;
  
#ifdef DEBUG_MODE
  printf("RQST_CompleteBsendRqsts (%p): BEGIN\n", PCS_self());
#endif
  
  while (active) {
    active_next = active->Next;
    rqst = &active->SubRqst;
	
    if (0 > test(&rqst, &flag, NULL)) {    
      fprintf("RQST_CompleteBsendRqsts (%p): Error in test. END\n", PCS_self()); fflush(stdout); 
      return RQT_E_INTEGRITY;
    }
    if (flag)
      rqstFreeSegment(rqsttab, (SegmentInfo_t)(rqst)->PackedBuffer);
    active = active_next;
  }
  
#ifdef DEBUG_MODE
  printf("RQST_CompleteBsendRqsts (%p): END\n", PCS_self());
#endif
  return RQT_E_OK;
}



  /*-------------------------------------------------------/
 /               Public request interface                 /
/-------------------------------------------------------*/
/*
 *  rqstAllocTable():
 *    Alloc space for the table with user requests and management of
 *    buffered request
 */
static int  registered = 0;

int rqstAllocTable (RqstTable_t *rqsttab) {

  RqstTable   *rqstTable;
  int          i, error;

  /* 1. Allocate request table */
  if (posix_memalign(&rqstTable, CACHE_LINE_SIZE, sizeof(RqstTable)))
    return RQST_E_EXHAUST;
  
  /* 2. Allocate communication requests array */
  objInit (&rqstTable->Rqst, PREALLOC_MAX_RQSTS_BLOCKS,
                             sizeof(Mpi_P_Request), 
                             PREALLOC_REQUEST_COUNT, 
                             BLOCK_REQUEST_COUNT, &error);
  if(error) {
    free(rqstTable);
    return RQST_E_EXHAUST;
  }
  
  /* 3. User buffer allocation */
  rqstTable->AttachedBuf  = (char *)NULL;
  rqstTable->Length       = 0;

  *rqsttab = rqstTable;

  return ((PREALLOC_REQUEST_COUNT * sizeof(Mpi_P_Request)) + sizeof (RqstTable));
}


/*
 *  rqstFreeTable():
 *   Free the Pending Request Table
 */
int rqstFreeTable (RqstTable_t *rqsttab) {
  
  objFinalize(&(*rqsttab)->Rqst);
   
  free(*rqsttab);
  *rqsttab = NULL;

  return RQT_E_OK;
}


/*
 *  rqstBufferAttach
 */
int rqstBufferAttach (RqstTable *rqsttab, void *buffer, int size) {

#ifdef DEBUG_MODE
  printf("rqstBufferAtach: rqsttab 0x%x  [BUF: 0x%x  SIZE: %d]\n", (int)rqsttab, (int)buffer, size);
#endif

  /* 1. User buffer already attached */
  if (rqsttab->Length != 0)   return RQT_E_INTEGRITY;

  /* 2. New attached buffer */
  rqsttab->AttachedBuf = buffer;
  rqsttab->Length      = size;

  /* 3. The avail list is the whole buffer segment (aligned) */
  rqsttab->Avail       = (SegmentInfo *)buffer;
  rqsttab->Avail->Size = size;
  rqsttab->Avail->Next = NULL;
  rqsttab->Avail->Prev = NULL;

  /* 4. There are not active segments */
  rqsttab->Active      = NULL;

  return RQT_E_OK;
}


/*
 *  rqstBufferDetach
 */
int rqstBufferDetach (RqstTable *rqsttab, void **buffer, int *size)
{
  Mpi_P_Request  *rqst;
  SegmentInfo    *active;
  SegmentInfo    *active_next;
  
#ifdef DEBUG_MODE
  printf("rqstBufferDetach: rqsttab 0x%x  Start\n", (int)rqsttab);
#endif
  
  if (rqsttab == NULL)                            return RQT_E_DISABLED;
  if (rqsttab->AttachedBuf == NULL)               return RQT_E_OK;
  
  active = rqsttab->Active;
  
  /* 1. Finalize all pending buffered transfers (BSEND, IBSEND and PBSEND) */
  while (active) {
	active_next = active->Next;
	rqst = &active->SubRqst;
	/*
	 * At this time the implementation does not permit to cancell buffered requests
	 *
	 if(!RQST_isCancelled(rqst)) {
     if (0 > waitone(&rqst, (RQST_Status_t)NULL))  return RQT_E_INTEGRITY;
     rqstFreeSegment(rqsttab, (SegmentInfo_t)(rqst)->PackedBuffer);
	 }
	 */
	if (0 > waitone(&rqst, (RQST_Status_t)NULL))  return RQT_E_INTEGRITY;
	rqstFreeSegment(rqsttab, (SegmentInfo_t)(rqst->PackedBuffer));
	active = active_next;
  }
  
  /* 2. Wait for cancelled request to get back from receiver LFQ or MBX queues     *
   *    At this time the implementation do not permit to cancell buffered requests *
   *
   * while(rqsttab->Active)
   *   AZQ_progress(me);
   */
  
  /* 3. Data to return */
  *buffer = rqsttab->AttachedBuf;
  *size   = rqsttab->Length;
  
  /* 4. Empty fields in the request table */
  rqsttab->AttachedBuf = (char *)NULL;
  rqsttab->Length      = 0;
  rqsttab->Active      = NULL;
  rqsttab->Avail       = NULL;
  
  
#ifdef DEBUG_MODE
  printf("rqstBufferDetach: rqsttab 0x%x [BUF: 0x%x  SIZE: %d] End\n", (int)rqsttab, (int)*buffer,
		 (int)*size);
#endif
  
  return RQT_E_OK;
}


/*
 *  rqstAlloc
 *    Allocate a not buffered request
 */
int rqstAlloc(RqstTable *rqsttab, Rqst_t *rqst, Mpi_P_Comm *comm, int type) {
  
  Rqst_t   req;
  int      idx;
  /*
  if (0 > objAlloc(rqsttab->Rqst, &req, &idx)) {
    *rqst = AZQ_RQST_NULL;
    return(RQT_E_EXHAUST);
  }
  */
  objAlloc(rqsttab->Rqst, &req, &idx);
  //added by Herman, initialize.
  memset(req,0,sizeof(struct  RQST_Obj));
  req->Index   = idx;  
  req->Comm    = comm;
  req->MpiType = type;
 // req->Pool    = rqsttab;
  assert(rqst!=NULL);
  *rqst = req;

#ifdef DEBUG_MODE
  fprintf(stdout, "rqstAlloc: [rqst 0x%x, idx %d] \n", (int)*rqst, index);
#endif
  //fprintf(stdout, "rqstAlloc(%p): Allocated rqst %p of Pool %p\n", self(), *rqst, req->Pool);

  return(RQT_E_OK);
}



/*
 *  rqstFree
 */
int rqstFree(RqstTable *rqsttab, Mpi_P_Request_t *rqst) {

  int  size;
  int  pos;

#ifdef DEBUG_MODE
  printf("rqstFree: rqst 0x%x\n", (int)*rqst);
#endif

  if (*rqst == RQST_NULL)                        return RQT_E_OK;

  /* 1. Free buffers if necessary */
  if ((*rqst)->PackedBuffer != NULL) {    
    if (RQST_isBuffered(*rqst)) {
      rqstFreeSegment(rqsttab, (SegmentInfo_t)(*rqst)->PackedBuffer);
    } else { /* Send non-blocking or receive non-blocking or persistent: packed buffer from a non-contig datatype */
      FREE((*rqst)->PackedBuffer);
    }
  }
  
  /* 2. Free the request */
  (*rqst)->MpiType      = EMPTY_RQST_TYPE;
  (*rqst)->PackedBuffer = NULL;
  (*rqst)->OrigBuffer   = NULL;
  (*rqst)->Datatype     = NULL;
  (*rqst)->Count        = 0;
  objFree(rqsttab->Rqst, rqst, (*rqst)->Index);
  
#ifdef DEBUG_MODE
  printf("rqstFree: End\n");
#endif

  return (RQT_E_OK);
}


  /*-------------------------------------------------------/
 /      Managing user attached buffer interface           /
/-------------------------------------------------------*/
/*
 *  rqstFreeSegment
 *    Free a segment from the active list. Try to merge it with left and rigth
 *    avail segments if any.
 */
static int rqstFreeSegment (RqstTable *rqsttab, SegmentInfo *segment) 
{
  SegmentInfo  *prev  = segment->Prev;
  SegmentInfo  *avail = rqsttab->Avail;
  SegmentInfo  *avail_prev;

#ifdef DEBUG_MODE
  printf("rqstFreeSegment (0x%x, segment 0x%x): Start\n", (int)rqsttab, (int)segment);
#endif

  /* 1. Free segment */
  if (prev)
    prev->Next = segment->Next;
  else
    rqsttab->Active = segment->Next;

  if (segment->Next) { 
    segment->Next->Prev = prev;
  }

  /* 2. Find avail (not used) chunks such as "segment" is between them (or null if end of list) */
  avail_prev = NULL;
  while (avail) {
    if (avail > segment)    break;
    avail_prev  = avail;
    avail       = avail->Next;
  }

  /* 3. Try to merge with next avail segment */
  if (avail) {

    if ((char *)segment + segment->Size == (char *)avail) {
#ifdef DEBUG_MODE
      printf("rqstFreeSegment (segment 0x%x merged with NEXT 0x%x)\n", (int)segment, (int)avail);
#endif
      segment->Size += avail->Size;
      segment->Next  = avail->Next;
      if (avail->Next)
        avail->Next->Prev = segment;
      avail = NULL;
    } else {
      segment->Next = avail;
      avail->Prev   = segment;
    }

  } else
    segment->Next = NULL;

  /* 4. Try to merge with previous avail segment */
  if (avail_prev) {

    if ((char *)avail_prev + avail_prev->Size == (char *)segment) {
#ifdef DEBUG_MODE
      printf("rqstFreeSegment (segment 0x%x merged with PREV 0x%x)\n", (int)segment, (int)avail_prev);
#endif
      avail_prev->Size += segment->Size;
      avail_prev->Next  = segment->Next;
      if (segment->Next)
        segment->Next->Prev = avail_prev;
    } else {
      avail_prev->Next = segment;
      segment->Prev    = avail_prev;
    }

  } else {
    rqsttab->Avail = segment;
    segment->Prev  = NULL;
  }

#ifdef DEBUG_MODE
  printf("rqstFreeSegment (0x%x, segment 0x%x): End.\n", (int)rqsttab, (int)segment);
#endif

  return RQT_E_OK;
}



/*
 *  rqstGetSegment
 *    Get a segment from the avail list. If neccessary divide the segment (avoiding too
 *    internal fragmentation throght MIN_SEGMENT_SIZE) and leave the free space in the avail list.
 */
int rqstGetSegment(RqstTable *rqsttab, SegmentInfo_t *segment, int size) {

           SegmentInfo  *segment_prev;
           SegmentInfo  *new_avail    = NULL;
  register SegmentInfo  *seg_aux;

#ifdef DEBUG_MODE
  printf("rqstGetSegment (0x%x, size %d): Start\n", (int)rqsttab, size);
#endif

  /* 0. Complete BSend pending requests, if any */
  RQST_CompleteBsendRqsts (rqsttab);

  seg_aux = rqsttab->Avail;
  /* 1. Find a free segment */
  while (seg_aux) {
    if ((seg_aux->Size - INFO_SEGMENT_SIZE) >= size) break;
    seg_aux = seg_aux->Next;
  }
  if (seg_aux == NULL) {
    *segment = NULL;
    return RQT_E_BUFFER;
  }

  /* 2. Enough space. The segment is divided if the rest free space is enough for
        another active segment. The MIN_SEGMENT_SIZE avoid too much fragmentation */
  if (seg_aux->Size >= (INFO_SEGMENT_SIZE + size + MIN_SEGMENT_SIZE)) {
    new_avail = (SegmentInfo *)((char *)seg_aux + INFO_SEGMENT_SIZE + size);
    new_avail->Size = seg_aux->Size - size - INFO_SEGMENT_SIZE;
    new_avail->Next = seg_aux->Next;
    new_avail->Prev = seg_aux;
    if (seg_aux->Next)
      seg_aux->Next->Prev = new_avail;
    seg_aux->Next = new_avail;
    seg_aux->Size = (char *)new_avail - (char *)seg_aux;
  }

  /* 2. Remove segment from avail list and insert it into active list */
  segment_prev = seg_aux->Prev;
  if (segment_prev)
    segment_prev->Next = seg_aux->Next;
  else
    rqsttab->Avail = seg_aux->Next;

  if (seg_aux->Next)
    seg_aux->Next->Prev = seg_aux->Prev;

  if (rqsttab->Active)
    rqsttab->Active->Prev = seg_aux;

  seg_aux->Next   = rqsttab->Active;
  seg_aux->Prev   = NULL;
  rqsttab->Active = seg_aux;

  *segment = seg_aux;

#ifdef DEBUG_MODE
  printf("rqstGetSegment (0x%x, segment 0x%x  free 0x%x): End\n", (int)rqsttab, (int)*segment, (int)new_avail);
#endif

  return RQST_E_OK;
}



/*
 *  rqstGetByAddr:
 *   Return Index of a request in the "object" structure by address. This index
 *   is stored in a field of the object (request), so return it
 */
int rqstGetByAddr(RqstTable *rqsttab, Mpi_P_Request *rqst) {
  return rqst->Index;
}


/*
 *  rqstGetByIndex:
 *   Return the reference to an object (request) by its index in the
 *   "object" structure.
 */
Mpi_P_Request *rqstGetByIndex(RqstTable *rqsttab, int index) {
  
  Mpi_P_Request *req;
  
  objGet(rqsttab->Rqst, index, &req);
  
  return req;
}


  /*-------------------------------------------------------/
 /    Implementation of private DEBUG interface           /
/-------------------------------------------------------*/

#ifdef DEBUG_MODE

PRIVATE int printRequests (RqstTable *rqsttab) {

  int i;

  fprintf(stdout, " >>>>>> PENDING REQUEST ENTRIES (TASK %d) <<<<<<  (Pending Request 0x%x)\n", getRank(), rqsttab->Rqst);
  /* TODO */
  /*
   objPrint(object);
   */
  
  return 0;
}

#endif
