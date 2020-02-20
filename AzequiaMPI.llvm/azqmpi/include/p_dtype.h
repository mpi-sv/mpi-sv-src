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
   |  llorente@unex.es, 2011 Apr. : Fix for 64-bit archs                   |
   |_______________________________________________________________________| */

#ifndef P_DTYPE_H
#define P_DTYPE_H

  /*----------------------------------------------------------------/
 /   Declaration of types and functions used by this module        /
/----------------------------------------------------------------*/
#include <config.h>
#include <arch.h>

  /*-------------------------------------------------------/
 /                   Public constants                     /
/-------------------------------------------------------*/
#define DTP_E_OK                 0
#define DTP_E_EXHAUST           (DTP_E_OK          - 1)
#define DTP_E_INTEGRITY         (DTP_E_EXHAUST     - 1)
#define DTP_E_TIMEOUT           (DTP_E_INTEGRITY   - 1)
#define DTP_E_INTERFACE         (DTP_E_TIMEOUT     - 1)
#define DTP_E_SYSTEM            (DTP_E_INTERFACE   - 1)
#define DTP_E_DISABLED          (DTP_E_SYSTEM      - 1)


/* Basic datatypes */
#define	MPI_P_CHAR                 0
#define	MPI_P_SHORT                1
#define MPI_P_INT                  2
#define	MPI_P_LONG                 3
#define MPI_P_LONG_LONG_INT        4
#define MPI_P_LONG_LONG            5
#define	MPI_P_UNSIGNED_CHAR        6
#define	MPI_P_UNSIGNED_SHORT       7
#define	MPI_P_UNSIGNED             8
#define	MPI_P_UNSIGNED_LONG        9
#define MPI_P_UNSIGNED_LONG_LONG  10
#define	MPI_P_FLOAT               11
#define	MPI_P_DOUBLE              12
#define	MPI_P_LONG_DOUBLE         13

/* MAXLOC and MINLOC datatypes structs */
#define MPI_P_2INT                14
#define MPI_P_SHORT_INT           15
#define MPI_P_LONG_INT            16
#define MPI_P_FLOAT_INT           17
#define MPI_P_DOUBLE_INT          18
#define MPI_P_LONG_DOUBLE_INT     19

/* Fortran datatypes */
#define MPI_P_COMPLEX             20
#define MPI_P_DOUBLE_COMPLEX      21

#define	MPI_P_BYTE                22
#define	MPI_P_PACKED              23
#define MPI_P_LB                  24
#define MPI_P_UB                  25

#define LAST_DATATYPE             MPI_P_UB


/* Number of basic datatypes. Enough for the list, excluding MPI_P_LB and MPI_P_UB */
#define BASIC_DATATYPES          (MPI_P_PACKED + 1)

/* Max number of signatures allowed for a derived datatype */

/* State is a bit-field of some useful values */
#define  DTYPE_ALLOCATED    0x00000001
#define  DTYPE_COMMITED     0x00000002
#define  DTYPE_BASIC        0x00000004
#define  DTYPE_MEM_CONTIG   0x00000008

#define DTYPE_CONTIGUOUS    0x00000010
#define DTYPE_VECTOR        0x00000020
#define DTYPE_HVECTOR       0x00000040
#define DTYPE_INDEXED       0x00000080
#define DTYPE_HINDEXED      0x00000100
#define DTYPE_STRUCT        0x00000200

/* Blocks for datatype creation. 
   TRICK: Do not need any prealloc block because all basic datatypes are in a global
          table, but, for Index (Fortran to/from C) works properly, it needs an
          initial block simulating all basic datatypes */
#define PREALLOC_DTYPE_COUNT           1
#define BLOCK_DTYPE_COUNT             32
#define PREALLOC_MAX_DTYPE_BLOCKS    512

  /*-------------------------------------------------------/
 /                    Public types                        /
/-------------------------------------------------------*/
struct mpi_complex {
  float  r;
  float  i;
};

struct mpi_double_complex {
  double  r;
  double  i;
};

/* Mpi_P_Aint is a type capable of storing any address */
#if (AZQMPI_ARCH == IA32)
typedef int          Mpi_P_Aint;
#elif ((AZQMPI_ARCH == IA64) || (AZQMPI_ARCH == AMD64))
typedef long long    Mpi_P_Aint;
#else
#error "CHECK THIS TYPE FOR ARCHITECTURE INTEGER OF POINTER SIZE"
#endif


typedef struct Mpi_P_Datatype Mpi_P_Datatype, *Mpi_P_Datatype_t;
struct Mpi_P_Datatype {
  int               State;        /* ALLOCATED, COMMITED, CONTIGUOUS, class, etc. */
  
  Mpi_P_Datatype_t *Types;        /* Basic types signatures */
  Mpi_P_Aint       *Displs;       /* Displacements */
  int              *BlkLen;       /* Quantity of each item */
  int               Count;        /* Size of the arrays */
  
  int               BasicSign;
  Mpi_P_Aint        BasicDispl;
  
  int               Size;         /* In bytes */
  int               Elements;     /* Number of primitive datatypes */
  int               Blocks;       /* Number of datatypes (sum of blklens) */
  int               PackSize;     /* Size of this datatype packed */
  
  int               Extent;
  Mpi_P_Aint        Lb;           /* Lower bound */
  Mpi_P_Aint        Ub;           /* Upper bound */
  
  int               Refs;         /* Refereces from other types */
  
  int               Index;        /* Integer (now index in the object structure) for supporting
								     Fortran to/from C interface */
};


typedef struct DTypeTable DTypeTable, *DTypeTable_t;


  /*-------------------------------------------------------/
 /                    Public data                         /
/-------------------------------------------------------*/
extern Mpi_P_Datatype BasicDTypeTable [BASIC_DATATYPES];

  /*-------------------------------------------------------/
 /           Declaration of public functions              /
/-------------------------------------------------------*/
/* Per process datatype table with default MPI datatypes */
extern int             dtypeCreateDefault       ();
extern int             dtypeDeleteDefault       ();


/* Datatype table interface */
extern int             dtypeAllocTable          (DTypeTable_t *dtypetable);
extern int             dtypeFreeTable           (DTypeTable_t *dtypetable);

/* Datatype interface */
extern int             dtypeCreate              (DTypeTable *dtypetable, int count,
                                                 int *blklen, Mpi_P_Aint *displs, Mpi_P_Datatype **dtypes,
                                                 int kind, Mpi_P_Datatype_t *datatypenew);

extern int             dtypeFree                (DTypeTable *dtypetable, Mpi_P_Datatype_t *datatype);

extern int             dtypeCommit              (Mpi_P_Datatype *datatype);


#define dtypeIsAllocated(datatype)                                             \
  ((datatype)->State & DTYPE_ALLOCATED)

#define dtypeIsBasic(datatype)                                                \
  ((datatype)->State & DTYPE_BASIC)

#define dtypeIsContiguous(datatype)                                            \
  ((datatype)->State & DTYPE_MEM_CONTIG)

#define dtypeGetSize(datatype)                                                 \
  (datatype)->Size

#define dtypeGetElements(datatype)                                             \
  (datatype)->Elements

#define dtypeGetBlocks(datatype)                                               \
  (datatype)->Blocks

#define dtypeGetCount(datatype)                                                \
  (datatype)->Count

#define dtypeGetExtent(datatype)                                               \
  ((Mpi_P_Aint)(datatype)->Extent)

#define dtypeGetLb(datatype)                                                   \
  ((Mpi_P_Aint)(datatype)->Lb)

#define dtypeGetUb(datatype)                                                   \
  ((Mpi_P_Aint)(datatype)->Ub)

#define dtypeIsCommited(datatype)                                              \
  ((datatype)->State & DTYPE_COMMITED)

#define dtypeGetBasicSignature(datatype)                                       \
  (datatype)->BasicSign

/* Pack interface */
extern int             pack                     (void *inbuf,  int incount,  Mpi_P_Datatype *datatype,
                                                 void *outbuf, int outsize,  int *position);
extern int             unpack                   (void *inbuf,  int insize,   int *position,
                                                 void *outbuf, int outcount, Mpi_P_Datatype *datatype);


extern int             dtypeGetByAddr           (DTypeTable *dtypetab, Mpi_P_Datatype *dtype);
extern Mpi_P_Datatype *dtypeGetByIndex          (DTypeTable *dtypetab, int index);


#define packSize(incount, datatype)                                            \
  (datatype)->PackSize * (incount)


#endif

