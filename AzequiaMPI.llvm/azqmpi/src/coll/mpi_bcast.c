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
  MPI_Request   req[128];
  
  size = commGetSize(comm);
  rank = commGetRank(comm);
  
  NEST_FXN_INCR();
  
  if (rank == root) {
  
	//posix_memalign(&req, 64, sizeof(MPI_Request) * size);
	for (i = 0; i < size; i++) {
	  if (i == root) {req[i] = MPI_REQUEST_NULL; continue;}
      CALL_MPI_NEST(MPI_Isend (buffer, count, dtype, i, BCAST_TAG, comm, &req[i]));
	  //fprintf(stdout, "Request ISEND %p para rank %d\n", req[i], i); 
    }
	
	//fprintf(stdout, "WAITALL -> \n", req[i], i); 
	
	CALL_MPI_NEST(MPI_Waitall (size, req, MPI_STATUSES_IGNORE));
		  
	//fprintf(stdout, "<- WAITALL \n", req[i], i); 
	//free(req);
	
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
 *  smp_bcast 
 *    Implements an intranode broadcast taking advance of SMP and threads capabilities
 */
inline int __smp_bcast (void *buffer, int count, Mpi_P_Datatype_t dtype, int root, Mpi_P_Comm_t comm) {

  int        rank;
  int        size;
  MPI_Aint   addr;
  long       addr_2;
  int        i;
  
  if (dtypeIsContiguous(dtype)) {

	size = commGetSize(comm);
	rank = commGetRank(comm);

	//sleep(2);
	//fprintf(stdout, "start %d rank of %d\n", rank, size);fflush(stdout);
	
	if (rank == root) {
	  addr = buffer;
	  //fprintf(stdout, "%d ROOT: %p\n", root, addr);fflush(stdout);
	  
	  for (i = 0; i < size; i++) {
		if (i == root) continue;
		MPI_Send(&addr, sizeof(MPI_Aint), MPI_BYTE, i, 33, comm);
	  }
	  
	} else {
	  //addr_r = 0;
	  //MPI_Allreduce(&addr_r, &addr_2, sizeof(MPI_Aint), MPI_BYTE, MPI_MAX, comm);
	  MPI_Recv(&addr_2, sizeof(MPI_Aint), MPI_BYTE, root, 33, comm, MPI_STATUS_IGNORE);
  	  //fprintf(stdout, "%d NO ROOT: %p\n", rank, addr_2);fflush(stdout);
	}
		
	//fprintf(stdout, "%d Rank: %p\n", getRank(), addr_2);fflush(stdout);
	
	if (rank != root) {
	  memcpy(buffer, addr_2, count * dtypeGetExtent(dtype));
	}
	
	MPI_Barrier(comm);
	
	//sleep(1);
	
  } else {
	return tree_bcast(buffer, count, dtype, root, comm);
  }
  
}



inline int _smp_bcast (void *buffer, int count, Mpi_P_Datatype_t dtype, int root, Mpi_P_Comm_t comm) {
  
  int        rank;
  int        size;
  MPI_Aint   addr;
  long       addr_2;
  int        i;
  void      *tmpbuf;
  
  int atm; // DEBE SER GLOBAL
  
  if (dtypeIsContiguous(dtype)) {
	
	size = commGetSize(comm);
	rank = commGetRank(comm);
	
	//sleep(2);
	//fprintf(stdout, "start %d rank of %d\n", rank, size);fflush(stdout);
	
	if (rank == root) {
	  
	  //posix_memalign(&tmpbuf, 64, count * dtypeGetExtent(dtype));
		tmpbuf = malloc(count * dtypeGetExtent(dtype));
	  atm = 0;
	  
	  addr = tmpbuf;
	  //fprintf(stdout, "%d ROOT: %p\n", root, addr);fflush(stdout);
	  
	  for (i = 0; i < size; i++) {
		if (i == root) continue;
		MPI_Send(&addr, sizeof(MPI_Aint), MPI_BYTE, i, 33, comm);
	  }
	  
	  if(__sync_bool_compare_and_swap(&atm, size, 0))
		free(tmpbuf);
	  
	} else {
	  //addr_r = 0;
	  //MPI_Allreduce(&addr_r, &addr_2, sizeof(MPI_Aint), MPI_BYTE, MPI_MAX, comm);
	  MPI_Recv(&addr_2, sizeof(MPI_Aint), MPI_BYTE, root, 33, comm, MPI_STATUS_IGNORE);
  	  //fprintf(stdout, "%d NO ROOT: %p\n", rank, addr_2);fflush(stdout);
	}
	
	//fprintf(stdout, "%d Rank: %p\n", getRank(), addr_2);fflush(stdout);
	
	if (rank != root) {
	  memcpy(buffer, addr_2, count * dtypeGetExtent(dtype));
	  __sync_fetch_and_add(&atm, 1);
	}
	
	//MPI_Barrier(comm);
	
	//sleep(1);
	
  } else {
	return tree_bcast(buffer, count, dtype, root, comm);
  }
  
}


/* 
 *  tree_bcast 
 *    Implements a tree broadcast algorithm
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
    mask >>= 1;
  }

  return MPI_SUCCESS;
}


/* 
 *  inter_bcast 
 *    Implements an internode broadcast
 */
int net_bcast (void *buffer, int count, Mpi_P_Datatype_t dtype, int root, Mpi_P_Comm_t comm) {
  
  int  mpi_errno;
  
  if (dtypeIsContiguous(dtype)) {
    mpi_errno = treeContig_bcast(buffer, count, dtype, root, comm);
  } 
  else {
    mpi_errno = tree_bcast(buffer, count, dtype, root, comm);
  }
  
  return mpi_errno;
}


/* 
 *  intra_bcast 
 *    Implements an intranode broadcast
 */



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





int smp_broadCast_value (void *buffer, int count, Mpi_P_Datatype_t dtype, int root, Mpi_P_Comm_t comm )  
{
  int          mpi_errno;
  int          i;
  int          me;
  int          gsize;
  int          dtsz;

#ifdef DEBUG_MODE
  fprintf(stdout, "smp_broadCast_value(%p): BEGIN\n", self()); fflush(stdout);
#endif

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

#ifdef DEBUG_MODE
  fprintf(stdout, "smp_broadCast_value(%p): END\n", self()); fflush(stdout);
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
  NEST_FXN_DECR();

mpi_exception:
  return commHandleError (comm, mpi_errno, "MPI_Scatter");
}


int smp_broadCast_reference (void *buffer, int count, Mpi_P_Datatype_t dtype, int root, Mpi_P_Comm_t comm )
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
  smp_broadCast_value (&rootBuff , sizeof(void *), MPI_BYTE, root, comm );


  if (me == root) {    
    for (i = 0; i < size; i++) {
      if(i == me) continue;
      MPI_Recv (NULL, 0, MPI_INT, i, REDUCE_TAG, comm, MPI_STATUS_IGNORE);
    }
  }
  else {
    dstAddr = buffer;     
    srcAddr = rootBuff;
    memcpy(dstAddr, srcAddr, nBytes );
    MPI_Send (NULL, 0, MPI_INT, root, REDUCE_TAG, comm);
    //dstRank = commGetGlobalRank(comm, root);
    //AZQ_sendLocalZero(dstRank, EXTENDED_TAG(commGetContext(comm), REDUCE_TAG));
    //timed_send(dstRank, (char *)&value[0], bufSize, REDUCE_TAG, 0, COM_FOREVER)))

  }
  return MPI_SUCCESS;
}



//Herman added extern to avoid clang do not export smp_bcast, so that klee reports smp_bcast is a external func
extern inline int smp_broadCast(void *buffer, int count, Mpi_P_Datatype_t dtype, int root, Mpi_P_Comm_t comm)
{
  int nBytes = count * dtypeGetExtent(dtype);
  if(nBytes <= 192) { 
    return(smp_broadCast_value(buffer, count, dtype, root, comm)); 
  }
  else 
    return(smp_broadCast_reference(buffer, count, dtype, root, comm)); 
}


//Herman added extern to avoid clang do not export smp_bcast, so that klee reports smp_bcast is a external func
extern inline int smp_bcast (void *buffer, int count, Mpi_P_Datatype_t dtype, int root, Mpi_P_Comm_t comm) {
  
  int  mpi_errno;
  
  if (dtypeIsContiguous(dtype)) {
    if(count < 2048)
      mpi_errno = treeContig_bcast(buffer, count, dtype, root, comm);
    else 
      mpi_errno = smp_broadCast(buffer, count, dtype, root, comm);
  } 
  else {
    mpi_errno = tree_bcast(buffer, count, dtype, root, comm);
  }
  
  return mpi_errno;
}


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
