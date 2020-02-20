#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"

#define NUM_MSG   10000
#define MSG_SIZE  (8 * 1024)

int buff[MSG_SIZE];


int main (int argc, char **argv) {
  
  int        myid, numprocs, i, rank_leader, rank_local;
  double     startwtime = 0.0, endwtime;
  int        namelen;
  char       processor_name[MPI_MAX_PROCESSOR_NAME];
  MPI_Group  group_world, group_leaders, group_locals;
  MPI_Comm   comm_leaders, comm_locals;
  int        ranks[16];
  
  
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD,&myid);
  MPI_Get_processor_name(processor_name,&namelen);
  
  if (numprocs != 16) {
	fprintf(stdout, "ERROR: num. procs must be 16\n");
	MPI_Finalize();
	exit(1);
  }
	
  /* COMM LEADERS */
  MPI_Comm_group(MPI_COMM_WORLD, &group_world);
  for (i = 0; i < numprocs / 2; i++) {
    ranks[i] = i + (numprocs / 2);
  }
  
  MPI_Group_excl(group_world, numprocs / 2, ranks, &group_leaders);  
  MPI_Comm_create(MPI_COMM_WORLD, group_leaders, &comm_leaders);

  
  /* COMM LOCALS */
  if (myid < (numprocs / 2)) {
	ranks[0] = myid;
	ranks[1] = myid + (numprocs / 2);
  } else {
	ranks[0] = myid - (numprocs / 2);
	ranks[1] = myid;
  }
  
  //fprintf(stdout, "ID %d  [%d %d]\n", myid, ranks[0], ranks[1]);
  
  MPI_Group_incl(group_world, 2, ranks, &group_locals);
  MPI_Group_rank(group_locals, &rank_local);
  //fprintf(stdout, "Rank %d is rank %d in group LOCAL\n", myid, rank_local);
  
  MPI_Comm_create(MPI_COMM_WORLD, group_locals, &comm_locals);
  
  MPI_Comm_rank(comm_locals, &rank_local);
  //fprintf(stdout, "Rank %d is rank %d in comm LOCAL\n", myid, rank_local);
    
  
  /* BCAST */
  if (myid == 0) {
    fprintf(stdout, "Clock resolution: %lf\n", MPI_Wtick());
    startwtime = MPI_Wtime();
  }
  
  for (i = 0; i < NUM_MSG; i++) {
    if (comm_leaders != MPI_COMM_NULL)
	  MPI_Bcast(buff, MSG_SIZE, MPI_INT, 0, comm_leaders);
	MPI_Bcast(buff, MSG_SIZE, MPI_INT, 0, comm_locals);
  }
  
  if (myid == 0) {
    endwtime = MPI_Wtime();
    printf("Wall clock time = %f\n", endwtime - startwtime);
  }
  
  MPI_Group_free(&group_world);
  MPI_Group_free(&group_leaders);
  MPI_Group_free(&group_locals);
  MPI_Comm_free(&comm_leaders);
  MPI_Comm_free(&comm_locals);
  
  MPI_Finalize();
  return 0;
}
