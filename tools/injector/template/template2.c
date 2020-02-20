#include "mpi.h"
#include "klee.h"

int __Inj_MPI_Irecv(void *buf, int count, MPI_Datatype datatype, int source, int tag,	MPI_Comm comm, MPI_Request *request){
  int i = 0;
  klee_make_symbolic(&i, sizeof(int), "symi");
  if (source != MPI_ANY_SOURCE) {
    if (i > 10) {
      return MPI_Irecv(buf, count, datatype, source, tag, comm, request);
    } else {
      return MPI_Irecv(buf, count, datatype, MPI_ANY_SOURCE, tag, comm, request);
    }
  } else {
    if (i > 10) {
      return MPI_Irecv(buf, count, datatype, 1, tag, comm, request);// we just mutant it to receive from 1
    } else {
      return MPI_Irecv(buf, count, datatype, MPI_ANY_SOURCE, tag, comm, request); 
    }
  }
}

int __Inj_MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status){
  int i = 0;
  klee_make_symbolic(&i, sizeof(int), "symi");
  if (source != MPI_ANY_SOURCE) {
    if (i > 10) {
      return MPI_Recv(buf, count, datatype, source, tag, comm, status);
    } else {
      return MPI_Recv(buf, count, datatype, MPI_ANY_SOURCE, tag, comm, status);
    }
  } else {
    if (i > 10) {
      return MPI_Recv(buf, count, datatype, 1, tag, comm, status);
    } else {
      return MPI_Recv(buf, count, datatype, MPI_ANY_SOURCE, tag, comm, status); // we just mutant it to receive from 1
    }
  }
}
