#include <stdlib.h>
#include "mpi.h"
#include "klee.h"


int __Inj_MPI_Irecv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Request *request) {
    //int i = 0;
    //klee_make_symbolic(&i, sizeof(int), "symi");
    return MPI_Irecv(buf, count, datatype, MPI_ANY_SOURCE, tag, comm, request);
}

int __Inj_MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status) {
    return MPI_Recv(buf, count, datatype, MPI_ANY_SOURCE, tag, comm, status);
}
