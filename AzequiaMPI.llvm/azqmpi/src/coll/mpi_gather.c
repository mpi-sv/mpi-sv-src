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
#undef MPI_Gather
#define MPI_Gather  PMPI_Gather
#endif


/*
 *  MPI_Gather
 */
int MPI_Gather_orig (void *sendbuf, int sendcount, MPI_Datatype sendtype,
                void *recvbuf, int recvcount, MPI_Datatype recvtype,
                int root, MPI_Comm comm )  {

  int          mpi_errno;
  int          i;
  int          rank;
  int          gsize;
  int          dtrecvsz;
  MPI_Request  req;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Gather (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_comm_type(comm, INTRACOMM))        goto mpi_exception;
  if (mpi_errno = check_group(commGetLocalGroup(comm)))    goto mpi_exception;
  if (mpi_errno = check_count(sendcount))                  goto mpi_exception;
  if (mpi_errno = check_datatype(sendtype))                goto mpi_exception;
  if (mpi_errno = check_dtype_commit(sendtype))            goto mpi_exception;
#endif

  NEST_FXN_INCR();

  rank  = commGetRank(comm);
  if (rank == root) {
    gsize = commGetSize(comm);

#ifdef CHECK_MODE
    if (mpi_errno = check_count(recvcount))                goto mpi_exception_unnest;
    if (mpi_errno = check_datatype(recvtype))              goto mpi_exception_unnest;
    if (mpi_errno = check_dtype_commit(recvtype))          goto mpi_exception_unnest;
    if (mpi_errno = check_root(root, comm))                goto mpi_exception_unnest;
#endif

    dtrecvsz = dtypeGetExtent(recvtype) * recvcount;
	
    for (i = 0; i < gsize; i++) {
      if (i == root) { /* root copy the buffer without sending it */

        CALL_MPI_NEST(MPI_Irecv((char *)recvbuf + i * dtrecvsz,
                 recvcount, recvtype, i, GATHER_TAG, comm, &req));

      } else { /* root receive from the other processes */

        CALL_MPI_NEST(MPI_Recv( (char *)recvbuf + i * dtrecvsz,
                 recvcount, recvtype, i, GATHER_TAG, comm, NULL));

      }
    }
  }

  /* Send the data to root, itself included */
  CALL_MPI_NEST(MPI_Send(sendbuf, sendcount, sendtype, root, GATHER_TAG, comm));

  if (rank == root) { /* Deallocate request by root */
    CALL_MPI_NEST(MPI_Wait(&req, NULL));
  }

  NEST_FXN_DECR();

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Gather (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
  NEST_FXN_DECR();

mpi_exception:
  return commHandleError (comm, mpi_errno, "MPI_Gather");
}


























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
#undef MPI_Gather
#define MPI_Gather  PMPI_Gather
#endif




 int smp_gather_value(void *sendbuf, int sendcount, MPI_Datatype sendtype,
                            void *recvbuf, int recvcount, MPI_Datatype recvtype,
                            int   root, MPI_Comm comm) 
{
  int     me   = commGetRank(comm);
  int     size = commGetSize(comm);
  int     i;
  char    tmpBuff[1024*8] __attribute__(( aligned(CACHE_LINE_SIZE) ));
  int     nBytes = sendcount * dtypeGetExtent(sendtype);

  if (me == root) {
    copyData (sendbuf, recvbuf + me * nBytes, sendcount, sendtype, root, comm);
    for (i = 0; i < size; i++) {
      if (me == i)  continue;
      MPI_Recv (recvbuf + i * nBytes, recvcount, recvtype, i, GATHER_TAG, comm, MPI_STATUS_IGNORE);
    }
  } 
  else
    MPI_Send (sendbuf, sendcount, sendtype, root, GATHER_TAG, comm);
  return MPI_SUCCESS;
}




 int smp_gather_reference (void *sendbuf, int sendcount, MPI_Datatype sendtype,
                                 void *recvbuf, int recvcount, MPI_Datatype recvtype,
                                 int   root,    MPI_Comm comm)
{
  int          i;
  void        *rootRecvBuff;
  void        *srcAddr, *dstAddr;

  long long    difference;
  const int    me     = commGetRank(comm);
  const int    size   = commGetSize(comm);
  const int    nBytes = sendcount * dtypeGetExtent(sendtype);

  if (me == root) {
    rootRecvBuff = recvbuf;
  }

  treeContig_bcast(&rootRecvBuff, sizeof(void *), MPI_BYTE, root, comm); 

  dstAddr = rootRecvBuff + (me * nBytes);     
  srcAddr = sendbuf;
  //if(nBytes) 
    memcpy(dstAddr, srcAddr, nBytes );


  if (me == root) {    
    for (i = 0; i < size; i++) {
      if(i == me) continue;
      MPI_Recv (NULL, 0, MPI_INT, i, GATHER_TAG, comm, MPI_STATUS_IGNORE);
    }
  }
  else
    MPI_Send (NULL, 0, MPI_INT, root, GATHER_TAG, comm);

  return MPI_SUCCESS;
}




int smp_gather(void *sendbuf, int sendcount, MPI_Datatype sendtype,
                      void *recvbuf, int recvcount, MPI_Datatype recvtype,
                      int   root, MPI_Comm comm) 
{
  int nBytes = sendcount * dtypeGetExtent(sendtype);
  if(nBytes <= (128)) 
    return(smp_gather_value(sendbuf, sendcount, sendtype,
                            recvbuf, recvcount, recvtype,
                            root, comm)); 
  else 
    return(smp_gather_reference(sendbuf, sendcount, sendtype,
                                recvbuf, recvcount, recvtype,
                                root, comm)); 
}





/*
 *  net_Gather
 */
int net_gather (void *sendbuf, int sendcount, MPI_Datatype sendtype,
                void *recvbuf, int recvcount, MPI_Datatype recvtype,
                int root, MPI_Comm comm )  {

  int          mpi_errno;
  int          i;
  int          rank;
  int          gsize;
  int          dtrecvsz;
  MPI_Request  req;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Gather (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_comm_type(comm, INTRACOMM))        goto mpi_exception;
  if (mpi_errno = check_group(commGetLocalGroup(comm)))    goto mpi_exception;
  if (mpi_errno = check_count(sendcount))                  goto mpi_exception;
  if (mpi_errno = check_datatype(sendtype))                goto mpi_exception;
  if (mpi_errno = check_dtype_commit(sendtype))            goto mpi_exception;
#endif

  NEST_FXN_INCR();

  rank  = commGetRank(comm);
  if (rank == root) {
    gsize = commGetSize(comm);

#ifdef CHECK_MODE
    if (mpi_errno = check_count(recvcount))                goto mpi_exception_unnest;
    if (mpi_errno = check_datatype(recvtype))              goto mpi_exception_unnest;
    if (mpi_errno = check_dtype_commit(recvtype))          goto mpi_exception_unnest;
    if (mpi_errno = check_root(root, comm))                goto mpi_exception_unnest;
#endif

    dtrecvsz = dtypeGetExtent(recvtype) * recvcount;
	
    for (i = 0; i < gsize; i++) {
      if (i == root) { /* root copy the buffer without sending it */
        CALL_MPI_NEST(MPI_Irecv((char *)recvbuf + i * dtrecvsz,
                 recvcount, recvtype, i, GATHER_TAG, comm, &req));
      } 
      else { /* root receive from the other processes */
        CALL_MPI_NEST(MPI_Recv( (char *)recvbuf + i * dtrecvsz,
                 recvcount, recvtype, i, GATHER_TAG, comm, NULL));
      }
    }
  }

  /* Send the data to root, itself included */
  CALL_MPI_NEST(MPI_Send(sendbuf, sendcount, sendtype, root, GATHER_TAG, comm));

  if (rank == root) { /* Deallocate request by root */
    CALL_MPI_NEST(MPI_Wait(&req, NULL));
  }

  NEST_FXN_DECR();

#ifdef DEBUG_MODE
  fprintf(stdout, "net_Gather (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
  NEST_FXN_DECR();

mpi_exception:
  return commHandleError (comm, mpi_errno, "net_Gather");
}




/* 
 *  mcc_gather
 *    Implements the algorithm for MPI_Gather based on communicators created for improving the 
 *    reduce en multicore clusters
 */
int mcc_gather (void *sendbuf, int sendcount, MPI_Datatype sendtype,
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
  
  return -1;
  NEST_FXN_INCR();
  
  /* 1. Rank in comm */
  CALL_MPI_NEST(MPI_Comm_rank(comm, &rank));
  
  comm_locals  = commMCCGetLocals(comm);
  comm_leaders = commMCCGetLeaders(comm);


  tmpbuf = (char *) malloc (sendcount* dtypeGetExtent(sendtype));
  
  /* 4. Last, gather message between locals. Message is in relative rank 0.  */
  if (comm_locals != MPI_COMM_NULL) {
	
	//fprintf(stdout, "SMP_REDUCE rank: %d  mchn: %d  root: %d\n", rank, getCpuId(), root); fflush(stdout); 
     //mpi_errno = smp_gather(sendbuf, tmpbuf, count, datatype, op, 0, comm_locals);
	mpi_errno = smp_gather(sendbuf, sendcount, sendtype,
                              recvbuf, recvcount, recvtype,
                              root, comm_locals);
	
  }
  

  /* 3. First, reduce message between leaders */
  if (comm_leaders != MPI_COMM_NULL) {
	
	//fprintf(stdout, "NET_REDUCE rank: %d  mchn: %d  root: %d\n", rank, getCpuId(), root); fflush(stdout);
    //mpi_errno = net_reduce(tmpbuf, recvbuf, count, datatype, op, root, comm_leaders);
    mpi_errno = net_gather(sendbuf, sendcount, sendtype,
                           recvbuf, recvcount, recvtype,
                           root, comm_leaders);
    if (0 > mpi_errno) return mpi_errno;
  }
  
  free(tmpbuf);
   
  
  NEST_FXN_DECR();
  
  return mpi_errno; 
  
mpi_exception_unnest:
  NEST_FXN_DECR();
  return mpi_errno;
}















int MPI_Gather (void *sendbuf, int sendcount, MPI_Datatype sendtype,
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
  fprintf(stdout, "MPI_Gather(start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_comm_type(comm, INTRACOMM))        goto mpi_exception;
  if (mpi_errno = check_dest_comm(root, comm))             goto mpi_exception;
  if (mpi_errno = check_count(sendcount))                      goto mpi_exception;
  if (mpi_errno = check_count(recvcount))                      goto mpi_exception;
  if (mpi_errno = check_datatype(sendtype))                goto mpi_exception;
  if (mpi_errno = check_datatype(recvtype))                goto mpi_exception;
  //if (mpi_errno = check_datatype(datatype))                goto mpi_exception;
  //if (mpi_errno = check_dtype_commit(datatype))            goto mpi_exception;
#endif
  if(sendcount == 0)
    return MPI_SUCCESS;
  
  if (commMCCSupport(comm)) {
    mpi_errno = mcc_gather(sendbuf, sendcount, sendtype,
                           recvbuf, recvcount, recvtype,
                           root, comm);
  } 
  else if (commSMPSupport(comm)) {
    mpi_errno = smp_gather(sendbuf, sendcount, sendtype,
                           recvbuf, recvcount, recvtype,
                           root, comm);
  } 
  else {
    mpi_errno = net_gather(sendbuf, sendcount, sendtype,
                           recvbuf, recvcount, recvtype,
                           root, comm);
  }
  
  if (0 > mpi_errno)                                       goto mpi_exception;
  
#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Gather (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
  NEST_FXN_DECR();

mpi_exception:
  if (tmpbuf) free(tmpbuf);
  return commHandleError (comm, mpi_errno, "MPI_Gather");
}


