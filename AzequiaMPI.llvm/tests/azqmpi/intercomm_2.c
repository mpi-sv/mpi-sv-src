#if defined(__OSI)
  #include <osi.h>
#else
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
#endif


#include "mpi.h"



int main (int argc, char **argv) {

  int         myid, numprocs;
  int         flag;
  MPI_Comm    myComm; /* intra-communicator of local sub-group */
  MPI_Comm    myFirstComm; /* inter-communicator */
  MPI_Comm    peer_comm;
  int         membershipKey;
  int         buf[10];
  MPI_Group   group_world, peer_group;
  int         ranks[2] = {0,1};
  MPI_Status  status;


  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);


  /* Generate membershipKey in the range [0, 1, 2] */
  membershipKey = myid % 2;

  /* Build intra-communicator for local sub-group */
  MPI_Comm_split(MPI_COMM_WORLD, membershipKey, myid, &myComm);

  MPI_Comm_group(MPI_COMM_WORLD, &group_world);
  MPI_Group_incl(group_world, 2, ranks, &peer_group);
  MPI_Comm_create(MPI_COMM_WORLD, peer_group, &peer_comm);

  /* Build inter-communicators. Tags are hard-coded. */
  if (membershipKey == 0) {

    /* Group 0 communicates with group 1. */
    MPI_Intercomm_create(myComm, 0, peer_comm, 1, 23, &myFirstComm);

  } else if (membershipKey == 1) {

    /* Group 1 communicates with groups 0 and 2. */
    MPI_Intercomm_create(myComm, 0, peer_comm, 0, 23, &myFirstComm);

  }


            /* Do work ... */

  if (myid == 0) {
    MPI_Comm_test_inter(myComm, &flag);
    fprintf(stdout, "myComm is an %s\n", flag ? "Intercommunicator" : "Intracommunicator");
    MPI_Comm_test_inter(myFirstComm, &flag);
    fprintf(stdout, "myFirstComm is an %s\n", flag ? "Intercommunicator" : "Intracommunicator");
  }


  if (myid == 0) {
    buf[0] = 98;
    MPI_Send(buf, 1, MPI_INT, 0, 45, myFirstComm);
  }
  if (myid == 1) {
    MPI_Recv(buf, 1, MPI_INT, 0, 45, myFirstComm, &status);
    fprintf(stdout, "Recibido %d\n", buf[0]);
  }



            /* ... End work */



  /* free communicators appropriately */
  MPI_Group_free(&group_world);
  MPI_Comm_free(&myComm);
  MPI_Comm_free(&myFirstComm);
  MPI_Finalize();
  return MPI_SUCCESS;
}
