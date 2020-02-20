#include <stdlib.h>
#include "mpi.h"
#include "klee.h"


int __Inj_MPI_Irecv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Request *request) {
    int i = 0;
    int j =1;
    klee_make_symbolic(&i, sizeof(int), "symi");
    klee_make_symbolic(&j, sizeof(int), "symj");
    if (source != MPI_ANY_SOURCE) {
        if(j>2)
            MPI_Send (&buf, count, MPI_INT, 1, 0, MPI_COMM_WORLD);
        else 
            MPI_Send (&buf, count, MPI_INT, 0, 0, MPI_COMM_WORLD);
        if (i > 10) {
            return MPI_Irecv(buf, count, datatype, MPI_ANY_SOURCE, tag, comm, request);
        } else {
            return MPI_Irecv(buf, count, datatype, source, tag, comm, request);
        }
    } else {
        if (i > 10) {
            int rank = -1;
            MPI_Comm_rank(MPI_COMM_WORLD, &rank);
            srand(time(NULL));
            int i = rand()%(rank + 1);
            return MPI_Irecv(buf, count, datatype, i, tag, comm, request); // we randomly pick up a process
        } else {
            return MPI_Irecv(buf, count, datatype, MPI_ANY_SOURCE, tag, comm, request);
        }
    }
}

int __Inj_MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status) {
    int i = 0;
    klee_make_symbolic(&i, sizeof(int), "symi");
    if (source != MPI_ANY_SOURCE) {
        if (i > 10) {
            return MPI_Recv(buf, count, datatype, MPI_ANY_SOURCE, tag, comm, status);
        } else {
            return MPI_Recv(buf, count, datatype, source, tag, comm, status);
        }
    } else {
        if (i > 10) {
            int rank = -1;
            MPI_Comm_rank(MPI_COMM_WORLD, &rank);
            srand(time(NULL));
            int i = rand() % (rank + 1);
            return MPI_Recv(buf, count, datatype, i, tag, comm, status); // we randomly pick up a process
        } else {
            return MPI_Recv(buf, count, datatype, MPI_ANY_SOURCE, tag, comm, status);
        }
    }
}
