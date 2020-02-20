

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>


int main(int argc, char** argv){

float  x , y; 
int myrank ;
MPI_Init(&argc , &argv ) ;
MPI_Comm_rank(MPI_COMM_WORLD, &myrank) ;

if(myrank==0) {
  x = 5.5;
  MPI_Send(&x ,1 , MPI_FLOAT, 1 , 99 , MPI_COMM_WORLD) ;
  MPI_Recv(&y , 1 , MPI_FLOAT, 1 , 99 , MPI_COMM_WORLD,MPI_STATUS_IGNORE);
}
else if(myrank==1) {
  x = 6.5;
      MPI_Send(&x ,1 , MPI_FLOAT, 0 , 99 , MPI_COMM_WORLD) ;
      MPI_Recv(&y , 1 , MPI_FLOAT, 0 , 99 , MPI_COMM_WORLD,MPI_STATUS_IGNORE);
  }
 
printf("rank%d finished,send %f, received %f\n",myrank,x,y);
//fflush(stdout);
MPI_Finalize();
return 0;
}
