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
 /   Declaration of types and functions used by this module        /
/----------------------------------------------------------------*/
#include <env.h>
#include <errhnd.h>
#include <check.h>


/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Bcast
#define MPI_Bcast  PMPI_Bcast
#endif



/* Threshold in bytes for different algorithms. These values could depend on 
   several values like architecture, number of ranks in collective, size of 
   message, topology, etc. */

/* Nehalem 8 ranks threshold for change to reference broadcast */
#define  REFERENCE_THRESHOLD  (1024)


/******************************************************************************/
/**********                      ALGORITHMS                          **********/
/******************************************************************************/


/*
 *  lin_bcast
 *    Implements a linear broadcast. Root send to all.
 */
int lin_bcast (void *buffer, int count, Mpi_P_Datatype_t dtype, int root, Mpi_P_Comm_t comm )  
{
  int          mpi_errno;
  int          i;
  int          me;
  int          gsize;
  int          dtsz;
  

  NEST_FXN_INCR();
  
  me  = commGetRank(comm);
  if (me == root) {
    gsize = commGetSize(comm);
    dtsz = dtypeGetExtent(dtype);
	
    for (i = 0; i < gsize; i++) {
      if (i == me) continue;
      CALL_MPI_NEST(MPI_Send(buffer, count, dtype, i, BCAST_TAG, comm));
    }
  }
  else {
    /* Receive the data from the root, itself included */
    CALL_MPI_NEST(MPI_Recv(buffer, count, dtype, root, BCAST_TAG, comm, NULL));
  }
  NEST_FXN_DECR();
    
  return MPI_SUCCESS;
  
mpi_exception_unnest:
  NEST_FXN_DECR();
  return mpi_errno;
}



/* 
 *  tree_bcast 
 *    Implements a binary tree broadcast algorithm
 */
int tree_bcast (void *buffer, int count, Mpi_P_Datatype_t dtype, int root, Mpi_P_Comm_t comm) {
  
  int           mpi_errno;
  int           mask;
  int           relative_rank;
  int           rank;
  int           size;
  int           src, dst;
  
  
  size = commGetSize(comm);
  rank = commGetRank(comm);
  
  relative_rank = (rank >= root) ? rank - root : rank - root + size;
  
  NEST_FXN_INCR();
  
  mask = 0x1;
  while (mask < size) {
    if (relative_rank & mask) {
      src = rank - mask;
      if (src < 0)
        src += size;
      CALL_MPI_NEST(MPI_Recv (buffer, count, dtype, src, BCAST_TAG, comm, MPI_STATUS_IGNORE));
      break;
    }
    mask <<= 1;
  }
  
  mask >>= 1;
  while (mask > 0) {
    if (relative_rank + mask < size) {
      dst = rank + mask;
      if (dst >= size)
        dst -= size;
      CALL_MPI_NEST(MPI_Send (buffer, count, dtype, dst, BCAST_TAG, comm));
    }
    mask >>= 1;
  }
  
  
  NEST_FXN_DECR();
  return MPI_SUCCESS;
  
mpi_exception_unnest:
  NEST_FXN_DECR();
  return mpi_errno;
}




/* 
 *  NO FUNCIONA TODAVIA
 *
 *  local_bcast 
 *    Implements a non-blocking send/blocking receive broadcast algorithm
 *    for intranode small scale nodes
 */
int local_bcast (void *buffer, int count, Mpi_P_Datatype_t dtype, int root, Mpi_P_Comm_t comm) {
  
  int           mpi_errno;
  int           rank;
  int           size;  
  int           i;
  MPI_Request   req[8];
  //MPI_Request   rqst;
  
  size = commGetSize(comm);
  rank = commGetRank(comm);
  
  NEST_FXN_INCR();
  
  if (rank == root) {
  
	req[root] = MPI_REQUEST_NULL;
	
	for (i = 0; i < size; i++) {
	  if (i == root) continue;
      CALL_MPI_NEST(MPI_Isend (buffer, count, dtype, i, BCAST_TAG, comm, &req[i]));
	  //CALL_MPI_NEST(MPI_Isend (buffer, count, dtype, i, BCAST_TAG, comm, &rqst));
	  //CALL_MPI_NEST(MPI_Wait (&rqst, MPI_STATUSES_IGNORE));
    }
	
	CALL_MPI_NEST(MPI_Waitall (size, req, MPI_STATUSES_IGNORE));
		  	
  } else {
  
    CALL_MPI_NEST(MPI_Recv (buffer, count, dtype, root, BCAST_TAG, comm, MPI_STATUS_IGNORE));
    
  }
  
  NEST_FXN_DECR();
  
  return MPI_SUCCESS;
  
mpi_exception_unnest:
  NEST_FXN_DECR();
  return mpi_errno;
}


/* 
 *  treeContig_bcast 
 *    Implements a tree broadcast algorithm for contiguous data
 */
int treeContig_bcast (const void *buffer, const int count, const Mpi_P_Datatype_t dtype, const int root, const Mpi_P_Comm_t comm) 
{
  int       mask, i;
  int       dstRank, srcRank;
  int       src, dst;
  const int me   = commGetRank(comm);  
  const int size = commGetSize(comm);
  const int cnt  = dtypeGetExtent(dtype) * count;
  const int relative_rank = (me >= root) ? me - root : me - root + size;
  
  mask = 0x1;
  while (mask < size) {
    if (relative_rank & mask) {
      src = me - mask;
      if (src < 0)
        src += size;
      srcRank = commGetGlobalRank(comm, src);
      AZQ_recv(srcRank, buffer, cnt, BCAST_TAG, AZQ_STATUS_IGNORE);
      break;
    }
    mask <<= 1;
  }
  
  mask >>= 1;
  i = 0;
  while (mask > 0) {
    if (relative_rank + mask < size) {
      dst = me + mask;
      if (dst >= size)
        dst -= size;
      dstRank = commGetGlobalRank(comm, dst);
      AZQ_send(dstRank, buffer, cnt, BCAST_TAG); 
    }
    mask>>= 1;
  }

  return MPI_SUCCESS;
}



/* 
 *  do_bcast 
 *    Select algorithm and do the broadcast
 */
inline int do_bcast (void *buffer, int count, Mpi_P_Datatype_t dtype, int root, Mpi_P_Comm_t comm )
{
  int  mpi_errno;

  /* Depends on many factors. Mainly:
   - Ranks number imply in operation
   - Message size
   - Topology (network, smp, mixed, hierarchical).
   - Architecture
   - Datatype (?)
   Take this information from the communicator (if possible) and do the rigth choice.
   */
  
  if      (0) {
    mpi_errno = lin_bcast (buffer, count, dtype, root, comm); 
  }
  else if (1) {
    mpi_errno = tree_bcast (buffer, count, dtype, root, comm); 
  }
  else if (0) {
	mpi_errno = local_bcast (buffer, count, dtype, root, comm); 
  }
  else if (0) {
	mpi_errno = treeContig_bcast (buffer, count, dtype, root, comm); 
  }
  
  return mpi_errno;  
}

/******************************************************************************/
/**********                      AUXILIARY                           **********/
/******************************************************************************/

inline int _smp_broadcast_reference (void *buffer, int count, Mpi_P_Datatype_t dtype, int root, Mpi_P_Comm_t comm )
{
  void        *rootBuff;
  void        *srcAddr, *dstAddr;
  const int    me     = commGetRank(comm);
  const int    size   = commGetSize(comm);
  const int    dtsz   = dtypeGetExtent(dtype);
  const int    nBytes = count * dtsz;
  int          i, dstRank;
  
  if (me == root) {
    rootBuff = buffer;
  }
  /* Broadcast address */
  do_bcast (&rootBuff, sizeof(void *), MPI_BYTE, root, comm );
  
  /* Synchronize ranks */
  if (me == root) {    
    for (i = 0; i < size; i++) {
      if(i == me) continue;
      MPI_Recv (NULL, 0, MPI_INT, i, BCAST_TAG, comm, MPI_STATUS_IGNORE);
    }
  }
  else {
    dstAddr = buffer;     
    srcAddr = rootBuff;
    memcpy(dstAddr, srcAddr, nBytes );
    //MPI_Send (NULL, 0, MPI_INT, root, REDUCE_TAG, comm);
    dstRank = commGetGlobalRank(comm, root);
    //AZQ_sendLocalZero(dstRank, EXTENDED_TAG(commGetContext(comm), REDUCE_TAG)); 
	AZQ_send(dstRank, NULL, 0, BCAST_TAG);
	
  }
  return MPI_SUCCESS;
}



inline int __smp_broadcast_reference (void *buffer, int count, Mpi_P_Datatype_t dtype, int root, Mpi_P_Comm_t comm )
{
  void        *rootBuff;
  void        *srcAddr, *dstAddr;
  const int    me     = commGetRank(comm);
  
  if (me == root) {
    rootBuff = buffer;
  }
  /* Broadcast address */
  do_bcast (&rootBuff, sizeof(void *), MPI_BYTE, root, comm);
  
  /* Do copy */
   if (me != root) {
   const int    size   = commGetSize(comm);
   const int    dtsz   = dtypeGetExtent(dtype);
   const int    nBytes = count * dtsz;
   
   dstAddr = buffer;     
   srcAddr = rootBuff;
   memcpy(dstAddr, srcAddr, nBytes);	
   }
    
  /* Synchronize ranks with root */
  //do_bcast(NULL, 0, MPI_BYTE, root, comm);
  //dissemination_barrier(comm);
  const int    size   = commGetSize(comm);
  int i;
  if (me == root) {    
    for (i = 0; i < size; i++) {
      if(i == me) continue;
      MPI_Recv (NULL, 0, MPI_INT, i, BCAST_TAG, comm, MPI_STATUS_IGNORE);
    }
  }
  else {
	int dstRank;
	
    dstRank = commGetGlobalRank(comm, root);
    //AZQ_sendLocalZero(dstRank, EXTENDED_TAG(commGetContext(comm), REDUCE_TAG)); 
	AZQ_send(dstRank, NULL, 0, BCAST_TAG);
	
  }
  
  return MPI_SUCCESS;
}



 int smp_broadcast_reference (void *buffer, int count, Mpi_P_Datatype_t dtype, int root, Mpi_P_Comm_t comm )
{
  void        *rootBuff, *destBuff;
  void        *srcAddr, *dstAddr;
  const int    me     = commGetRank(comm);
  
  
  int dest;
  int myroot;
  
  myroot = root;
  if ((root < 4) && (me >= 4)) {
	myroot = 4;
  }
  if ((root >= 4) && (me < 4)) {
	myroot = 0;
  }
  
  if (root < 4) dest = 4;
  else          dest = 0;
  if (me == root) {
	MPI_Send(buffer, count, dtype, dest, BCAST_TAG, comm);
  } 
  if (me == dest) {
	MPI_Recv(buffer, count, dtype, root, BCAST_TAG, comm, MPI_STATUS_IGNORE); 
  }
  
  /* Broadcast address */
  if (me == root) {
    rootBuff = buffer;
  }
  if (me == dest) {
	destBuff = buffer;
  }

  int i, j;
  MPI_Status stat;

  if (me == root) {
	if (root < 4) i = 0;
	else          i = 4;
	for (j = i; j <  i + 4; j++) {
	  if (j == root) continue;
	  MPI_Send(&rootBuff, sizeof(void *), MPI_BYTE, j, 3434, comm);
	  //fprintf(stdout, "ROOT %d ha enviado a %d\n", root, j);fflush(stdout);
	}
  }
  if (me == dest) {
	if (dest == 0) i = 0;
	else           i = 4;
	for (j = i; j < i + 4; j++) {
	  if (j == dest) continue;
	  MPI_Send(&destBuff, sizeof(void *), MPI_BYTE, j, 3434, comm);
	  //fprintf(stdout, "DEST %d ha enviado a %d\n", dest, j);fflush(stdout);
	}	
  }
  if((me != root) && (me != dest)) {
	MPI_Recv(&rootBuff, sizeof(void *), MPI_BYTE, myroot, 3434, comm, MPI_STATUS_IGNORE);
	//MPI_Recv(&rootBuff, sizeof(void *), MPI_BYTE, myroot, 3434, comm, &stat);
	//fprintf(stdout, " %d ha recibido de %d\n", me, stat.MPI_SOURCE); fflush(stdout);
  }
  //fprintf(stdout, " [%d] TERMINA broadcast de buffer %x\n", me, rootBuff); fflush(stdout);
  //do_bcast (&rootBuff, sizeof(void *), MPI_BYTE, root, comm);  
  //do_bcast (&destBuff, sizeof(void *), MPI_BYTE, dest, comm);
  //dissemination_barrier(comm);
  //fprintf(stdout, " ------------\n"); fflush(stdout);
  
  /* Do copy */
  if ((me != root) && (me != dest)) {
	const int    size   = commGetSize(comm);
    const int    dtsz   = dtypeGetExtent(dtype);
    const int    nBytes = count * dtsz;
	
	/*
	if ((root < 4) && (me < 4)) {
	  srcAddr = rootBuff;
	}
	if ((root < 4) && (me >= 4)) {
	  srcAddr = destBuff;
	}
	if ((root >= 4) && (me < 4)) {
	  srcAddr = destBuff;
	}
	if ((root >= 4) && (me >= 4)) {
	  srcAddr = rootBuff;
	}
	*/
	srcAddr = rootBuff;
	dstAddr = buffer;     
	memcpy(dstAddr, srcAddr, nBytes);		
  	
  }

  
  const int    size   = commGetSize(comm) / 2;
  
  if (me == root) {    
    for (i = 0; i < size - 1; i++) {
      //if(i == me) continue;
	  MPI_Recv (NULL, 0, MPI_INT, MPI_ANY_SOURCE, 252525, comm, MPI_STATUS_IGNORE);
      //MPI_Recv (NULL, 0, MPI_INT, MPI_ANY_SOURCE, 252525, comm, &stat);
	  //fprintf(stdout, "Root %d ha recibido de %d\n", root, stat.MPI_SOURCE); fflush(stdout);
    }
  }
  if (me == dest) {    
    for (i = 0; i < size - 1; i++) {
      //if(i == me) continue;
	  MPI_Recv (NULL, 0, MPI_INT, MPI_ANY_SOURCE, 252525, comm, MPI_STATUS_IGNORE);
      //MPI_Recv (NULL, 0, MPI_INT, MPI_ANY_SOURCE, 252525, comm, &stat);
	  //fprintf(stdout, "Dest %d ha recibido de %d\n", dest, stat.MPI_SOURCE); fflush(stdout);
    }
  }
  if((me != root) && (me != dest)) {
	int dstRank;

    //dstRank = commGetGlobalRank(comm, myroot);
	dstRank = myroot;
    //AZQ_sendLocalZero(dstRank, EXTENDED_TAG(commGetContext(comm), 252525)); 
	AZQ_send(dstRank, NULL, 0, 252525);
	//fprintf(stdout, " %d ha enviado a %d\n", me, dstRank);fflush(stdout); 
  }
  
  //fprintf(stdout, " [%d] TERMINA la synchro\n", me); fflush(stdout);
  
  /* Synchronize ranks with root */
  //do_bcast(NULL, 0, MPI_BYTE, root, comm);
  //dissemination_barrier(comm);
  
  return MPI_SUCCESS;
}







/******************************************************************************/
/**********                       TOPOLOGY                           **********/
/******************************************************************************/

/* 
 *  smp_bcast 
 *    Implements an intranode broadcast
 */
inline int smp_bcast (void *buffer, int count, Mpi_P_Datatype_t dtype, int root, Mpi_P_Comm_t comm) {
  
  int  mpi_errno;
  
  int  nBytes     = count * dtypeGetExtent(dtype);

  if(nBytes <= REFERENCE_THRESHOLD) { 
    mpi_errno = do_bcast (buffer, count, dtype, root, comm); 
  }
  else {
    mpi_errno = smp_broadcast_reference (buffer, count, dtype, root, comm); 
  }
  
  return mpi_errno;
}


/* 
 *  net_bcast 
 *    Implements an internode broadcast
 */
inline int net_bcast (void *buffer, int count, Mpi_P_Datatype_t dtype, int root, Mpi_P_Comm_t comm) {
  
  int  mpi_errno;
  
  mpi_errno = do_bcast(buffer, count, dtype, root, comm);
  
  return mpi_errno;
}


/* 
 *  mcc_bcast
 *    Implements the algorithm for MPI_Bcast based on communicators created for improving the 
 *    bcast en multicore clusters
 */
int mcc_bcast (void *buffer, int count, Mpi_P_Datatype_t dtype, int root, Mpi_P_Comm_t comm) {
  
  int           mpi_errno = MPI_SUCCESS;
  Mpi_P_Comm_t  comm_leaders;
  Mpi_P_Comm_t  comm_locals;
  int           rank;            /* Rank in comm */
  int           root_grank;
  int           global_rank;     /* My global rank */
  int           node;            /* My node */
  int           root_node;       /* Node where root is running */
  int           root_rlrank;     /* Relative rank of root in leaders */
  
  
  NEST_FXN_INCR();
  
  /* 1. Rank in comm */
  CALL_MPI_NEST(MPI_Comm_rank(comm, &rank));
  
  comm_locals  = commMCCGetLocals(comm);
  comm_leaders = commMCCGetLeaders(comm);
  
  /* 2. Root, but not leader. The root needs to send the buffer to its local leader */
  if ((rank == root) && (comm_leaders == (Mpi_P_Comm *)NULL)) {
	
	CALL_MPI_NEST(MPI_Send(buffer, count, dtype, 0, BCAST_TAG, comm_locals));
	
	/* 2.1. Not root, but leader */
  } else if ((rank != root) && (comm_leaders != (Mpi_P_Comm *)NULL)) {
	
	global_rank = commGetGlobalRank(comm, rank);
	PCS_getNode(global_rank, &node);
	
	root_grank = commGetGlobalRank(comm, root);
	PCS_getNode(root_grank, &root_node);
	
    /* 2.2. Only leader running on the same machine as root receive message */
	if (node == root_node) {
	  
	  CALL_MPI_NEST(MPI_Recv(buffer, count, dtype, MPI_ANY_SOURCE, BCAST_TAG, comm_locals, MPI_STATUS_IGNORE));
	  
	}
  }
  
  /* 3. First, broadcast message between leaders */
  if (comm_leaders != MPI_COMM_NULL) {
	
	/* 3.1. If the root of the broadcast is not the leader, now the root is the leader running in
	 the same node as root. All leaders need to know the node of root (Azequia support) and
	 find the leader of that machine */
	root_grank = commGetGlobalRank(comm, root);
	PCS_getNode(root_grank, &root_node);
	
	root_rlrank = comm->Node_Leaders[root_node];
	
    mpi_errno = net_bcast(buffer, count, dtype, root_rlrank, comm_leaders);
    if (0 > mpi_errno) return mpi_errno;
	
  }
  
  /* 4. Last, Broadcast message between locals. Message is in relative rank 0.  */
  if (comm_locals != MPI_COMM_NULL) {
	
	mpi_errno = smp_bcast(buffer, count, dtype, 0, comm_locals);
	
  }
  
  NEST_FXN_DECR();
  
  return mpi_errno; 
  
mpi_exception_unnest:
  NEST_FXN_DECR();
  return mpi_errno;
}



/******************************************************************************/
/**********                      MPI_BCAST                           **********/
/******************************************************************************/

/*
 *  MPI_Bcast
 *
 *    Main function for doing a broadcast. It calls:
 *
 *      - MCC (multicore cluster) broadcast if support mapping of processes
 *      - SMP (Symetric Multiprocessor) broadcast if shared memory machine 
 *            algorithms support
 *      - NET broadcast else.
 *
 */
int MPI_Bcast (void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm) {

  int           mpi_errno;
  int           mask;
  int           relative_rank;
  int           rank;
  int           size;
  int           src, dst;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Bcast (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_comm_type(comm, INTRACOMM))        goto mpi_exception;
  if (mpi_errno = check_datatype(datatype))                goto mpi_exception;
  if (mpi_errno = check_dtype_commit(datatype))            goto mpi_exception;
  if (mpi_errno = check_root(root, comm))                  goto mpi_exception;
  if (mpi_errno = check_count(count))                      goto mpi_exception;
#endif
  
  if(count == 0) return MPI_SUCCESS;

  if (commMCCSupport(comm)) {
	
	mpi_errno = mcc_bcast(buffer, count, datatype, root, comm);
  
  } else if (commSMPSupport(comm)) {
	
	mpi_errno = smp_bcast(buffer, count, datatype, root, comm);
	
  } else {
  
	mpi_errno = net_bcast(buffer, count, datatype, root, comm);
  
  }
  
  if (0 > mpi_errno)                                       goto mpi_exception;
  
#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Bcast (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception:
  return commHandleError (comm, mpi_errno, "MPI_Bcast");
}
