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
#include <mpi.h>

#undef wait
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <env.h>
#include <errhnd.h>
#include <check.h>
#include <arch.h>

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Reduce
#define MPI_Reduce  PMPI_Reduce
#endif


static inline int dimCube(int value) 
{
  int dim, size;
  
  for (dim = 0, size = 1; size < value; ++dim, size <<= 1) {
	continue;
  }
  
  return dim;
}


int copyData (void *from, void *to, int count, MPI_Datatype dtype, int root, MPI_Comm comm) {
  
  MPI_Request  req;
  int          nbytes;
  int          mpi_errno;
  
  if (dtypeIsContiguous(dtype)) {
    nbytes = dtypeGetExtent(dtype) * count;
    memcpy(to, from, nbytes);
  } 
  else {
    CALL_MPI_NEST(MPI_Irecv (to, count, dtype, root, REDUCE_TAG, comm, &req));
    CALL_MPI_NEST(MPI_Send (from, count, dtype, root, REDUCE_TAG, comm));
    CALL_MPI_NEST(MPI_Wait(&req, NULL));	
  }
  return 0;
 mpi_exception_unnest:
    NEST_FXN_DECR();
}


int lin_reduce (void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm) {
  
  int          mpi_errno;
  int          rank;
  int          size;
  char        *tmpbuf  = NULL;
  int          dtsizebytes;
  int          i;
  MPI_Request  req;
  
    
  NEST_FXN_INCR();
  
  rank = commGetRank(comm);
  
  /* 1. Root: Receive messages and operate on them */
  if (rank == root) {
    size        = commGetSize(comm);
    dtsizebytes = count * dtypeGetExtent(datatype);

    /* 1.1. Only malloc at root for temporary buffer */
    CALL_FXN (MALLOC(tmpbuf, dtsizebytes), MPI_ERR_INTERN);
	
    /* 1.2. No use memcpy or pack. Receive+Send avoids errors with derived non-contiguous datatypes */
    if (root == 0) {
	  
	  if (0 > copyData (sendbuf, recvbuf, count, datatype, root, comm))         goto mpi_exception_unnest;
	  
    } else {
	  
      CALL_MPI_NEST(MPI_Recv (recvbuf, count, datatype, 0, REDUCE_TAG, comm, NULL));
	  
    }
	
    for (i = 1; i < size; i++) {
	  
      /* 1.3. No use memcpy or pack. Receive+Send avoids errors with derived non-contiguous datatypes */
      if (root == i) {
		
		if (0 > copyData (sendbuf, tmpbuf, count, datatype, root, comm))        goto mpi_exception_unnest;
		
      } else {
		
        CALL_MPI_NEST(MPI_Recv (tmpbuf, count, datatype, i, REDUCE_TAG, comm, NULL));
		
      }
	  
      (copsGetFunction(op)) (tmpbuf, recvbuf, &count, datatype);
    }
	
    FREE (tmpbuf);
	
	/* 2. I am not the root, send the buffer. */
  } else {
	
    CALL_MPI_NEST(MPI_Send (sendbuf, count, datatype, root, REDUCE_TAG, comm));
	
  }
  
  NEST_FXN_DECR();
  
  return MPI_SUCCESS;
  
mpi_exception_unnest:
  NEST_FXN_DECR();
  
mpi_exception:
  if (tmpbuf) free(tmpbuf);
  return commHandleError (comm, mpi_errno, "MPI_Reduce");
}





int log_reduce (void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm) {
  
  int          mpi_errno;
  int          rank;
  int          size;
  char        *tmpbuf  = NULL;
  int          dtsizebytes;
  int          i;
  MPI_Request  req;
  
  int vrank;
  int dim;
  int extent;
  int peer;
  int mask;
  void *snd_buffer;
  
    
  if (!copsIsConmute(op)) {
	return lin_reduce (sendbuf, recvbuf, count, datatype, op, root, comm);
  }

  
  NEST_FXN_INCR();
  
  rank = commGetRank(comm);
  size = commGetSize(comm);
  vrank = (rank - root + size) % size;
  dim = dimCube(size);
  
  /* 1. Root: Receive messages and operate on them */	
  extent = dtypeGetExtent(datatype);
  dtsizebytes = count * extent;
  /* 1.1. Only malloc at root for temporary buffer */
  CALL_FXN (MALLOC(tmpbuf, dtsizebytes), MPI_ERR_INTERN);
  
  snd_buffer = sendbuf;
  
  /* 1.2. No use memcpy or pack. Receive+Send avoids errors with derived non-contiguous datatypes */
  for (i = 0, mask = 1; i < dim; ++i, mask <<= 1) {
	
	if (vrank & mask) {
	  
	  peer = vrank & ~mask;
	  peer = (peer + root) % size;
	  
	  CALL_MPI_NEST(MPI_Send (snd_buffer, count, datatype, peer, REDUCE_TAG, comm));
	  
	  snd_buffer = recvbuf;
	  
    } else {
	  
	  peer = vrank | mask;
	  if (peer >= size) continue;
	  peer = (peer + root) % size;
	  
	  
      CALL_MPI_NEST(MPI_Recv (tmpbuf, count, datatype, peer, REDUCE_TAG, comm, MPI_STATUS_IGNORE));
	  
      if (snd_buffer != sendbuf) {
        (copsGetFunction(op)) (tmpbuf, recvbuf, &count, datatype);
	  } else {
		(copsGetFunction(op)) (sendbuf, recvbuf, &count, datatype);
	  }
	  snd_buffer = tmpbuf;
	  tmpbuf = recvbuf;
    }
  }
  

  NEST_FXN_DECR();
  
  
  return MPI_SUCCESS;
  
mpi_exception_unnest:
  NEST_FXN_DECR();
  
mpi_exception:
  if (tmpbuf) free(tmpbuf);
  return commHandleError (comm, mpi_errno, "MPI_Reduce");
}



/* 
 *  net_reduce 
 *    Implements an internode reduce
 */
int net_reduce (void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm) {
  return lin_reduce(sendbuf, recvbuf, count, datatype, op, root, comm);
}


/* 
 *  smp_reduce
 *    Implements an intranode reduce
 *
inline int smp_reduce (void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm) {
  return lin_reduce(sendbuf, recvbuf, count, datatype, op, root, comm);
}
*/



 int smp_reduce_0 (void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm)
{
  int          mpi_errno;
  int          me;
  int          size;
  int          dtsizebytes;
  int          i;
  void        *tmpbuf;
  void        *sendBuffPtr = sendbuf;
  
  me = commGetRank(comm);
  
  /* Part I. Get all the sendbufs */
  if (me == root) {
    size        = commGetSize(comm);
	
    for (i = 0; i < size; i++) {
      if (me == i) 
        tmpbuf = sendbuf;
      else
        MPI_Recv (&tmpbuf, 8, MPI_BYTE, i, REDUCE_TAG, comm, NULL);
      (copsGetFunction(op)) (tmpbuf, recvbuf, &count, datatype);
    }
  } 
  else {
    MPI_Send (&sendBuffPtr, 8, MPI_BYTE, root, REDUCE_TAG, comm);
  }
  MPI_Barrier(comm);
  return MPI_SUCCESS;
}





inline int smp_reduce_1 (void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm) 
{
  int     mpi_errno;
  int     me;
  int     size;
  int     i;
  void   *sendBuff;
  
  me = commGetRank(comm);
  
  if (me == root) {
    copyData (sendbuf, recvbuf, count, datatype, root, comm);
    size        = commGetSize(comm);
    for (i = 0; i < size; i++) {
      if (me == i)  continue;
      MPI_Recv (&sendBuff, 8, MPI_BYTE, i, REDUCE_TAG, comm, MPI_STATUS_IGNORE);
      (copsGetFunction(op)) (sendBuff, recvbuf, &count, datatype);
      MPI_Send (NULL, 0, MPI_BYTE, i, REDUCE_TAG, comm);
    }
  } 
  else {
    MPI_Send (&sendbuf, 8, MPI_BYTE, root, REDUCE_TAG, comm);
    MPI_Recv (NULL, 0, MPI_BYTE, root, REDUCE_TAG, comm, MPI_STATUS_IGNORE);
  }
  return MPI_SUCCESS;
}









#ifdef FBOX_BUF_MAX

inline int smp_reduce_value(void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm) 
{
  int     me   = commGetRank(comm);
  int     size = commGetSize(comm);
  int     i;
  char    tmpBuff[FBOX_BUF_MAX*2] __attribute__(( aligned(CACHE_LINE_SIZE) ));
//dtsizebytes = count * dtypeGetExtent(datatype);

  if (me == root) {
    copyData (sendbuf, recvbuf, count, datatype, root, comm);
    for (i = 0; i < size; i++) {
      if (me == i)  continue;
      MPI_Recv (tmpBuff, count, datatype, i, REDUCE_TAG, comm, MPI_STATUS_IGNORE);
      (copsGetFunction(op)) (tmpBuff, recvbuf, &count, datatype);
    }
  } 
  else
    MPI_Send (sendbuf, count, datatype, root, REDUCE_TAG, comm);
  return MPI_SUCCESS;
}


#endif









inline int smp_reduce_reference_0 (void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm) 
{
  int        mpi_errno;
  int        me   = commGetRank(comm);
  int        size = commGetSize(comm);
  int        i    = 0, amount;
  void      *rootRecvBuff;
  int        done[128], doneNr = 0;

  /* Do not enter until getting my turn */
  //while (comm->comm_root->ReduceTurn != comm->ReduceMyTurn);

  if (me == root) {
    copyData (sendbuf, recvbuf, count, datatype, root, comm);
  
    /* Do not enter until getting my turn */
    while (comm->comm_root->ReduceTurn != comm->ReduceMyTurn);

    comm->comm_root->ReduceRootRecvBuf = recvbuf;

    /* Do not exit while lasting my turn (the buffer must be there untill it is finished) */
    while (comm->comm_root->ReduceTurn == comm->ReduceMyTurn);
  }

  else {

    int chunkSize = count / (size-1);
    int chunkRest = count % (size-1);
    int period    = chunkSize * dtypeGetExtent(datatype);

    /* Do not enter until getting my turn */
    while (comm->comm_root->ReduceTurn != comm->ReduceMyTurn);

    /* Ensure root has entered and stored the pointer of the reduction buffer */
    while((comm->comm_root->ReduceTurn != comm->ReduceMyTurn)  )
      if(++i  == 0) { 
        __asm__ __volatile__ ( "mfence" ::: "memory" );
        printf("**%d** ", i);
      }
    //printf("%d ", i);  
    rootRecvBuff = comm->comm_root->ReduceRootRecvBuf;
    for (i = 0; i < size-1; i++) 
      done[i] = 0;
    do {
      for (i = 0; i < size-1; i++) {
        if(done[i]) continue;
        lock(&comm->comm_root->ReduceLock[i].Cerrojo);
        amount = (i == size-1 ? chunkSize + chunkRest : chunkSize);
        (copsGetFunction(op)) (&((char *)sendbuf)[i*period ], &((char *)rootRecvBuff)[i*period ], &amount, datatype);
        unlock(&comm->comm_root->ReduceLock[i].Cerrojo) ;
        done[i] = 1;
        doneNr++;
      }
    } while (doneNr < size - 1);

    if(1 == __sync_fetch_and_add(&comm->comm_root->ReduceRootWait, -1)) {  
      comm->comm_root->ReduceRootRecvBuf = NULL; 
      comm->comm_root->ReduceRootWait    = size - 1;
      comm->comm_root->ReduceTurn        = !comm->comm_root->ReduceTurn; /* Update global turn */
    }
  }

  /* Update my turn */
  comm->ReduceMyTurn = !comm->ReduceMyTurn;

  return MPI_SUCCESS;
}



/*
                                    .---------------. .---------------. .---------------. .---------------. 
   L�neas de cach�                  `---------------� `---------------� `---------------� `---------------� 

                           +---------------+ +---------------+ +---------------+ +---------------+
   H1 "send buffer" B1     |       0       | |       1       | |       2       | |       3       |
                           +---------------+ +---------------+ +---------------+ +---------------+
                                   ||                ||                ||                ||        
                                   \/                \/                \/                \/        
                           +---------------+ +---------------+ +---------------+ +---------------+
   ROOT "recv buffer" BR   |       0       | |       1       | |       2       | |       3       |
                           +---------------+ +---------------+ +---------------+ +---------------+
                                   /\                /\                /\                /\       
                                   ||                ||                ||                ||       
                           +---------------+ +---------------+ +---------------+ +---------------+
   H2 "send buffer" B2     |       0       | |       1       | |       2       | |       3       |
                           +---------------+ +---------------+ +---------------+ +---------------+
*/

inline int smp_reduce_reference_1 (void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm) 
{
  int          mpi_errno;
  int          me   = commGetRank(comm);
  int          size = commGetSize(comm);
  int         *locK, i, amount, idx;
  void        *rootRecvBuff;
  int          done[128], doneNr = 0, nBytes = dtypeGetExtent(datatype);

  if (me == root) {
    rootRecvBuff = recvbuf;
    memset(recvbuf, 0, count * nBytes );
  }
  MPI_Bcast(&rootRecvBuff, sizeof(void *), MPI_BYTE, root, comm); 

  int chunkSize = count / (size);
  int chunkRest = count % (size);
  int period    = chunkSize * nBytes;

  for (i = 0; i < size; i++) 
    done[i] = 0;
  do {
    for (i = 0; i < size; i++) {
      if(done[i]) continue;
      amount = (i == size-1 ? chunkSize + chunkRest : chunkSize);
      locK     = &comm->comm_root->ReduceLock[i].Cerrojo;

      lock(locK);
      (copsGetFunction(op)) (&((char *)sendbuf)[i*period ], &((char *)rootRecvBuff)[i*period ], &amount, datatype);
      unlock(locK) ;

      done[i] = 1;
      doneNr++;
    }
  } while (doneNr < size);

  if (me == root) {    
    for (i = 0; i < size; i++) {
      if (me == i)  continue;
      MPI_Recv (NULL, 0, MPI_INT, i, REDUCE_TAG, comm, MPI_STATUS_IGNORE);
    }
  }
  else
    MPI_Send (NULL, 0, MPI_INT, root, REDUCE_TAG, comm);

  return MPI_SUCCESS;
}





inline int smp_reduce_reference_2 (void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm) 
{
  int          mpi_errno;
  int          me   = commGetRank(comm);
  int          size = commGetSize(comm);
  int          i, amount;
  void        *rootRecvBuff;
  int          done[128], doneNr = 0, nBytes;

  if (me == root) {
    //printf(".");
    rootRecvBuff = recvbuf;
    copyData (sendbuf, recvbuf, count, datatype, root, comm);
  }
  MPI_Bcast(&rootRecvBuff, sizeof(void *), MPI_BYTE, root, comm); 

  if (me == root) {    
    for (i = 0; i < size; i++) {
      if (me == i)  continue;
      MPI_Recv (NULL, 0, MPI_INT, i, REDUCE_TAG, comm, MPI_STATUS_IGNORE);
    }
  }
  else {
    int chunkSize = count / (size-1);
    int chunkRest = count % (size-1);
    int period = chunkSize * dtypeGetExtent(datatype);

    for (i = 0; i < size-1; i++) 
      done[i] = 0;
    do {
      for (i = 0; i < size-1; i++) {
        if(done[i]) continue;
        if(0 > trylock(&comm->comm_root->ReduceLock[i].Cerrojo)) continue;
        amount = (i == size-1 ? chunkSize + chunkRest : chunkSize);
        (copsGetFunction(op)) (&((char *)sendbuf)[i*period ], &((char *)rootRecvBuff)[i*period ], &amount, datatype);
        unlock(&comm->comm_root->ReduceLock[i].Cerrojo) ;
        done[i] = 1;
        doneNr++;
      }
    } while (doneNr < size - 1);
    MPI_Send (NULL, 0, MPI_INT, root, REDUCE_TAG, comm);
  }
  return MPI_SUCCESS;
}



//"extern" added by Herman
extern inline int smp_reduce_reference (const void * const sendbuf, void *recvbuf, const int count, const
								 MPI_Datatype datatype, const MPI_Op op, const int root, const MPI_Comm comm)
{
  int         *locK, i, amount, idx;
  void        *rootRecvBuff;
  void        *sendAddr, *recvAddr;
  
  long long    difference;
  const int    me     = commGetRank(comm);
  const int    size   = commGetSize(comm);
  const int    nBytes = dtypeGetExtent(datatype);
  
  if (me == root) {
	rootRecvBuff = recvbuf;
	memset(recvbuf, 0, count * nBytes );
  }
  
  MPI_Bcast(&rootRecvBuff, sizeof(void *), MPI_BYTE, root, comm);
  difference = sendbuf - rootRecvBuff;
  
  int chunkSize = count / (size);
  int chunkRest = count % (size);
  int period    = chunkSize * nBytes;
  
  for (i = 0; i < size; i++) {
	idx = i + me;
	if(idx >= size) idx -= size;
	
	amount   = (idx == size-1 ? chunkSize + chunkRest : chunkSize);
	recvAddr = &((char *)rootRecvBuff)[idx*period];
	sendAddr = recvAddr + difference;
	locK     = &comm->comm_root->ReduceLock[idx].Cerrojo;
	
	lock(locK);
	(copsGetFunction(op)) (sendAddr, recvAddr, &amount, datatype);
	unlock(locK);
  }
  //MPI_Barrier(comm);
  
  if (me == root) {
	for (i = 0; i < size; i++) {
	  if(i == me) continue;
	  MPI_Recv (NULL, 0, MPI_INT, i, REDUCE_TAG, comm, MPI_STATUS_IGNORE);
	}
  }
  else
	MPI_Send (NULL, 0, MPI_INT, root, REDUCE_TAG, comm);
  
  return MPI_SUCCESS;
}




//added by Herman to make klee stop complaining that is's a external one..
extern inline int smp_reduce(void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm)
{
#ifdef FBOX_BUF_MAX
  int nBytes = count * dtypeGetExtent(datatype);
  if(nBytes <= FBOX_BUF_MAX/2) 
    return(smp_reduce_value      (sendbuf, recvbuf, count, datatype, op, root, comm)); 
  else 
#endif
    return(smp_reduce_reference  (sendbuf, recvbuf, count, datatype, op, root, comm)); 
}





/* 
 *  mcc_reduce
 *    Implements the algorithm for MPI_Reduce based on communicators created for improving the 
 *    reduce en multicore clusters
 */
int mcc_reduce (void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm) {
  
  int           mpi_errno = MPI_SUCCESS;
  Mpi_P_Comm_t  comm_leaders;
  Mpi_P_Comm_t  comm_locals;
  int           rank;            /* Rank in comm */
  int           root_grank;
  int           global_rank;     /* My global rank */
  int           node;            /* My node */
  int           root_node;       /* Node where root is running */
  int           root_rlrank;     /* Relative rank of root in leaders */
  
  char         *tmpbuf;  
  
  NEST_FXN_INCR();
  
  /* 1. Rank in comm */
  CALL_MPI_NEST(MPI_Comm_rank(comm, &rank));
  
  comm_locals  = commMCCGetLocals(comm);
  comm_leaders = commMCCGetLeaders(comm);


  tmpbuf = (char *) malloc (count * dtypeGetExtent(datatype));
  
  /* 4. Last, reduce message between locals. Message is in relative rank 0.  */
  if (comm_locals != MPI_COMM_NULL) {
	
	//fprintf(stdout, "SMP_REDUCE rank: %d  mchn: %d  root: %d\n", rank, getCpuId(), root); fflush(stdout); 
	mpi_errno = smp_reduce(sendbuf, tmpbuf, count, datatype, op, 0, comm_locals);
	
  }
  

  /* 3. First, reduce message between leaders */
  if (comm_leaders != MPI_COMM_NULL) {
	
	//fprintf(stdout, "NET_REDUCE rank: %d  mchn: %d  root: %d\n", rank, getCpuId(), root); fflush(stdout);
    mpi_errno = net_reduce(tmpbuf, recvbuf, count, datatype, op, root, comm_leaders);
    if (0 > mpi_errno) return mpi_errno;
	
  }
  
  free(tmpbuf);
   
  
  NEST_FXN_DECR();
  
  return mpi_errno; 
  
mpi_exception_unnest:
  NEST_FXN_DECR();
  return mpi_errno;
}



/**
 *  MPI_Reduce
 *
 *    Main function for doing a reduce. It calls:
 *
 *      - MCC (multicore cluster) reduce if support mapping of processes
 *      - SMP (Symetric Multiprocessor) reduce if shared memory machine 
 *            algorithms support
 *      - NET reduce else.
 *
 */
int MPI_Reduce (void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm) {
  int old_chk_flag = klee_disable_sync_chk(0);
  int          mpi_errno;
  int          rank;
  int          size;
  char        *tmpbuf  = NULL;
  int          dtsizebytes;
  int          i;
  MPI_Request  req;


#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Reduce (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_comm_type(comm, INTRACOMM))        goto mpi_exception;
  if (mpi_errno = check_dest_comm(root, comm))             goto mpi_exception;
  if (mpi_errno = check_count(count))                      goto mpi_exception;
  if (mpi_errno = check_datatype(datatype))                goto mpi_exception;
  if (mpi_errno = check_dtype_commit(datatype))            goto mpi_exception;
  if (mpi_errno = check_ops(op))                           goto mpi_exception;
#endif
  if(count == 0)
    return MPI_SUCCESS;
  
  if (commMCCSupport(comm)) {
    mpi_errno = mcc_reduce(sendbuf, recvbuf, count, datatype, op, root, comm);
	
  } 
  else if (commSMPSupport(comm)) {
    mpi_errno = smp_reduce(sendbuf, recvbuf, count, datatype, op, root, comm);
  } 
  else {
    mpi_errno = net_reduce(sendbuf, recvbuf, count, datatype, op, root, comm);
  }
  
  if (0 > mpi_errno)                                       goto mpi_exception;
  
#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Reduce (end)  \tProcess: 0x%x\n", PCS_self());
#endif
  if(old_chk_flag)   	klee_enable_sync_chk(0);
  return MPI_SUCCESS;

mpi_exception_unnest:
  NEST_FXN_DECR();

mpi_exception:
  if (tmpbuf) free(tmpbuf);
  if(old_chk_flag)   	klee_enable_sync_chk(0);
  return commHandleError (comm, mpi_errno, "MPI_Reduce");
}
