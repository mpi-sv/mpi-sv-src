#ifndef __HOOK_H__
#define __HOOK_H__
#include "mpi.h"
#include "mpise_comm.h"

#undef MPI_Irecv
#undef MPI_Recv
#undef MPI_Isend
#undef MPI_Barrier
#undef MPI_Wait
#undef MPI_Test
#undef MPI_Ssend
#undef MPI_Send
#undef MPI_Init
#undef MPI_Finalize
#undef MPI_Comm_rank
#undef MPI_Waitall
#undef MPI_Waitany
#undef MPI_Sendrecv
#undef MPI_Sendrecv_replace
#undef MPI_Alltoall
#undef MPI_Alltoallv
#undef MPI_Reduce
#undef MPI_Bcast
#undef MPI_Allreduce
#undef MPI_Comm_create
#undef MPI_Gather
#undef MPI_Allgather
#undef MPI_Gatherv
#undef MPI_Allgatherv
#undef MPI_Scatterv
#undef MPI_Scatter
#undef MPI_Unpack
#undef MPI_Pack
#undef MPI_Type_hvector

extern int PMPI_Ssend(void *buf, int count, MPI_Datatype datatype, int dest, int tag,		MPI_Comm comm);

int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag,		MPI_Comm comm, MPI_Status *status);
int MPI_LBSend(void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);
int MPI_Send (void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);
int MPI_Ssend(void *buf, int count, MPI_Datatype datatype, int dest, int tag,		MPI_Comm comm);
int MPI_Isend(void *buf, int count, MPI_Datatype datatype, int dest, int tag,		MPI_Comm comm, MPI_Request *request);
int MPI_Irecv(void *buf, int count, MPI_Datatype datatype, int source, int tag,		MPI_Comm comm, MPI_Request *request);
int MPI_Barrier(MPI_Comm  comm);
int MPI_Wait(MPI_Request *request, MPI_Status *status);
int MPI_Test(MPI_Request *request, int *flag, MPI_Status *status);
int MPI_Init(int *, char ***);
int MPI_Finalize(void);
int MPI_Waitall(int count, MPI_Request *reqs, MPI_Status *stats);
int MPI_Waitany(int count,MPI_Request *rqst, int *index, RQST_Status *status);
int MPI_Comm_rank(MPI_Comm comm, int *rank);
int MPI_Sendrecv(void *, int, MPI_Datatype,int, int, void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Status *);
int MPI_Sendrecv_replace (void *buf, int count, MPI_Datatype datatype, int dest, int sendtag,
				                  int source, int recvtag, MPI_Comm comm, MPI_Status *status);
int MPI_Alltoall (void *sendbuf, int sendcount, MPI_Datatype sendtype,
                  void *recvbuf, int recvcount, MPI_Datatype recvtype,
                  MPI_Comm comm );
int MPI_Alltoallv (void *sendbuf, int *sendcounts, int *sdispls, MPI_Datatype sendtype,
                   void *recvbuf, int *recvcounts, int *rdispls, MPI_Datatype recvtype,
                   MPI_Comm comm );
int MPI_Reduce (void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm);/**/
int MPI_Bcast (void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm); /**/
int MPI_Allreduce (void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm);/**/
int MPI_Comm_create (MPI_Comm comm, MPI_Group group, MPI_Comm *newcomm);
int MPI_Gather (void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype,
				int root, MPI_Comm comm ); /**/
int MPI_Allgather ( void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype,
					MPI_Comm comm ); /**/
int MPI_Gatherv (void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int *recvcounts, int *displs, MPI_Datatype recvtype,
				 int root, MPI_Comm comm ); /**/
int MPI_Allgatherv (void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int *recvcounts, int *displs,
					MPI_Datatype recvtype, MPI_Comm comm); /**/
int MPI_Scatterv (void *sendbuf, int *sendcounts, int *displs, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype,
				  int root, MPI_Comm comm ); /**/
int MPI_Scatter(void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype,
				  int root, MPI_Comm comm ); /**/
int MPI_Unpack (void *inbuf,  int insize,   int *position, void *outbuf, int outcount, MPI_Datatype datatype, MPI_Comm comm);
int MPI_Pack (void *inbuf,  int incount, MPI_Datatype datatype, void *outbuf, int outsize, int *position, MPI_Comm comm);
int MPI_Type_hvector (int count, int blocklength, Mpi_P_Aint stride, MPI_Datatype oldtype, MPI_Datatype *newtype);
#endif
