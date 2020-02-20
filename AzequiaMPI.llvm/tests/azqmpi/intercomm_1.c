#if defined(__OSI)
  #include <osi.h>
#else
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
#endif


#include "mpi.h"


#define RANK  3

int main (int argc, char **argv) {

  int         myid, numprocs;
  int         flag;
  MPI_Comm    myComm; /* intra-communicator of local sub-group */
  MPI_Comm    myFirstComm; /* inter-communicator */
  MPI_Comm    mySecondComm; /* second inter-communicator (group 1 only) */
  int         membershipKey;


  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);

  if (myid == RANK) {
    MPI_Comm_test_inter(MPI_COMM_WORLD, &flag);
    fprintf(stdout, "MPI_COMM_WORLD is an %s\n", flag ? "Intercommunicator" : "Intracommunicator");
  }


  /* Generate membershipKey in the range [0, 1, 2] */
  membershipKey = myid % 3;

  /* Build intra-communicator for local sub-group */
  MPI_Comm_split(MPI_COMM_WORLD, membershipKey, myid, &myComm);

  /* Build inter-communicators. Tags are hard-coded. */
  if (membershipKey == 0) {

    /* Group 0 communicates with group 1. */
    MPI_Intercomm_create(myComm, 0, MPI_COMM_WORLD, 1, 01, &myFirstComm);

  } else if (membershipKey == 1) {

    /* Group 1 communicates with groups 0 and 2. */
    MPI_Intercomm_create(myComm, 0, MPI_COMM_WORLD, 0, 01, &myFirstComm);
    MPI_Intercomm_create(myComm, 0, MPI_COMM_WORLD, 2, 12, &mySecondComm);

  } else if (membershipKey == 2) {

    /* Group 2 communicates with group 1. */
    MPI_Intercomm_create(myComm, 0, MPI_COMM_WORLD, 1, 12, &myFirstComm);
  }


            /* Do work ... */

  if (myid == RANK) {
    MPI_Comm_test_inter(myFirstComm, &flag);
    fprintf(stdout, "myFirstComm is an %s\n", flag ? "Intercommunicator" : "Intracommunicator");
  }


            /* ... End work */



  /* free communicators appropriately */
  MPI_Comm_free(&myComm);
  MPI_Comm_free(&myFirstComm);
  if(membershipKey == 1)
    MPI_Comm_free(&mySecondComm);
  MPI_Finalize();

  return MPI_SUCCESS;
}
