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

  /*----------------------------------------------------------------/
 /   Declaration of public functions implemented by this module    /
/----------------------------------------------------------------*/
#include <p_comn.h>

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

#include <mpi.h>
#include <env.h>
#include <p_errhnd.h>
#include <p_group.h>
#include <p_collops.h>
#include <p_config.h>

#include <assert.h>
  /*-------------------------------------------------------/
 /         Declaration of private interface               /
/-------------------------------------------------------*/
#ifdef DEBUG_MODE
PRIVATE int printComm (CommTable *commtable);
#endif


  /*-------------------------------------------------------/
 /                 Public interface                       /
/-------------------------------------------------------*/
/*
 *  commAllocTable():
 *   Allocate a communicator table with a number of communicators, the number
 *   of attributes in an internal attributes table, and if the communicator has topology info
 */
int commAllocTable (CommTable_t *commtable) {

  CommTable   *commtab;
  int          error;

  /* 1. Allocate communicator table memory */
  if (posix_memalign((void *)&commtab, CACHE_LINE_SIZE, sizeof(CommTable)))     goto exception1;
  
  /* 2. Allocate communicators */
  objInit (&commtab->Comms, PREALLOC_MAX_COMMS_BLOCKS, 
                            sizeof(Mpi_P_Comm), 
                            PREALLOC_COMMS_COUNT, 
                            BLOCK_COMMS_COUNT, &error); 
  if(error)                                                                              goto exception2;
	
  commtab->World = commtab->Self = (Mpi_P_Comm *) NULL;

  /* 2. Table of communicators to return */
  *commtable = commtab;


  return ((PREALLOC_COMMS_COUNT * sizeof(Mpi_P_Comm)) + sizeof(struct CommTable));

exception2:
  free(commtab);
exception1:
  return COM_E_EXHAUST;
}


/*
 *  commFreeTable():
 *   Free the communicator table
 */
int commFreeTable (CommTable_t *commtable) {

  int i;


  objFinalize(&(*commtable)->Comms);
  
  free(*commtable);

  *commtable = (CommTable *) NULL;

  return COM_E_OK;
}

  /*-------------------------------------------------------------------------/
 /    Implementation of exported interface for communicators management     /
/-------------------------------------------------------------------------*/
/*
 *  commCreate():
 *   Create a new communicator
 */
int commCreate (CommTable *commtable, Mpi_P_Comm *intracomm,
                                      Mpi_P_Group *local_group, Mpi_P_Group *remote_group,
                                      int key_attrs_nr, int context, int type,
                                      void *errhnd, Mpi_P_Comm_t *newcomm) {

  Mpi_P_Comm  *comm2 = (Mpi_P_Comm *)NULL;
  int          idx;

  /* 1. New comm */
  /*
  if (0 > objAlloc(commtable->Comms, &comm2, &idx)) {
	*newcomm = (Mpi_P_Comm *)NULL;
    return(COM_E_EXHAUST);
  }
  */
  objAlloc((Object_t)commtable->Comms, &comm2, &idx);
  /* 2. Key attributes */
  assert(comm2!=NULL);
  if (key_attrs_nr > 0) {
	
    if (posix_memalign((void *)&comm2->Attributes, CACHE_LINE_SIZE, 
  					  sizeof(struct Mpi_Attribute) * key_attrs_nr))
     return COM_E_EXHAUST;
    memset((void *)comm2->Attributes, 0, sizeof(struct Mpi_Attribute) * key_attrs_nr);
  
  } else {
  
	comm2->Attributes = (Mpi_P_Attribute *)NULL;
  
  }
  /* 3. Topology initialization */
  comm2->Topology = (Mpi_P_Topology *)NULL;

  /* 4. Create a new communicator */
  if (intracomm == (Mpi_P_Comm *) MPI_COMM_NULL) { /* MPI_COMM_WORLD and MPI_COMM_SELF */
    comm2->CommUp        = -1;
    comm2->ErrorHandler  = errhndGetRef(errhnd);
  } else {
    comm2->CommUp        = intracomm->CommUp;
    comm2->ErrorHandler  = errhndGetRef(intracomm->ErrorHandler);
  }

  comm2->Index         = idx;
  
  comm2->Type          = type;
  comm2->LocalGroup    = local_group;
  comm2->RemoteGroup   = remote_group;
  comm2->Context       = context;

  commtable->CommNrMax = context;
  
  *newcomm = comm2;

#ifdef DEBUG_MODE
  printComm(commtable);
#endif

  return COM_E_OK;
}


/*
 *  commDelete():
 *   Delete a communicator
 */
int commDelete (CommTable *commtable, Mpi_P_Comm_t *comm) {
  
  /* 1. Free topology if any */
  if ((*comm)->Topology)
	free((*comm)->Topology);
  
  /* 2. Free attributes if any */
  if ((*comm)->Attributes)
	free((*comm)->Attributes);
	
  /* 3. Free comm */
  objFree((Object_t)commtable->Comms, comm, (*comm)->Index);

  *comm = (Mpi_P_Comm *)NULL;

  return COM_E_OK;
}


/*
 *  commGetGroup():
 *   Return the group associated to the communicator. It is a wildcard function used in pt2pt
 *   communication. If the communicator is an intracommunicator, returns the local group,
 *   if it is a intercommunication, returns the local group.
 */
Mpi_P_Group *commGetGroup (Mpi_P_Comm *comm) {

  if (comm->Type == INTRACOMM)
    return comm->LocalGroup;

  return comm->RemoteGroup;
}


/*
 *  commGetGlobalRank():
 *   Return the global (Azequia) rank of a local (AzqMPI) rank in the inter or intra communicator group.
 */
int commGetGlobalRank (Mpi_P_Comm *comm, int localrank) {

  if (comm->Type == INTRACOMM)
    return groupGetGlobalRank (comm->LocalGroup, localrank);

  return groupGetGlobalRank (comm->RemoteGroup, localrank);
}


/*
 *  commGetLocalGroupRef():
 *   Return a new reference to the local group associated to the communicator
 */
Mpi_P_Group *commGetLocalGroupRef (Mpi_P_Comm *comm) {

  Mpi_P_Group *newgroup;

  groupGetRef(comm->LocalGroup, &newgroup);

  return newgroup;
}


/*
 *  commGetRemoteGroupRef():
 *   Return a new reference to the remote group associated to the intercommunicator
 */
Mpi_P_Group *commGetRemoteGroupRef (Mpi_P_Comm *comm) {

  Mpi_P_Group *newgroup;

  groupGetRef(comm->RemoteGroup, &newgroup);

  return newgroup;
}


/*
 *  commGetByAddr:
 *   Return Index of a comm in the "object" structure by address. This index
 *   is stored in a field of the object (communicator), so return it
 */
int commGetByAddr(CommTable *commtab, Mpi_P_Comm *comm) {
  return comm->Index;
}


/*
 *  commGetByIndex:
 *   Return the reference to an object (communicator) by its index in the
 *   "object" structure.
 */
Mpi_P_Comm *commGetByIndex(CommTable *commtab, int index) {
  
  Mpi_P_Comm *comm;
  
  if (index == COMM_WORLD)
    return commtab->World;
  if (index == COMM_SELF)
    return commtab->Self;
  
  objGet((Object_t)commtab->Comms, index, &comm);
  
  return comm;
}


  /*-------------------------------------------------------/
 /   Implementation of public ERROR HANDLING interface    /
/-------------------------------------------------------*/

/*
 *  commHandleError
 *   Invoke the error handle for a coommunicator
 */
int commHandleError (Mpi_P_Comm *comm, int errcode, char *where) {

  Mpi_P_Comm   *comm_hnd;

  if (PCS_self()->NestLevel == 0) {

    comm_hnd = comm;

    /* If not communicator associated, get the MPI_COMM_WORLD error handler */
    if (comm_hnd == (Mpi_P_Comm *)MPI_COMM_NULL)
      comm_hnd = MPI_COMM_WORLD;

    errhndExec(comm_hnd->ErrorHandler, &comm_hnd, &errcode, where);

  }

  return errcode;
}


/*
 *  commSetErrHnd():
 *   Set an error handler for a communicator
 */
int commSetErrHnd (Mpi_P_Comm *comm, void *errhnd) {

  comm->ErrorHandler = errhndGetRef((Mpi_P_Errhandler *) errhnd);

  return COM_E_OK;
}


/*
 *  commGetErrHnd():
 *   Return an error handler for a communicator
 */
int commGetErrHnd (Mpi_P_Comm *comm, void **errhnd) {

  *errhnd = (Mpi_P_Errhandler *) comm->ErrorHandler;

  return COM_E_OK;
}


  /*-------------------------------------------------------/
 / Implementation of public ATTRIBUTE CACHING interface   /
/-------------------------------------------------------*/

/*
 *  commPutAttr():
 *   Put an attribute value in the communicator
 */
int commPutAttr (Mpi_P_Comm *comm, int keyval, void *value) {

  comm->Attributes[keyval].Value      = value;
  comm->Attributes[keyval].Allocated  = TRUE;

  return ATR_E_OK;
}


/*
 *  commGetAttr():
 *   Return an attribute value in parameter value
 *   Return 0 if there is an allocated attribute value, else != 0
 */
int commGetAttr (Mpi_P_Comm *comm, int keyval, void **value) {

  *value = comm->Attributes[keyval].Value;

  return (comm->Attributes[keyval].Allocated);
}


/*
 *  commDelAttr():
 *   Delete an attribute value
 *   Return 0 if attribute exists and else != 0
 */
int commDelAttr (Mpi_P_Comm *comm, int keyval) {

  if (!comm->Attributes[keyval].Allocated)
    return ATR_E_NOT_ALLOC;

  comm->Attributes[keyval].Value     = NULL;
  comm->Attributes[keyval].Allocated = FALSE;

  return ATR_E_OK;
}



  /*-------------------------------------------------------/
 /    Implementation of public TOPOLOGY interface         /
/-------------------------------------------------------*/
/*
 *  commGraphCreate():
 *   Return the type of topology or < 0 if topology is desactivated
 */
int commGraphCreate (Mpi_P_Comm *comm, int rank, int nnodes, int *index, int *edges) {

  Mpi_Graph_Topology  *graph;
  int                  i;

  /* 1. Make room */
  if (posix_memalign(&comm->Topology, CACHE_LINE_SIZE, sizeof(Mpi_P_Topology))) return TOPO_E_EXHAUST;
  
  /* 2. Initialize new graph */	
  comm->Topology->Type = TOPO_GRAPH;

  graph = &(comm->Topology->Topology.Graph);

  graph->Nindex = nnodes;
  graph->Nedges = index[nnodes - 1];

  for (i = 0; i < nnodes; i++)
	graph->Index[i] = index[i];

  for (i = 0; i < graph->Nedges; i++)
	graph->Edges[i] = edges[i];

  return TOPO_E_OK;
}


/*
 *  commGetDimGraph():
 *   Return the nodes and edges
 */
int commGraphGetDim (Mpi_P_Comm *comm, int *nnodes, int *nedges) {

  *nnodes = comm->Topology->Topology.Graph.Nindex;
  *nedges = comm->Topology->Topology.Graph.Nedges;

  return TOPO_E_OK;
}


/*
 *  commGraphGet():
 *   Return the Graph topology information
 */
int commGraphGet (Mpi_P_Comm *comm, int maxindex, int maxedges, int *index, int *edges) {

  Mpi_Graph_Topology  *graph;
  int                  i;

  graph =  &(comm->Topology->Topology.Graph);

  for (i = 0; i < maxindex; i++)
    index[i] = graph->Index[i];

  for (i = 0; i < maxedges; i++)
    edges[i] = graph->Edges[i];

  return TOPO_E_OK;
}


/*
 *  commGraphNborsCnt():
 *   Return the neighbors of a rank in the graph communicator
 */
int commGraphNborsCnt (Mpi_P_Comm *comm, int rank, int *nneighbors) {

  Mpi_Graph_Topology  *graph;

  graph =  &(comm->Topology->Topology.Graph);

  if (rank == 0)
    *nneighbors = graph->Index[rank];
  else
    *nneighbors = graph->Index[rank] - graph->Index[rank - 1];

  return TOPO_E_OK;
}


/*
 *  commGraphNbors():
 *   Return the neighbors of a rank in the graph communicator
 */
int commGraphNbors (Mpi_P_Comm *comm, int rank, int maxneighbors, int *neighbors) {

  Mpi_Graph_Topology  *graph;
  int                  i,
                       index_start,
                       index_end;

  graph =  &(comm->Topology->Topology.Graph);

  if (rank == 0) index_start = 0;
  else           index_start = graph->Index[rank - 1];

  index_end = graph->Index[rank];

  for (i = index_start; i < index_end; i++)
    *neighbors++ = graph->Edges[i];

  return TOPO_E_OK;
}


/*
 *  commGraphMap():
 *   Compute a placement for the calling process on the physical machine
 *   By now return the same rank of the calling process
 */
int commGraphMap(Mpi_P_Comm *comm, int rank, int nnodes, int *index, int *edges, int *newrank) {

  if (rank < nnodes)
	  *newrank = rank;
  else
    *newrank = PROC_NULL;

  return TOPO_E_OK;
}


/*
 *  commTopoType():
 *   Return the type of topology or < 0 if topology is desactivated
 */
int commTopoType (Mpi_P_Comm *comm) {

  if (comm->Topology == NULL)                    return TOPO_E_ALLOCATED;

  return (comm->Topology->Type);
}


/*
 *  commCartCreate():
 *   Allocate info for a topology in a communicator
 */
int commCartCreate (Mpi_P_Comm *comm, int nnodes, int rank, int ndims, int *dims, int *periods) {

  Mpi_Cart_Topology  *cart;
  int                 i;
  int                 r;
  //int                 n;

  /* 1. Make room */
  if (posix_memalign(&comm->Topology, CACHE_LINE_SIZE, sizeof(Mpi_P_Topology))) return TOPO_E_EXHAUST;
  
  /* 2. Initialize new cart */	  
  comm->Topology->Type = TOPO_CART;

  cart = &(comm->Topology->Topology.Cart);

  cart->Ndims = ndims;

/* MPICH algorithm
  n = nnodes;
  r = rank;
  for (i = 0; i < ndims; i++) {
	cart->Dims[i]     = dims[i];
	cart->Periods[i]  = periods[i];
	n = n / dims[i];
	cart->Position[i] = r / n;
	r = r % n;
  }
*/

  /* My algorithm (nnodes not needed) */
  r = rank;
  for (i = ndims - 1; i >= 0; i--) {
	cart->Dims[i]     = dims[i];
	cart->Periods[i]  = periods[i];
    cart->Position[i] = r % dims[i];
	r = r / dims[i];
  }


  return TOPO_E_OK;
}


/*
 *  commCartGetCoords():
 *   Return type of topology
 */
int commCartGetCoords(Mpi_P_Comm *comm, int rank, int maxdims, int *coords) {

  int   i;
  int   r;
  Mpi_Cart_Topology  *cart;


  cart =  &(comm->Topology->Topology.Cart);

  if (groupGetLocalRank(comm->LocalGroup) == rank)
    for (i = 0; i < maxdims; i++)
	    coords[i] = cart->Position[i];
  else {

/*
  nnodes = cart_ptr->topo.cart.nnodes;
  for ( i=0; i < cart_ptr->topo.cart.ndims; i++ ) {
	nnodes    = nnodes / cart_ptr->topo.cart.dims[i];
	coords[i] = rank / nnodes;
	rank      = rank % nnodes;
  }
*/
    r = rank;
    for (i = cart->Ndims - 1; i >= 0; i--) {
      coords[i] = r % cart->Dims[i];
      r = r / cart->Dims[i];
	  }
  }

  return TOPO_E_OK;
}


/*
 *  commCartGetRank():
 *   Return type of topology
 */
int commCartGetRank(Mpi_P_Comm *comm, int *coords, int *rank) {

  Mpi_Cart_Topology  *cart;
  int                 ndims;
  int                 multiplier;
  int                 coord;
  int                 i;


  cart =  &(comm->Topology->Topology.Cart);

  ndims = cart->Ndims;
  *rank = 0;
  multiplier = 1;

  for (i = ndims - 1; i >= 0; i--) {
    coord = coords[i];
    if (cart->Periods[i]) {
      if (coord >= cart->Dims[i])
		    coord = coord % cart->Dims[i];
      else if (coord <  0) {
		    coord = coord % cart->Dims[i];
        if (coord)
          coord = cart->Dims[i] + coord;
	    }
	  }
	  *rank += multiplier * coord;
	  multiplier *= cart->Dims[i];
  }

  return TOPO_E_OK;
}



/*
 *  commCartGetDim():
 *   Return the number of dimensions of the Catesian topology
 */
int commCartGetDim (Mpi_P_Comm *comm) {
  return (comm->Topology->Topology.Cart.Ndims);
}


/*
 *  commCartGet():
 *   Return the number of dimensions of the Catesian topology
 */
int commCartGet (Mpi_P_Comm *comm, int maxdims, int *dims, int *periods, int *coords) {

  Mpi_Cart_Topology  *cart;
  int                 i;

  cart =  &(comm->Topology->Topology.Cart);

  for (i = 0; i < maxdims; i++) {
    dims[i]    = cart->Dims[i];
	  periods[i] = cart->Periods[i];
	  coords[i]  = cart->Position[i];
  }

  return TOPO_E_OK;
}


/*
 *  commCartShift():
 *   Performs a shift in a Catesian topology, returning the source and destination ranks
 */
int commCartShift(Mpi_P_Comm *comm, int direction, int disp, int rank, int *src, int *dst) {

  Mpi_Cart_Topology  *cart;
  int                 i;
  int                 pos[MAX_CART_DIMS];


  /* 1. No displacement */
  if (disp == 0) {
    *src = *dst = rank;
	  return TOPO_E_OK;
  }

  cart =  &(comm->Topology->Topology.Cart);

  for (i = 0; i < cart->Ndims; i++) {
    pos[i] = cart->Position[i];
  }

  /* 2. Return PROC_NULL if shifted over the edge */
  pos[direction] += disp;
  if (!cart->Periods[direction] &&
      (pos[direction] >= cart->Dims[direction] ||
       pos[direction] < 0)) {
    *dst = PROC_NULL;
  }	else {
    commCartGetRank(comm, pos, dst);
  }

  pos[direction] = cart->Position[direction] - disp;
  if (!cart->Periods[direction] &&
      (pos[direction] >= cart->Dims[direction] ||
       pos[direction] < 0)) {
    *src = PROC_NULL;
  } else {
	  commCartGetRank(comm, pos, src);
  }

  return TOPO_E_OK;
}


/*
 *  commCartMap():
 *   Compute a placement for the calling process on the physical machine
 *   By now return the same rank of the calling process
 */
int commCartMap (Mpi_P_Comm *comm, int rank, int ndims, int *dims, int *periods, int *newrank) {

  int  nnodes;
  int  size;
  int  i;


  *newrank = MPI_UNDEFINED;

  nnodes = 1;
  for (i = 0; i < ndims; i++)
    nnodes *= dims[i];

  size = commGetSize(comm);
  if (size < nnodes)
	  return TOPO_E_EXHAUST;

  if (rank < nnodes)
    *newrank = rank;

  return TOPO_E_OK;
}


/*
 *  commCopyTopol():
 *   Copy the topology information from a communicator to another
 */
int commCopyTopol (Mpi_P_Comm *comm1, Mpi_P_Comm *comm2) {

  comm2->Topology = (Mpi_P_Topology *)NULL;
  if (comm1->Topology == (Mpi_P_Topology *)NULL) return TOPO_E_OK;

  if (posix_memalign(&comm2->Topology, CACHE_LINE_SIZE, sizeof(Mpi_P_Topology))) 
	return TOPO_E_EXHAUST;
  comm2->Topology->Type = comm1->Topology->Type;
  
  if (comm1->Topology->Type == TOPO_CART) {
    memcpy(&comm2->Topology->Topology, &comm1->Topology->Topology, sizeof(struct Mpi_Cart_Topology));
  } else /* Topo GRAPH */{
    memcpy(&comm2->Topology->Topology, &comm1->Topology->Topology, sizeof(struct Mpi_Graph_Topology));
  }

  return TOPO_E_OK;
}


  /*-------------------------------------------------------/
 /    Implementation of private DEBUG interface           /
/-------------------------------------------------------*/

#ifdef DEBUG_MODE

PRIVATE int printComm (CommTable *commtable) {

  int i, s;

  fprintf(stdout, " >>>>>> COMMS (TASK %d) <<<<<<  (commtable %x)\n", getRank(), commtable->Comms);
  /* TODO */
  /*
   objPrint(commtable->Comms);
   */

  return 0;
}

#endif


