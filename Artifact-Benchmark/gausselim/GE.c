/*
 * $Id: gauss.c,v 10.12 1997/12/23 07:53:22 alc Exp $
 * modified by xueruini to use in windows.
 * 
 * change iters and size in main:
 * int i, iters = 1, j, size = 1024;
 */

//#include <windows.h>
#include <stdio.h>
//#include <unistd.h>
#include <stdlib.h>
#include <string.h>
//#include <sys/time.h>
#include "mpi.h"

#define SWAP(a,b)       { long tmp; tmp = (a); (a) = (b); (b) = tmp; }
#define ABS(a)		(((a) > 0) ? (a) : -(a))	/* much faster than fabs */

#define PIVOT_DATA 200



#define BARRIER_DATA 300
struct GlobalMemory {
  long	**a;
  int		odd_sweep_pivot, even_sweep_pivot;
};

void HZY_Barrier(int val,int nprocs,int me){
  int i, pid, x=1;
  MPI_Status status;

  pid = val % nprocs;

  if(pid != me){
    MPI_Send(&x, 1, MPI_INT, pid, BARRIER_DATA, MPI_COMM_WORLD);
    MPI_Recv(&x, 1, MPI_INT, pid, BARRIER_DATA, MPI_COMM_WORLD, &status);
  } else {
    for (i=0;i<nprocs;i++){
      if(i==me) continue;
      MPI_Recv(&x, 1, MPI_INT, i, BARRIER_DATA, MPI_COMM_WORLD, &status);
    }
    for (i=0;i<nprocs;i++){
      if(i==me) continue;
      MPI_Send(&x, 1, MPI_INT, i, BARRIER_DATA, MPI_COMM_WORLD);
    }

  }
}

void masterSignal(int val){

  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Bcast(&val, 1, MPI_INT, 0,  MPI_COMM_WORLD);
  /*
    if (val == 0){
    MPI_Finalize();
    exit(0);
    }
  */
}




#define LOWERVAL 0.0
#define UPPERVAL 2.0


long	**b;

int CheckRow(row, rownum, size)
     long *row;
     int rownum;
     int size; {

       int     i;
       int     errorcount = 0;

       for (i = 0; i < rownum; i++)
	 if (row[i] != LOWERVAL)
	   errorcount++;
       for (i = rownum; i < size; i++)
	 if (row[i] != UPPERVAL)
	   errorcount++;
       if(errorcount){
	 fprintf(stdout, "%d error in row %d\n",errorcount,rownum);
       }
       return errorcount;
     }


int CheckArray(a, size)	/* check_array & check_row used to check res */
     long **a;
     int size; {

       int i;
       int errorcount = 0;

       for (i = 0; i < size; i++)
	 errorcount += CheckRow(a[i], i, size);
       return errorcount;
     }


void TransposeAndZero(a, size) 
     long **a;
     int size; {

       int     i, j;

       for (i = 0; i < size; i++)
	 for (j = 0; j < i; j++) {
	   SWAP(a[i][j], a[j][i]);
	   a[i][j] = 0;
	 }
     }


void InitArray( long **a,  int size,int nprocs,int me)
 {

       int i, j;

       for (i = me; i < size; i += nprocs) {
	 for (j = 0; j < i; j++){
	   a[i][j] = j * 2 + 2;
	 }

	 for (j = i; j < size; j++){
	   a[i][j] = i * 2 + 2;
	 }
       }
     }


void PrintArray(a, size)
     long **a;
     int size; {

       int i, j;
       FILE *fp;

       //fp = fopen("res", "w");
       fp = stdout;
       printf("\n");
       for (i = 0; i < size; i++) {
	 for (j = 0; j < size; j++)
	   fprintf(fp, "%f   ", a[i][j]);
	 fprintf(fp, "\n");
       }
       //fclose(fp);
       printf("\n");
     }


void Compute(a, size, iters,nprocs,me,temp,Global)
     long **a;
     int size;
     int iters;
      int nprocs; 
	int me;
	char * temp;
	struct GlobalMemory *Global;{

       int	curr_pivot, pivot_col, j, k;
       int	starting_row = me;
       MPI_Status status;
       int i;
       //struct timeval start, finish;

       //if(!me)gettimeofday(&start, NULL);
       for (curr_pivot = 0; curr_pivot < iters; curr_pivot++) {

	 if (starting_row == curr_pivot) {	/* Find pivot element */
	   long max = ABS(a[curr_pivot][curr_pivot]);
	   pivot_col = curr_pivot;

	   for (j = curr_pivot+1; j < size; j++) {
	     long abs_j = ABS(a[curr_pivot][j]);
	     if (max < abs_j) {
	       pivot_col = j;
	       max = abs_j;
	     }
	   }

	   /* Place the pivot element on the diagonal */
	   SWAP(a[curr_pivot][curr_pivot], a[curr_pivot][pivot_col]);

	   if (curr_pivot & 0x01)		/* Odd sweep */
	     Global->odd_sweep_pivot = pivot_col;
	   else
	     Global->even_sweep_pivot = pivot_col;

	   for (j = curr_pivot+1; j < size; j++)
	     a[curr_pivot][j] /= a[curr_pivot][curr_pivot];

	   *(int *)temp = Global->odd_sweep_pivot;
	   *((int *)temp + 1) = Global->even_sweep_pivot;
	   memcpy(temp+2*sizeof(int), a[curr_pivot], size*sizeof(long));

	   for (i=0;i<nprocs;i++){
	     if(i==me) continue;
	     MPI_Send(temp, 2*sizeof(int)+size*sizeof(long), MPI_CHAR, i, PIVOT_DATA, MPI_COMM_WORLD);
	   }
	   //printf("%d: send pivot to others\n", me);

	   starting_row += nprocs;
	 } else {
	   MPI_Recv(temp, 2*sizeof(int)+size*sizeof(long), MPI_CHAR, MPI_ANY_SOURCE, PIVOT_DATA, MPI_COMM_WORLD, &status);
	   Global->odd_sweep_pivot = *(int *)temp;
	   Global->even_sweep_pivot = *((int *)temp + 1);
	   memcpy(a[curr_pivot], temp+2*sizeof(int), size*sizeof(long));
	   //printf("%d: receive pivot\n", me);
	 }

	 /*if(!me)gettimeofday(&finish, NULL);
	   if(!me)
	   printf("Elapsed time before barrier: %.6f seconds\n\n",
	   (((finish.tv_sec * 1000000.0) + finish.tv_usec) -
	   ((start.tv_sec * 1000000.0) + start.tv_usec)) / 1000000.0);*/

	 //MPI_Barrier(MPI_COMM_WORLD);
	 //HZY_Barrier(0);
	 /*if(!me)gettimeofday(&start, NULL);
	   if(!me)
	   printf("Elapsed time of barrier: %.6f seconds\n\n",
	   (((start.tv_sec * 1000000.0) + start.tv_usec) -
	   ((finish.tv_sec * 1000000.0) + finish.tv_usec)) / 1000000.0);*/



	 pivot_col = (curr_pivot & 0x01) ? Global->odd_sweep_pivot : 
	   Global->even_sweep_pivot;

	 for (k = starting_row; k < size; k += nprocs) {
	   SWAP(a[k][curr_pivot], a[k][pivot_col]);
	   for (j = curr_pivot + 1; j < size; j++) {
	     a[k][j] -= a[k][curr_pivot] * a[curr_pivot][j];
	   }
	 }
	 //if(!me)gettimeofday(&finish, NULL);
	 //MPI_Barrier(MPI_COMM_WORLD);
       }
       MPI_Barrier(MPI_COMM_WORLD);

     }


void main(argc, argv)
     int argc;
     char **argv; {
int     me, nprocs;
char	*temp;
struct GlobalMemory *Global;
       int i, iters = 2, j, size=32;//size = 4096;//, c;
       extern char *optarg;
       //struct timeval start, finish;
       //double start, finish;

       //start = GetTickCount();
       /* Initialise communication */
       MPI_Init(&argc, &argv);
       MPI_Comm_rank(MPI_COMM_WORLD, &me);
       MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
       /* todo
	  while ((c = getopt(argc, argv, "i:r:")) != -1) {
	  switch (c) {
	  case 'i':
	  iters = atoi(optarg);
	  break;
	  case 'r':
	  size = atoi(optarg);
	  break;
	  }
	  }
       */

       //printf("%d: Gaussian elimination on %d by %d using %d processors\n",
       //      me, size, size, nprocs);

       if (iters == 0)
	 iters = size - 1;
       else {
	 if ((iters < 1) || (iters >= size)) {
	   //printf("\tIllegal value, %d, to \"-i\" option.\n",
	   //	  iters);

	   iters = size - 1;
	 }
	 //printf("\tHalting after %d elimination steps.\n",
	 //iters);
       }

       Global = (struct GlobalMemory *) malloc(sizeof(struct GlobalMemory));
       Global->a = (long **) malloc(size*sizeof(long *));

       for (j = 0; j < nprocs; j++) {	/* Wacky init for distinct cachelines*/
	 for (i = j; i < size; i += nprocs) {
	   Global->a[i] = (long *) malloc(size*sizeof(long));
	 }
       }

       temp = (char *)malloc(size*sizeof(long) + 2*sizeof(int));

       MPI_Barrier(MPI_COMM_WORLD);

       InitArray(Global->a, size,nprocs,me);

       MPI_Barrier(MPI_COMM_WORLD);

       //gettimeofday(&start, NULL);

       Compute(Global->a, size, iters,nprocs,me,temp,Global);

       MPI_Barrier(MPI_COMM_WORLD);
       //HZY_Barrier(0);

       //gettimeofday(&finish, NULL);

       MPI_Barrier(MPI_COMM_WORLD);
       //HZY_Barrier(0);

       masterSignal(0);

       //finish = GetTickCount();
       if (!me) {
	 //printf("Elapsed time: %.3f seconds\n\n", (finish - start)/1000.0);

	 if (iters == size - 1) {
	   TransposeAndZero(Global->a, size);
	   //if ((i = CheckArray(Global->a, size)) != 0)
	   //printf("ERRORs IN RESULT! %d errors found\n", i);
	 }
	 // else
	 //printf("\tSkipping verification step.\n");
       }
       
       MPI_Finalize();
     }
