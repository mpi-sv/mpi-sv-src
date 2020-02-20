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
#if defined (__OSI)
  #include <osi.h>
#else
  #include <string.h>
#endif

#include <env.h>
#include <errhnd.h>
#include <check.h>

/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Scatter
#define MPI_Scatter  PMPI_Scatter
#endif

int smp_scatter_value (void *sendbuf, int sendcount, MPI_Datatype sendtype,
                 void *recvbuf, int recvcount, MPI_Datatype recvtype,
                 int root, MPI_Comm comm )  
{
  int          mpi_errno;
  int          i;
  int          rank;
  int          gsize;
  int          dtsendsz;
  MPI_Request  req;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Scatter (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_comm_type(comm, INTRACOMM))        goto mpi_exception;
  if (mpi_errno = check_group(commGetLocalGroup(comm)))    goto mpi_exception;
  if (mpi_errno = check_count(recvcount))                  goto mpi_exception;
  if (mpi_errno = check_datatype(recvtype))                goto mpi_exception;
  if (mpi_errno = check_dtype_commit(recvtype))            goto mpi_exception;
#endif

  NEST_FXN_INCR();

  rank  = commGetRank(comm);

  if (rank == root) {
    gsize = commGetSize(comm);

#ifdef CHECK_MODE
    if (mpi_errno = check_count(sendcount))                goto mpi_exception_unnest;
    if (mpi_errno = check_datatype(sendtype))              goto mpi_exception_unnest;
    if (mpi_errno = check_dtype_commit(sendtype))          goto mpi_exception_unnest;
    if (mpi_errno = check_root(root, comm))                goto mpi_exception_unnest;
#endif

    dtsendsz = dtypeGetExtent(sendtype);

    for (i = 0; i < gsize; i++) {
      if (i == root) { /* root copy the buffer without sending it */
        CALL_MPI_NEST(MPI_Isend((char *)sendbuf + (i * sendcount * dtsendsz), sendcount, sendtype,
                                 i, SCATTER_TAG, comm, &req));
      } 
      else { /* root send to the other processes */
        CALL_MPI_NEST(MPI_Send((char *)sendbuf + (i * sendcount * dtsendsz), sendcount, sendtype, i, SCATTER_TAG, comm));
      }
    }
  }
  /* Receive the data from the root, itself included */
  CALL_MPI_NEST(MPI_Recv(recvbuf, recvcount, recvtype, root, SCATTER_TAG, comm, NULL));

  if (rank == root) { /* Root must deallocate the request */
    CALL_MPI_NEST(MPI_Wait(&req, NULL));
  }

  NEST_FXN_DECR();

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Scatter (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
  NEST_FXN_DECR();

mpi_exception:
  return commHandleError (comm, mpi_errno, "MPI_Scatter");
}




inline int smp_scatter_reference (void *sendbuf, int sendcount, MPI_Datatype sendtype,
                                  void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm )
{
  void        *rootSendBuff;
  void        *sendAddr, *recvAddr;
  const int    me     = commGetRank(comm);
  const int    size   = commGetSize(comm);
  const int    dtsendsz = dtypeGetExtent(sendtype);
  const int    nBytes   = sendcount * dtsendsz;
  int          i;

  if (me == root) {
    rootSendBuff = sendbuf;
  }

  MPI_Bcast(&rootSendBuff , sizeof(void *), MPI_BYTE, root, comm); 

  recvAddr = recvbuf;     
  sendAddr = rootSendBuff + (me * nBytes);
  memcpy(recvAddr, sendAddr, nBytes );

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




inline int smp_scatter(void *sendbuf, int sendcount, MPI_Datatype sendtype,
                       void *recvbuf, int recvcount, MPI_Datatype recvtype,  int root, MPI_Comm comm ) 
{
#ifdef FBOX_BUF_MAX
  int nBytes = sendcount * dtypeGetExtent(sendtype);
  if(nBytes <= FBOX_BUF_MAX/2) 
    return(smp_scatter_value      (sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm)); 
  else 
#endif
    return(smp_scatter_reference  (sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm)); 
}




/*
 *  net_Scatter
 */
int net_scatter (void *sendbuf, int sendcount, MPI_Datatype sendtype,
                 void *recvbuf, int recvcount, MPI_Datatype recvtype,
                 int root, MPI_Comm comm )  
{
  int          mpi_errno;
  int          i;
  int          rank;
  int          gsize;
  int          dtsendsz;
  MPI_Request  req;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Scatter (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_comm_type(comm, INTRACOMM))        goto mpi_exception;
  if (mpi_errno = check_group(commGetLocalGroup(comm)))    goto mpi_exception;
  if (mpi_errno = check_count(recvcount))                  goto mpi_exception;
  if (mpi_errno = check_datatype(recvtype))                goto mpi_exception;
  if (mpi_errno = check_dtype_commit(recvtype))            goto mpi_exception;
#endif

  NEST_FXN_INCR();

  rank  = commGetRank(comm);

  if (rank == root) {
    gsize = commGetSize(comm);

#ifdef CHECK_MODE
    if (mpi_errno = check_count(sendcount))                goto mpi_exception_unnest;
    if (mpi_errno = check_datatype(sendtype))              goto mpi_exception_unnest;
    if (mpi_errno = check_dtype_commit(sendtype))          goto mpi_exception_unnest;
    if (mpi_errno = check_root(root, comm))                goto mpi_exception_unnest;
#endif

    dtsendsz = dtypeGetExtent(sendtype);

    for (i = 0; i < gsize; i++) {
      if (i == root) { /* root copy the buffer without sending it */

        CALL_MPI_NEST(MPI_Isend((char *)sendbuf + (i * sendcount * dtsendsz), sendcount, sendtype,
                                 i, SCATTER_TAG, comm, &req));

      } else { /* root send to the other processes */

        CALL_MPI_NEST(MPI_Send((char *)sendbuf + (i * sendcount * dtsendsz), sendcount, sendtype, i, SCATTER_TAG, comm));

      }
    }
  }
  /* Receive the data from the root, itself included */
  CALL_MPI_NEST(MPI_Recv(recvbuf, recvcount, recvtype, root, SCATTER_TAG, comm, NULL));

  if (rank == root) { /* Root must deallocate the request */
    CALL_MPI_NEST(MPI_Wait(&req, NULL));
  }

  NEST_FXN_DECR();

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Scatter (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
  NEST_FXN_DECR();

mpi_exception:
  return commHandleError (comm, mpi_errno, "MPI_Scatter");
}







/* 
 *  mcc_scatter
 *    Implements the algorithm for MPI_Reduce based on communicators created for improving the 
 *    reduce en multicore clusters
 */
int mcc_scatter (void *sendbuf, int sendcount, MPI_Datatype sendtype,
                 void *recvbuf, int recvcount, MPI_Datatype recvtype,
                 int root, MPI_Comm comm) 
{
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


  tmpbuf = (char *) malloc (sendcount * dtypeGetExtent(recvtype)); 
  
  /* 4. Last, reduce message between locals. Message is in relative rank 0.  */
  if (comm_locals != MPI_COMM_NULL) {
    mpi_errno = smp_scatter(sendbuf, sendcount, sendtype, tmpbuf, recvcount, recvtype, root, comm_locals);
  }
  
  /* 3. First, reduce message between leaders */
  if (comm_leaders != MPI_COMM_NULL) {
    mpi_errno = net_scatter(tmpbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm_leaders);
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
 *  MPI_Scatter
 *
 *    Main function for doing a reduce. It calls:
 *
 *      - MCC (multicore cluster) reduce if support mapping of processes
 *      - SMP (Symetric Multiprocessor) reduce if shared memory machine 
 *            algorithms support
 *      - NET reduce else.
 *
 */
int MPI_Scatter(void *sendbuf, int sendcount, MPI_Datatype sendtype,
                 void *recvbuf, int recvcount, MPI_Datatype recvtype,
                 int root, MPI_Comm comm ) 
 {

  int          mpi_errno;
  int          rank;
  int          size;
  char        *tmpbuf  = NULL;
  int          dtsizebytes;
  int          i;
  MPI_Request  req;


#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Scatter(start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_comm_type(comm, INTRACOMM))        goto mpi_exception;
  if (mpi_errno = check_dest_comm(root, comm))             goto mpi_exception;
  if (mpi_errno = check_count(sendcount))                      goto mpi_exception;
  if (mpi_errno = check_count(recvcount))                      goto mpi_exception;
  if (mpi_errno = check_datatype(sendtype))                goto mpi_exception;
  if (mpi_errno = check_datatype(recvtype))                goto mpi_exception;
  //if (mpi_errno = check_count(count))                      goto mpi_exception;
  //if (mpi_errno = check_datatype(datatype))                goto mpi_exception;
  //if (mpi_errno = check_dtype_commit(datatype))            goto mpi_exception;
  //if (mpi_errno = check_ops(op))                           goto mpi_exception;
#endif
  if(sendcount == 0)
    return MPI_SUCCESS;

  
  if (commMCCSupport(comm)) {
    mpi_errno = mcc_scatter(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm);
	
  } 
  else if (commSMPSupport(comm)) {
    mpi_errno = smp_scatter(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm);
  } 
  else {
    mpi_errno = net_scatter(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm);
  }
  
  if (0 > mpi_errno)                                       goto mpi_exception;
  
#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Scatter(end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
  NEST_FXN_DECR();

mpi_exception:
  if (tmpbuf) free(tmpbuf);
  return commHandleError (comm, mpi_errno, "MPI_Reduce");
}

