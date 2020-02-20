#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"
#include "hook.h"

struct bis {
  char dev[128];
  char maq[128];
};
typedef struct bis bis;

bis names[8] = {
 {"1102913728", "m0"},
 {"1103048896", "m1"},
 {"1103184064", "m2"},
 {"1103319232", "m3"},
 {"1090134208", "m4"},
 {"1089052864", "m5"},
 {"1076203712", "m6"},
 {"1077448896", "m7"}
};


int show_bis () {

  int          i, numprocs, myid;
  char         processor_name[MPI_MAX_PROCESSOR_NAME];
  int          namelen;


  MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank (MPI_COMM_WORLD, &myid);
  MPI_Get_processor_name(processor_name,&namelen);

  for (i = 0; i < 8; i++) {
    if (strcmp(&processor_name[0], &(names[i].dev)[0]) == 0) {
      fprintf(stdout, "Rank %d/%d in processor %d\n", myid, numprocs, names[i].maq); 
      fflush(stdout);
      return 0;
    }
  }

  /* Para Azequia */
  fprintf(stdout,  "Rank %d/%d in processor %d\n", myid, numprocs, processor_name); 

}


int main (int argc, char **argv) {

  int          numprocs, myid;
  char         message[20]={'\0'};
  MPI_Status   status;
  double       t_start, t_end;

 
  MPI_Init (NULL, NULL);
  MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank (MPI_COMM_WORLD, &myid);

  if (0 > show_bis()) {
  } 
  
  
  t_start = MPI_Wtime();
  
  if (myid == 0) {
		
    strcpy(message, "Hello, there");
    MPI_Send (message, strlen(message), MPI_CHAR, 1, 99, MPI_COMM_WORLD);

	
  } else if(myid == 1) {

    MPI_Recv(message, 20, MPI_CHAR, 0, 99, MPI_COMM_WORLD, &status);
    //printf("Received :%s:\n", message);
  //    for(int i=0;message[i]!='\0' && i<=19;i++)
//     printf("Received :%c:\n", message[i]);
  }

  //t_end = MPI_Wtime();
  //if (myid == 1) fprintf(stdout, "Time: %f  Resolution: %.9f\n", t_end - t_start, MPI_Wtick());
  
  MPI_Finalize ();

  return MPI_SUCCESS;
}

