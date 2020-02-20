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

#ifndef P_COMN_H
#define P_COMN_H

  /*----------------------------------------------------------------/
 /   Declaration of types and functions used by this module        /
/----------------------------------------------------------------*/
#include <config.h>
#include <arch.h>

#if defined(__OSI)
  #include <osi.h>
#else
  #include <stdio.h>
  #include <pthread.h>
#endif

#include <pthread.h>
#include <p_group.h>

  /*-------------------------------------------------------/
 /                   Public constants                     /
/-------------------------------------------------------*/
#define COM_E_OK                 0
#define COM_E_EXHAUST           (COM_E_OK          - 1)
#define COM_E_INTEGRITY         (COM_E_EXHAUST     - 1)
#define COM_E_TIMEOUT           (COM_E_INTEGRITY   - 1)
#define COM_E_INTERFACE         (COM_E_TIMEOUT     - 1)
#define COM_E_SYSTEM            (COM_E_INTERFACE   - 1)
#define COM_E_DISABLED          (COM_E_SYSTEM      - 1)

#define ATR_E_OK                 0
#define ATR_E_NOT_ALLOC         (ATR_E_OK          - 1)

#define TOPO_E_OK                0
#define TOPO_E_UNDEFINED        (TOPO_E_OK         - 1)
#define TOPO_E_ALLOCATED        (TOPO_E_UNDEFINED  - 1)
#define TOPO_E_EXHAUST          (TOPO_E_ALLOCATED  - 1)

/* Degree (average) of the nodes in a graph topology. For allocating static memory */
#define DEGREE                   2
#define MAX_CART_DIMS           (4)

/* Index for predefined communicators */
#define COMM_WORLD               0
#define COMM_SELF                1
#define COMM_WORLD_LEADERS       2
#define COmM_WORLD_LOCALS        3

/* Type of a communicator */
#define NOT_ALLOCATED             0x0000
#define INTRACOMM                 0x0001
#define INTERCOMM                 0x0002

/* Blocks for communication creation */
#define PREALLOC_COMMS_COUNT           8
#define BLOCK_COMMS_COUNT             32
#define PREALLOC_MAX_COMMS_BLOCKS     64

/* By now, attributes is a static arrays with keyval indexes */
#define  DEFAULT_ATTRIBUTES_COUNT     16

/* Offset for assigning a new communicator context */
#define COMM_CONTEXT_OFFSET            3

  /*-------------------------------------------------------/
 /                    Public types                        /
/-------------------------------------------------------*/

/* Graph topology */
struct Mpi_Graph_Topology {
  int   Nindex;
  int   Index    [128];
  int   Nedges;
  int   Edges    [128 * DEGREE];
};
typedef struct Mpi_Graph_Topology  Mpi_Graph_Topology, *Mpi_Graph_Topology_t;

/* Cartesian topology */
struct Mpi_Cart_Topology {
  int   Ndims;
  int   Dims     [MAX_CART_DIMS];
  int   Periods  [MAX_CART_DIMS];
  int   Position [MAX_CART_DIMS];
};
typedef struct Mpi_Cart_Topology   Mpi_Cart_Topology,  *Mpi_Cart_Topology_t;

/* topologies */
struct Mpi_P_Topology {
  int Type;
  union Topology {
    Mpi_Cart_Topology   Cart;
	Mpi_Graph_Topology  Graph;
  } Topology;
};
typedef struct Mpi_P_Topology      Mpi_P_Topology,     *Mpi_P_Topology_t;

/* Communicator attributes */
struct Mpi_Attribute {
  int                Allocated;
  void              *Value;
};
typedef struct Mpi_Attribute       Mpi_P_Attribute,    *Mpi_P_Attribute_t;


struct SenseS {
  int  Sense;
  char Padding[CACHE_LINE_SIZE - sizeof(int)];
};
typedef struct SenseS SenseS, *SenseS_t;


/* Barrier fields for SMP */
struct barrier_t {
  volatile int  Counter __attribute__(( aligned(CACHE_LINE_SIZE) )) ;
  volatile int  Flag;   
  SenseS        Sense[128];
};

/*
struct cerrojo {
  volatile int Cerrojo / *__attribute__(( aligned(CACHE_LINE_SIZE) ))* / ;
  //char         Pad[CACHE_LINE_SIZE - sizeof(int)];
};
typedef struct cerrojo cerrojo, *cerrojo_t;
*/

/* Communicator */
struct Mpi_Comm {
  struct barrier_t   Barrier;       /* SMP barrier struct */ 
  cerrojo            ReduceLock[128];
  volatile int       ReduceRootWait;
  volatile int       ReduceRootWaitCnt;
  volatile void     *ReduceRootRecvBuf;
  volatile int       ReduceTurn;
  int                ReduceMyTurn;
  int                Type;
  Mpi_P_Group       *LocalGroup;    /* Intracommunicators and local group on intercommunicators */
  Mpi_P_Group       *RemoteGroup;   /* Remote group for intercommunicators. No sense in intracommunicators */
  int                Context;       /* Comunicator context */
  int                CommUp;        /* Upper (father) communicator, from which this communicator was created */
  void              *ErrorHandler;  /* Error handler associated to this communicator */
  
  Mpi_P_Attribute   *Attributes;    /* Communicator attribute table */
  
  Mpi_P_Topology    *Topology;      /* Topology associated to this communicator */
  
  /* Multicore clusters improvement for collectives */
  int                MCC_Support;   /* True if active in this communicator the improvement of collectives */
  struct Mpi_Comm   *Comm_Leaders;  /* Communicator of leaders in each node */
  struct Mpi_Comm   *Comm_Locals;   /* Communicator of local ranks to a node */
  int               *Node_Leaders;  /* Leaders of each node (relative rank in Comm_Leaders) */
  
  /* SMP improvements for collectives */
  int                SMP_Support;   /* True if active in this communicator the improvements for SMP */
  
  struct Mpi_Comm   *comm_root;
  
  int                Index;         /* Integer (now index in the object structure) for supporting
									  Fortran to/from C interface */
};
typedef struct Mpi_Comm            Mpi_P_Comm,         *Mpi_P_Comm_t;

struct CommTable {
  Mpi_P_Comm   *Comms;
  int           CommNrMax;
  Mpi_P_Comm   *World;
  Mpi_P_Comm   *Self;
};
typedef struct CommTable           CommTable,          *CommTable_t;


  /*-------------------------------------------------------/
 /           Declaration of public functions              /
/-------------------------------------------------------*/
/* Communicators table interface */
extern int          commAllocTable   (CommTable_t *commtable);
extern int          commFreeTable    (CommTable_t *commtable);
#define             commGetNrMax(commtable)  (commtable)->CommNrMax

/* Communicators interface */
extern int          commCreate       (CommTable *commtable, Mpi_P_Comm *comm,
                                                            Mpi_P_Group *local_group, Mpi_P_Group *remote_group,
                                                            int local_rank, int commnr, int type,
                                                            void *errhnd, Mpi_P_Comm_t *newcomm);
extern int          commDelete       (CommTable *commtable, Mpi_P_Comm_t *comm);


extern Mpi_P_Group   *commGetGroup          (Mpi_P_Comm *comm);

/* PROTOTYPE: extern Mpi_P_Group   *commGetLocalGroup     (Mpi_P_Comm *comm) */
#define commGetLocalGroup(comm)                                                \
  (comm)->LocalGroup
  
extern Mpi_P_Group   *commGetLocalGroupRef  (Mpi_P_Comm *comm);
/* PROTOTYPE: extern int          commGetContext        (Mpi_P_Comm *comm)  */
#define commGetContext(comm)                                                   \
  (comm)->Context
  
/* PROTOTYPE: extern int          commGetRank           (Mpi_P_Comm *comm) */
#define commGetRank(comm)                                                      \
  groupGetLocalRank((comm)->LocalGroup)


extern int          commGetGlobalRank     (Mpi_P_Comm *comm, int localrank);

/* PROTOTYPE: extern int          commGetSize           (Mpi_P_Comm *comm) */
#define commGetSize(comm)                                                      \
  groupGetSize((comm)->LocalGroup)
  
  
/* PROTOTYPE: extern int          commGetType           (Mpi_P_Comm *comm) */
#define commGetType(comm)                                                      \
  (comm)->Type
  
/* PROTOTYPE: extern int          commGetRemoteSize     (Mpi_P_Comm *comm) */
#define commGetRemoteSize(comm)                                                \
  groupGetSize((comm)->RemoteGroup)
  
/* PROTOTYPE: extern Mpi_P_Group   *commGetRemoteGroup    (Mpi_P_Comm *comm) */
#define commGetRemoteGroup(comm)                                               \
  (comm)->RemoteGroup

extern Mpi_P_Group   *commGetRemoteGroupRef (Mpi_P_Comm *comm);

extern int            commGetByAddr         (CommTable *commtab, Mpi_P_Comm *comm);
extern Mpi_P_Comm    *commGetByIndex        (CommTable *commtab, int index);

/* MCC (Multicore Collectives) improvement support */
#define commMCCSupport(comm)      (comm)->MCC_Support
#define commMCCGetLeaders(comm)   (comm)->Comm_Leaders
#define commMCCGetLocals(comm)    (comm)->Comm_Locals

/* SMP improvement support */
#define commSMPSupport(comm)      (comm)->SMP_Support


/* Communicator error handler interface */
extern int          commSetErrHnd    (Mpi_P_Comm *comm, void  *errhnd);
extern int          commGetErrHnd    (Mpi_P_Comm *comm, void **errhnd);
extern int          commHandleError  (Mpi_P_Comm *comm, int errcode, char *where);

/* Attributes interface */
extern int          commPutAttr      (Mpi_P_Comm *comm, int keyval, void *value);
extern int          commGetAttr      (Mpi_P_Comm *comm, int keyval, void **value);
extern int          commDelAttr      (Mpi_P_Comm *comm, int keyval);

/* Topology interface */
extern int          commGraphCreate  (Mpi_P_Comm *comm, int rank, int nnodes, int *index, int *edges);
extern int          commGraphGetDim  (Mpi_P_Comm *comm, int *nnodes, int *nedges);
extern int          commGraphGet     (Mpi_P_Comm *comm, int maxindex, int maxedges, int *index, int *edges);
extern int          commGraphNborsCnt(Mpi_P_Comm *comm, int rank, int *nneighbors);
extern int          commGraphNbors   (Mpi_P_Comm *comm, int rank, int maxneighbors, int *neighbors);
extern int          commGraphMap     (Mpi_P_Comm *comm, int rank, int nnodes, int *index, int *edges, int *newrank);

extern int          commTopoType     (Mpi_P_Comm *comm);

extern int          commCartCreate   (Mpi_P_Comm *comm, int nnodes, int rank, int ndims, int *dims, int *periods);
extern int          commCartGetCoords(Mpi_P_Comm *comm, int rank, int maxdims, int *coords);
extern int          commCartGetRank  (Mpi_P_Comm *comm, int *coords, int *rank);
extern int          commCartGetDim   (Mpi_P_Comm *comm);
extern int          commCartGet      (Mpi_P_Comm *comm, int maxdims, int *dims, int *periods, int *coords);
extern int          commCartShift    (Mpi_P_Comm *comm, int direction, int disp, int rank, int *src, int *dst);
extern int          commCartMap      (Mpi_P_Comm *comm, int rank, int ndims, int *dims, int *periods, int *newrank);

extern int          commCopyTopol    (Mpi_P_Comm *comm1, Mpi_P_Comm *comm2);

#endif

