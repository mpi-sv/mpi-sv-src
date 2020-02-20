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

#ifndef P_RQST_H
#define P_RQST_H

  /*----------------------------------------------------------------/
 /   Declaration of types and functions used by this module        /
/----------------------------------------------------------------*/
#include <config.h>

#include <com.h>
#include <rqst.h>

#include <p_dtype.h>
#include <p_comn.h>

  /*-------------------------------------------------------/
 /                   Public constants                     /
/-------------------------------------------------------*/
#define RQT_E_OK                 0
#define RQT_E_EXHAUST           (RQT_E_OK          - 1)
#define RQT_E_INTEGRITY         (RQT_E_EXHAUST     - 1)
#define RQT_E_TIMEOUT           (RQT_E_INTEGRITY   - 1)
#define RQT_E_INTERFACE         (RQT_E_TIMEOUT     - 1)
#define RQT_E_SYSTEM            (RQT_E_INTERFACE   - 1)
#define RQT_E_DISABLED          (RQT_E_SYSTEM      - 1)
#define RQT_E_BUFFER            (RQT_E_DISABLED    - 1)

/* A null request */
#define  RQST_NULL   (Mpi_P_Request *)NULL

/* Request types */
#define  EMPTY_RQST_TYPE         0x0000
#define  BSEND_RQST_TYPE         0x0001
#define  IBSEND_RQST_TYPE        0x0002
#define  PBSEND_RQST_TYPE        0x0004

/* Utility macros */
#define RQST_isBuffered(rqst)    \
        ((rqst)->MpiType & (BSEND_RQST_TYPE  | IBSEND_RQST_TYPE | PBSEND_RQST_TYPE))

/* Number of requests */
#define  PREALLOC_REQUEST_COUNT       16
#define  BLOCK_REQUEST_COUNT         128
#define PREALLOC_MAX_RQSTS_BLOCKS   1024


  /*-------------------------------------------------------/
 /                    Public types                        /
/-------------------------------------------------------*/
/* An AzqMPI request is an Azequia request */
typedef Rqst                  Mpi_P_Request,  *Mpi_P_Request_t;
typedef struct RqstTable      RqstTable,      *RqstTable_t;

/* Info to be set before a data message in the user buffer */
#define SegmentInfoSize ( 2 * sizeof (void *) + \
                          1 * sizeof (int)           + \
                          1 * sizeof (Rqst)            \
                        )

struct SegmentInfo {
  struct SegmentInfo  *Next;
  struct SegmentInfo  *Prev;
  int                  Size;
  Rqst                 SubRqst;
  char                 Pad[(SegmentInfoSize % CACHE_LINE_SIZE ? CACHE_LINE_SIZE - (SegmentInfoSize % CACHE_LINE_SIZE) : 0)];
};
typedef struct SegmentInfo SegmentInfo, *SegmentInfo_t;
#define INFO_SEGMENT_SIZE (sizeof(SegmentInfo))

  /*-------------------------------------------------------/
 /           Declaration of public functions              /
/-------------------------------------------------------*/
/*
 *  rqstSet
 *    Set fields related to specific modes of communication
 */
#define rqstSet(rqsttab, rqst, packedbuf, origbuf, datatype, count)    \
  {                                                                    \
    (rqst)->PackedBuffer = packedbuf;                                  \
    (rqst)->OrigBuffer   = origbuf;                                    \
    (rqst)->Datatype     = (void *)datatype;                           \
    (rqst)->Count        = count;                                      \
  }

extern int  rqstAllocTable      (RqstTable_t *rqsttab);
extern int  rqstFreeTable       (RqstTable_t *rqsttab);

extern int  rqstBufferAttach    (RqstTable   *rqsttab, void  *buffer, int  size);
extern int  rqstBufferDetach    (RqstTable   *rqsttab, void **buffer, int *size);

extern int  rqstAlloc           (RqstTable   *rqsttab, Mpi_P_Request_t *rqst, Mpi_P_Comm *comm, int type);
extern int  rqstFree            (RqstTable   *rqsttab, Mpi_P_Request_t *rqst);

extern int  rqstGetSegment      (RqstTable   *rqsttab, SegmentInfo_t *segment, int size);
extern int  rqstfreeSegment     (RqstTable   *rqsttab, SegmentInfo   *segment);

extern int  rqstGetByAddr       (RqstTable *rqsttab, Mpi_P_Request *rqst);
extern Mpi_P_Request *
            rqstGetByIndex      (RqstTable *rqsttab, int index);


#endif

