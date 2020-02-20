

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

int main(int argc, char** argv){

time_t ctime;
time(&ctime);
struct tm *timeinfo = localtime(&ctime);
printf("time now is %s\n",asctime(timeinfo));
int  x ,y,times,i;
times=atoi(argv[1]); 
MPI_Request req;
int myrank ;
MPI_Init(&argc , &argv ) ;
MPI_Comm_rank(MPI_COMM_WORLD, &myrank) ;

if(myrank==0) {
  x = 5;

  for(i=0;i<times;i++){
  MPI_Isend(&x ,1 , MPI_INT, 0 , 99 , MPI_COMM_WORLD,&req) ;
  MPI_Recv(&y , 1 , MPI_INT, 1 , 99 , MPI_COMM_WORLD,MPI_STATUS_IGNORE);
  MPI_Wait(&req,MPI_STATUS_IGNORE);
  }
}

printf("rank%d finished,send %f, received %f\n",myrank,x,y);
//fflush(stdout);
MPI_Finalize();
return 0;
}
