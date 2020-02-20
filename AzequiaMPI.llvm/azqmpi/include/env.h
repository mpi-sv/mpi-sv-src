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

#ifndef	ENV_H
#define	ENV_H

  /*----------------------------------------------------------------/
 /   Declaration of types and functions used by this module        /
/----------------------------------------------------------------*/
#include <config.h>

#if defined (__OSI)
  #include <osi.h>
#else
  #include <pthread.h>
  #include <errno.h>
  #include <string.h>
#endif

#include <com.h>

#include <p_group.h>
#include <p_errhnd.h>
#include <p_comn.h>
#include <p_dtype.h>
#include <p_collops.h>
#include <p_key.h>
#include <p_rqst.h>

  /*-------------------------------------------------------/
 /                   Public constants                     /
/-------------------------------------------------------*/

/* Useful and clear kind of invoke some functions for handling erros.
   This macro requires in the calling function:
   - an integer named "mpi_errno"
   - a "mpi_exception_unnest" label
 */
#define CALL_FXN(fxn, code)     if (0 != (fxn)) {mpi_errno = code; goto mpi_exception_unnest;}

/* Useful and clear kind of invoke nested MPI functions and handling erros */
#ifdef CHECK_MODE
#define CALL_MPI_NEST(fxn)      if (MPI_SUCCESS != (mpi_errno = (fxn))) goto mpi_exception_unnest
#else
#define CALL_MPI_NEST(fxn)      (fxn)
#endif

/* Default communicators */
#define COMM_WORLD_PTR          ((Mpi_P_Comm *)PCS_self()->CommTable->World)
#define COMM_SELF_PTR           ((Mpi_P_Comm *)PCS_self()->CommTable->Self)

/* Default groups */
#define GROUP_WORLD_PTR         ((Mpi_P_Group *)PCS_self()->GroupTable->World)
#define GROUP_EMPTY_PTR         ((Mpi_P_Group *)PCS_self()->GroupTable->Empty)
#define GROUP_SELF_PTR          ((Mpi_P_Group *)PCS_self()->GroupTable->Self)

/* Default error handlers */
#define FATAL_ERROR_PTR         ((Mpi_P_Errhandler *)PCS_self()->ErrHndTable->Fatal)
#define RETURN_ERROR_PTR        ((Mpi_P_Errhandler *)PCS_self()->ErrHndTable->Return)

/* Max number of processes in a AzequiaMPI application */
#define MAX_MPI_PROCS            128

/* Null process for communication */
#define PROC_NULL                (-1)

/* Internal TAGs for send/receive collective messages */
#define	BARRIER_TAG   0x00008001
#define	REDUCE_TAG    0x00008002
#define	BCAST_TAG     0x00008003
#define SCATTER_TAG   0x00008004
#define ALLTOALL_TAG  0x00008005
#define GATHER_TAG    0x00008006

#define TAG_UB        0x00007FFF

/* Tag Azequia carries information about tag and communicator number in AzequiaMPI */
#define EXTENDED_TAG(commNr, tag)   ((((commNr) << 16) & 0xFFFF0000) | ((tag) & 0x0000FFFF))

#define TAG_EXT(tag)           ((tag) & 0x0000FFFF)
#define COMNR_EXT(tag)        (((tag) & 0xFFFF0000) >> 16)

/* Topologies */
#define TOPO_UNDEFINED          (-1)     /* Same as MPI_UNDEFINED */
#define TOPO_CART               (1)
#define TOPO_GRAPH              (2)


/* A MPI Process (endpoint) */
struct Process {
  int               Initialized;
  int               NestLevel;
  int               ThreadLevelSupported;
  CommTable        *CommTable;
  GroupTable       *GroupTable;
  CopsTable        *CopsTable;
  ErrHndTable      *ErrHndTable;
  DTypeTable       *DTypeTable;
  KeysTable        *KeysTable;
  RqstTable        *RqstTable;
  
  /* Padding */
  char              Pad [CACHE_LINE_SIZE - 
						 (( 3  *  sizeof(int)    +
						    7  *  sizeof(void *)  )
						   % CACHE_LINE_SIZE)];
};


/* MPI processes */
typedef struct ProcTable ProcTable, *ProcTable_t;
typedef struct Process Process, *Process_t;


pthread_key_t  key_self;
#define PCS_self()              ((Process *)pthread_getspecific(key_self))

/* Communicators utilities */
#define COMM_INDEX_MASK          0x000000FF

/* Errors */
#define PCS_E_OK                 0
#define PCS_E_EXHAUST           (PCS_E_OK          - 1)
#define PCS_E_INTEGRITY         (PCS_E_EXHAUST     - 1)
#define PCS_E_TIMEOUT           (PCS_E_INTEGRITY   - 1)
#define PCS_E_INTERFACE         (PCS_E_TIMEOUT     - 1)
#define PCS_E_SYSTEM            (PCS_E_INTERFACE   - 1)
#define PCS_E_DISABLED          (PCS_E_SYSTEM      - 1)
#define PCS_E_EXIST             (PCS_E_DISABLED    - 1)


#define  MALLOC(mem, size)                                             \
  (NULL == (mem = (void *) malloc (size)))

#define  FREE(mem)                                                     \
  (free(mem))


/* Timer. Depends on operating system */
#define USE_CLOCK_GETTIME 0
#define USE_GETTIMEOFDAY  1
#define USE_MACOSX_TIME   2

/* Exception Azequia-like printing */
#define XPN_print(excpn)                                                                     \
  {                                                                                          \
    if(excpn < 8) {                                                                          \
      fprintf(stdout, "\t\t\t>>> Exception %s.", e_names[-(excpn)]);                         \
      fprintf(stdout, "\t\t\t    Raised by %x in %s \n", (unsigned int)(THR_self()), where); \
    }                                                                                        \
  }

  /*-------------------------------------------------------/
 /           Declaration of public functions              /
/-------------------------------------------------------*/

/* Count nest level in thread calling functions. Error handlers are only 
   invoked in upper level */
#define NEST_FXN_INCR() \
  PCS_self()->NestLevel++

#define NEST_FXN_DECR() \
  PCS_self()->NestLevel--

/* Requests interface */
#define PCS_rqstAlloc(request, comm, type)                                     \
  rqstAlloc(PCS_self()->RqstTable, request, comm, type)

#define PCS_rqstFree(request)                                                  \
  rqstFree(PCS_self()->RqstTable, request)

#define PCS_rqstSet(request, packedbuf, origbuf, datatype, count)              \
  rqstSet(PCS_self()->RqstTable, request, packedbuf, origbuf, datatype, count)

#define PCS_rqstGetSegment(segment, size)                                      \
  rqstGetSegment(PCS_self()->RqstTable, segment, size)

#define PCS_rqstFreeSegment(segment)                                           \
  rqstFreeSegment(PCS_self()->RqstTable, segment)

#define PCS_rqstBufferAttach(buffer, size)                                     \
  rqstBufferAttach(PCS_self()->RqstTable, buffer, size)

#define PCS_rqstBufferDetach(buffer, size)                                     \
  rqstBufferDetach(PCS_self()->RqstTable, buffer, size)

#define PCS_rqstGetByAddr(request)                                             \
  rqstGetByAddr(NULL, request)

#define PCS_rqstGetByIndex(index)                                              \
  rqstGetByIndex(PCS_self()->RqstTable, index)


/* Processes/Endpoints/Threads */
extern int               PCS_init              (int thread_level_support);
extern int               PCS_finalize          ();
extern int               PCS_isInit            ();

extern int               PCS_getEnv            (int *node, int *nodes_nr, int *node_group_nr);
extern int               PCS_getNode           (int global_rank, int *rank_node);

/* Groups */
 /* PROTOTYPE: int PCS_groupCreate (int *members, int size, Mpi_P_Group_t *newgroup) */
#define  PCS_groupCreate(members, size, newgroup)                              \
  groupCreate(PCS_self()->GroupTable, members, size, newgroup)
 /* PROTOTYPE: int PCS_groupDelete (Mpi_P_Group_t *group) */
#define  PCS_groupDelete(group)                                                \
  groupDelete(PCS_self()->GroupTable, group)

extern int               PCS_groupSet          (int grp_index, Mpi_P_Group *grp);


/* Communicators */
 /* PROTOTYPE: int PCS_commCreate (Mpi_P_Comm *comm, 
                                   Mpi_P_Group *local_group, Mpi_P_Group *remote_group,
                                   int key_attrs_nr, int commnr, int type,
                                   Mpi_P_Errhandler *errhnd, Mpi_P_Comm_t *newcomm)
  */
#define  PCS_commCreate(comm, local_group, remote_group, key_attrs_nr, commnr, \
                        type, errhnd, newcomm)                                 \
  commCreate(PCS_self()->CommTable, comm, local_group, remote_group,           \
                        key_attrs_nr, commnr, type, errhnd, newcomm)

#define PCS_commGetByAddr(comm)                                                \
  commGetByAddr(NULL, comm)

#define PCS_commGetByIndex(index)                                              \
  commGetByIndex(PCS_self()->CommTable, index)


extern int               PCS_commDelete        ();
extern int               PCS_commSet           (int comm_index, Mpi_P_Comm *comm);


 /* PROTOTYPE: int PCS_commGetNrMax () */
#define  PCS_commGetNrMax()                                                    \
  commGetNrMax(PCS_self()->CommTable)


/* Collectives */
 /* PROTOTYPE: int PCS_copCreate (Mpi_P_User_function *function, int commute, 
                                  Mpi_P_Op_t *collop) 
  */
#define  PCS_copCreate(function, commute, collop)                              \
  copsCreate(PCS_self()->CopsTable, function, commute, collop)

 /* PROTOTYPE: int PCS_copDelete (Mpi_P_Op_t *collop) */
#define  PCS_copDelete(collop)                                                 \
  copsDelete(PCS_self()->CopsTable, collop)

 /* PROTOTYPE: Mpi_P_Op *PCS_copGet (int cop_index) */
#define                  PCS_copGet(cop_index)                                 \
  (&BasicCopsTable[(cop_index)])


/* Error handlers */
 /* PROTOTYPE: int PCS_errhndCreate (Mpi_P_Handler_function *function, 
                                     Mpi_P_Errhandler_t *errhnd)
  */
#define  PCS_errhndCreate(function, errhnd)                                    \
  errhndCreate(PCS_self()->ErrHndTable, function, errhnd)

 /* PROTOTYPE: int PCS_errhndDelete (Mpi_P_Errhandler_t *errhnd) */
#define  PCS_errhndDelete(errhnd)                                              \
  errhndDelete(PCS_self()->ErrHndTable, errhnd)

extern int               PCS_errhndSet         (int errhnd_index, Mpi_P_Errhandler *errhnd);


/* Attributes */
 /* PROTOTYPE: int PCS_keyAlloc (int *keyval, void *extra_state, 
                                 void *copy_fxn, void *del_fxn) 
  */
#define  PCS_keyAlloc(keyval, extra_state, copy_fxn, del_fxn)                  \
  keyAlloc(PCS_self()->KeysTable, keyval, extra_state, copy_fxn, del_fxn)

 /* PROTOTYPE: int PCS_keyFree (int  keyval) */
#define  PCS_keyFree(keyval)                                                   \
  keyFree(PCS_self()->KeysTable, keyval)

 /* PROTOTYPE: int PCS_keyPutAttr (Mpi_P_Comm *comm, int keyval, 
                                   void *attribute_val) 
  */
#define  PCS_keyPutAttr(comm, keyval, attribute_val)                           \
  keyPutAttr(PCS_self()->KeysTable, comm, keyval, attribute_val)

 /* PROTOTYPE: int PCS_keyGetAttr (Mpi_P_Comm *comm, int keyval, 
                                   void **attribute_val, int *flag)
  */
#define  PCS_keyGetAttr(comm, keyval, attribute_val, flag)                     \
  keyGetAttr(PCS_self()->KeysTable, comm, keyval, attribute_val, flag)

 /* PROTOTYPE: int PCS_keyDelAttr (Mpi_P_Comm *comm, int keyval) */
#define  PCS_keyDelAttr(comm, keyval)                                          \
  keyDelAttr(PCS_self()->KeysTable, comm, keyval)
  
 /* PROTOTYPE: int PCS_keyDelAllAttr (Mpi_P_Comm *comm) */
#define  PCS_keyDelAllAttr(comm)                                               \
  keyDelAllAttr(PCS_self()->KeysTable, comm)

 /* PROTOTYPE: int PCS_keyCopyAllAttr (Mpi_P_Comm *comm, Mpi_P_Comm *newcomm) */
#define  PCS_keyCopyAllAttr(comm, newcomm)                                     \
  keyCopyAllAttr(PCS_self()->KeysTable, comm, newcomm)

 /* PROTOTYPE: int PCS_keyCopyDfltAttr (Mpi_P_Comm *comm, Mpi_P_Comm *newcomm) */
#define  PCS_keyCopyDfltAttr(comm, newcomm)                                    \
  keyCopyDfltAttr(PCS_self()->KeysTable, comm, newcomm)


/* Datatypes */
 /* PROTOTYPE: int PCS_dtypeCreate (int count, int *blklens, Mpi_P_Aint *displs,
                                    Mpi_P_Datatype **dtypes, int kind, 
                                    Mpi_P_Datatype_t *newtype)
  */
#define  PCS_dtypeCreate(count, blklens, displs, dtypes, kind, newtype)        \
  dtypeCreateStruct(PCS_self()->DTypeTable, count, blklens, displs, dtypes,    \
                    kind, newtype)
							
 /* PROTOTYPE: int PCS_dtypeFree (Mpi_P_Datatype_t *datatype) */
#define  PCS_dtypeFree(datatype)                                               \
  dtypeFree(PCS_self()->DTypeTable, datatype)
  
#define  PCS_dtypeGet(dtype_index)                                             \
  (&BasicDTypeTable[(dtype_index)])


#define PCS_dtypeGetByAddr(dtype)                                              \
  dtypeGetByAddr(NULL, dtype)
 
#define PCS_dtypeGetByIndex(index)                                             \
  dtypeGetByIndex(PCS_self()->DTypeTable, index)



#endif
