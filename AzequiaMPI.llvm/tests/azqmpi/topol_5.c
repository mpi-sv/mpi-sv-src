#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "mpi.h"

#define MAX_MPI_PROCS   32
#define DEGREE           2
#define FALSE            0

int main (int argc, char **argv) {

  int       myid, numprocs, graph_rank;
  int       nnodes  = 4;
  int       index[] = {2,3,4,6};
  int       edges[] = {1,3,0,3,0,2};
  int       nindex[MAX_MPI_PROCS];
  int       nedges[MAX_MPI_PROCS * DEGREE];
  MPI_Comm  comm_graph;
  int       newnodes, newedges;
  int       i;


  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);

  MPI_Graph_create(MPI_COMM_WORLD, nnodes, index, edges, FALSE, &comm_graph);

  if (comm_graph != MPI_COMM_NULL) {

    MPI_Comm_rank(comm_graph, &graph_rank);
    fprintf(stdout, "\nProcess %d (World: %d):\n", graph_rank, myid);

	  MPI_Graphdims_get(comm_graph, &newnodes, &newedges);
	  fprintf(stdout, "DIMS: nodes %d  edges %d\n", newnodes, newedges);

	  if (graph_rank == 0) {
	    MPI_Graph_get(comm_graph, newnodes, newedges, nindex, nedges);
	    fprintf(stdout, "\nINDEX\n");
	    for (i = 0; i < newnodes; i++)
	      fprintf(stdout, "%d ", nindex[i]);
	    fprintf(stdout, "\nEDGES\n");
	    for (i = 0; i < newedges; i++)
  	    fprintf(stdout, "%d ", nedges[i]);
	  }
  }

  MPI_Finalize();
  return MPI_SUCCESS;
}

