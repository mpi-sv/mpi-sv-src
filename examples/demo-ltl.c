#include <stdio.h>
#include "mpi.h"

int main(int argc, char **argv) {
    int nprocs = -1;
    int rank = -1;
    char processor_name[128];
    int namelen = 128;

    MPI_Status status;
    MPI_Request req;

    /* init */
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Get_processor_name(processor_name, &namelen);
    printf("(%d) is alive on %s\n", rank, processor_name);
    fflush(stdout);

    if (rank == 0) {
        int v1;
        MPI_Recv(&v1, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);
    } else if (rank == 1) {
        MPI_Isend(&rank, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &req);
    } else if (rank == 2) {
        int v1;
        MPI_Recv(&v1, 1, MPI_INT, 3, 0, MPI_COMM_WORLD, &status);
    } else if (rank == 3) {
        MPI_Isend(&rank, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &req);
    }

    MPI_Finalize();
    printf("(%d) Finished normally ---------------\n", rank);

    return 0;
}