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
#if defined(__OSI)
#include <osi.h>
#else
#include <stdio.h>
#include <pthread.h>
#endif

#include <arch.h>
#include <env.h>
#include <errhnd.h>
#include <check.h>

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Barrier
#define MPI_Barrier  PMPI_Barrier
#endif

/*
 *  dissemination_barrier
 *    Implements a barrier algorithm based on Hensgen, Finkel, Manber. IJPP 1988.
 *    Messages: 
 */
int dissemination_barrier (Mpi_P_Comm_t comm) {
  
  int    mpi_errno;
  int    local_rank;
  int    local_size;
  int    mask;
  int    src, dst;
  
  
  local_size = commGetSize(comm);
  local_rank = commGetRank(comm);
  
  if (local_size == 1)   return MPI_SUCCESS;
  
  NEST_FXN_INCR();
  
  mask = 0x1;
  while (mask < local_size) {
	
	dst = (local_rank + mask)              % local_size;
	src = (local_rank - mask + local_size) % local_size;
	
	CALL_MPI_NEST(MPI_Sendrecv(NULL, 0, MPI_BYTE, dst, BARRIER_TAG, 
							   NULL, 0, MPI_BYTE, src, BARRIER_TAG, comm, MPI_STATUS_IGNORE));
	
	mask <<= 1;
  }
  
  NEST_FXN_DECR();
  return MPI_SUCCESS;
  
mpi_exception_unnest:
  NEST_FXN_DECR();
  return mpi_errno;  
}


/*
 *  sr_barrier
 *    Implements a barrier algorithm based on sendrecv.
 *    Messages: 
 */
int sr_barrier (Mpi_P_Comm_t comm) {
  
  int    mpi_errno;
  int    local_rank;
  int    local_size;
  int    i;
  
  
  NEST_FXN_INCR();
  
  local_size = commGetSize(comm);
  local_rank = commGetRank(comm);
  
  /* Rank 0 is the root */
  if (local_rank == 0) {
    for (i = 0; i < local_size; i++)
      if (i != local_rank) {
        CALL_MPI_NEST(MPI_Recv(NULL, 0, MPI_BYTE, i, BARRIER_TAG, comm, MPI_STATUS_IGNORE));
      }
  } else {
    CALL_MPI_NEST(MPI_Sendrecv(NULL, 0, MPI_BYTE, 0, BARRIER_TAG, NULL, 0, MPI_BYTE, 0, BARRIER_TAG, comm, MPI_STATUS_IGNORE));
  }
  
  /* End the barrier. Rank 0 up the barrier */
  if (local_rank == 0)
    for (i = 0; i < local_size; i++)
      if (i != local_rank) {
        CALL_MPI_NEST(MPI_Send(NULL, 0, MPI_BYTE, i, BARRIER_TAG, comm));
      }
  
  
  NEST_FXN_DECR();
  return MPI_SUCCESS;
  
mpi_exception_unnest:
  NEST_FXN_DECR();
  return mpi_errno;  
}



void smp_barrier_old (Mpi_P_Comm_t comm) 
{
  int i     = 0;
  int size  = commGetSize(comm);
  int me    = commGetRank(comm);
  int sense = comm->comm_root->Barrier.Sense[me].Sense;
 
  //printf("smp_barrier_old. BEGIN\n");
  if (__sync_fetch_and_add(&comm->comm_root->Barrier.Counter, 1) == (size - 1)) {
    comm->comm_root->Barrier.Counter = 0;
    comm->comm_root->Barrier.Flag    = 1 - sense;
    //write_barrier();
  } 
  else {
    while (comm->comm_root->Barrier.Flag == sense) {
      if(++i % 1000 == 0) {
        //write_barrier();
        //sched_yield();
      }
    }
  }
  comm->comm_root->Barrier.Sense[me].Sense = 1 - sense;
  //printf("smp_barrier_old. END\n");

  return;
}




static inline int treeContig_bcast_Zero (const Mpi_P_Comm_t comm) 
{
  int       mask, i;
  int       dstRank, srcRank;
  int       src, dst;
  const int me   = commGetRank(comm);  
  const int size = commGetSize(comm);
  const int relative_rank = (me >= 0) ? me - 0 : me - 0 + size;
  
  mask = 0x1;
  while (mask < size) {
    if (relative_rank & mask) {
      src = me - mask;
      if (src < 0)
        src += size;
      srcRank = commGetGlobalRank(comm, src);
      AZQ_recv(srcRank, NULL, 0, BARRIER_TAG, AZQ_STATUS_IGNORE);
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
      AZQ_ssend(dstRank, NULL, 0, BARRIER_TAG); 
      //AZQ_sendLocalZero(dstRank, BCAST_TAG); 
    }
    mask >>= 1;
  }

  return MPI_SUCCESS;
}


//Herman added extern. Orelse klee complains that smp_barrier is a external call.
extern inline int smp_barrier (Mpi_P_Comm_t comm)
{
  int barrier __attribute__(( aligned(CACHE_LINE_SIZE) )) ;
  //int barrier;
  return(treeContig_bcast_Zero(comm)); 
}







/*
 *  inter_barrier
 *    Implements internode barrier
 */
//inline
int net_barrier (Mpi_P_Comm_t comm) {
  return dissemination_barrier(comm);
}



/*
 *  mcc_barrier
 *    Implements the algorithm for the barrier with the improvement for MCC
 *      (MultiCore Clusters) architectures
 */
//inline
int mcc_barrier (Mpi_P_Comm_t comm) {
  
  int                mpi_errno = MPI_SUCCESS;
  Mpi_P_Comm_t       comm_leaders;
  Mpi_P_Comm_t       comm_locals;
  
  
  comm_locals = commMCCGetLocals(comm);
  if (comm_locals != MPI_COMM_NULL)
    smp_barrier(comm_locals); 
  
  comm_leaders = commMCCGetLeaders(comm);
  if (comm_leaders != MPI_COMM_NULL) {
    mpi_errno = net_barrier(comm_leaders);
    if (0 > mpi_errno) return mpi_errno;
  }
  
  if (comm_locals != MPI_COMM_NULL) {
    smp_barrier(comm_locals);
  }
  return mpi_errno;  
}



/*
 *  MPI_Barrier
 *
 *    Main function for doing a barrier. It calls:
 *
 *      - MCC (multicore cluster) barrier if support mapping of processes
 *      - SMP (Symetric Multiprocessor) barrier if shared memory machine 
 *            algorithms support
 *      - NET barrier else.
 *
 */
//#define  DEBUG_MODE
int MPI_Barrier (MPI_Comm comm) {
  
  int mpi_errno;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Barrier (start)\tProcess: %d\n", commGetRank(comm)); fflush(stdout);
#endif
  
#ifdef CHECK_MODE
  if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_comm_type(comm, INTRACOMM))        goto mpi_exception;
#endif
  
    
  if (commMCCSupport(comm)) {
    printf("mcc_barrier\n"); fflush(stdout);
    mpi_errno = mcc_barrier(comm);
  } 
  else if (commSMPSupport(comm)) {
     mpi_errno = smp_barrier(comm);	
   //smp_barrier_old(comm);	
   //sr_barrier(comm);	
   //dissemination_barrier(comm);	
  } 
  else {	
    mpi_errno = net_barrier(comm);
  }
  
#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Barrier (end)  \tProcess:%d\n", commGetRank(comm)); fflush(stdout);
#endif
  
  return MPI_SUCCESS;
  
  
mpi_exception:
  return commHandleError (comm, mpi_errno, "MPI_Barrier");
}
