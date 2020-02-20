#include <config.h>
#ifndef _F_PROTO_H
#define _F_PROTO_H


#include "f_util.h"


extern  void   mpi_init__       (MPI_Fint *ierr);
extern  void   mpi_finalize__   (MPI_Fint *ierr);

extern  void   mpi_comm_size__  (MPI_Fint *comm, MPI_Fint *rank, MPI_Fint *ierr);
extern  void   mpi_comm_rank__  (MPI_Fint *comm, MPI_Fint *rank, MPI_Fint *ierr);
extern  void   mpi_comm_dup__   (MPI_Fint *comm, MPI_Fint *newcomm, MPI_Fint *ierr);
extern  void   mpi_comm_split__ (MPI_Fint *comm, MPI_Fint *color, MPI_Fint *key, MPI_Fint *newcomm, MPI_Fint *ierr);
extern  void   mpi_comm_free__  (MPI_Fint *comm, MPI_Fint *ierr);

extern  void   mpi_send__       (char *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *ierr);
extern  void   mpi_recv__       (char *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *status, MPI_Fint *ierr);

extern  void   mpi_isend__      (char *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *dest, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr);
extern  void   mpi_irecv__      (char *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *request, MPI_Fint *ierr);
extern  void   mpi_wait__       (MPI_Fint *request, MPI_Fint *status, MPI_Fint *ierr);
extern  void   mpi_waitall__    (MPI_Fint *count, MPI_Fint *array_of_requests, MPI_Fint *array_of_statuses, MPI_Fint *ierr);

extern  void   mpi_barrier__    (MPI_Fint *comm, MPI_Fint *ierr);
extern  void   mpi_bcast__      (char *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *ierr);
extern  void   mpi_alltoall__   (char *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, char *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr);
extern  void   mpi_alltoallv__  (char *sendbuf, MPI_Fint *sendcounts, MPI_Fint *sdispls, MPI_Fint *sendtype, char *recvbuf, MPI_Fint *recvcounts, MPI_Fint *rdispls, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr);
extern  void   mpi_reduce__     (char *sendbuf, char *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *root, MPI_Fint *comm, MPI_Fint *ierr);
extern  void   mpi_allreduce__  (char *sendbuf, char *recvbuf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *op, MPI_Fint *comm, MPI_Fint *ierr);

extern  void   mpi_type_size__  (MPI_Fint *type, MPI_Fint *size, MPI_Fint *ierr);

extern  void   mpi_abort__      (MPI_Fint *comm, MPI_Fint *errorcode, MPI_Fint *ierr);

extern  double mpi_wtick__      (void);
extern  double mpi_wtime__      (void);


#endif
