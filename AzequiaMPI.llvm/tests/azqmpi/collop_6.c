#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"

#define LONG 16

void user_fxn_sum (void *invec, void *inoutvec, int *len, MPI_Datatype *datatype) {

	int i;

	if (*datatype == MPI_INT) {
			int *a = (int *)invec;
			int *b = (int *)inoutvec;

			for (i = 0; i < *len; i++) {
				b[i] = a[i] + b[i];
			}
		}
}


void user_fxn_subs (void *invec, void *inoutvec, int *len, MPI_Datatype *datatype) {

	int        i;
	static int odd = 0;


  if (*datatype == MPI_INT) {
			int *a = (int *)invec;
			int *b = (int *)inoutvec;

			if (odd++ % 2) {
			  for (i = 0; i < *len; i++) {
				  b[i] = a[i] - b[i];
			  }
			}
		}
}


int nrandom() {
  return rand() % 1000;
}


int main (int argc, char **argv) {

	int myrank, numprocs;
	int mess[LONG];
	int res[LONG];
	int i;
	int root = 0;
	MPI_Status	status;
	int count;
	MPI_Op myopsum, myopsub;
	int s;
	int sr;


	MPI_Init(NULL, NULL);

	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

  srand(9999 * myrank);

  /* One element SUM */
  s = myrank;
  sr = 0;
  MPI_Reduce(&s, &sr, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
  if (myrank == 0)
    printf("The SUM is:\t%d\n", sr);

  s = myrank;
  sr = 0;
  MPI_Reduce(&s, &sr, 1, MPI_INT, MPI_MAX, 2, MPI_COMM_WORLD);
  if (myrank == 2)
    printf("The MAX is:\t%d\n", sr);

  s = myrank;
  sr = 0;
  MPI_Reduce(&s, &sr, 1, MPI_INT, MPI_LAND, 3, MPI_COMM_WORLD);
  if (myrank == 3)
    printf("The LAND is:\t%d\n", sr);

  s = myrank;
  sr = 0;
  MPI_Reduce(&s, &sr, 1, MPI_INT, MPI_PROD, 1, MPI_COMM_WORLD);
  if (myrank == 1)
    printf("The PROD is:\t%d\n", sr);


  /* Vector sum */

	for (i = 0; i < LONG; i++)
		res[i] = 0;

  for (i = 0; i < LONG; i++)
    mess[i] = myrank;

	MPI_Reduce(mess, res, LONG, MPI_INT, MPI_SUM, root, MPI_COMM_WORLD);

	if (myrank == root) {
		printf("*** SUM ***\n");
		for (i = 0; i < LONG; i++)
			printf("\tResult %d is %d\n", i, res[i]);
		printf("---> Original buffer\n");
		for (i = 0; i < LONG; i++)
			printf("\tResult %d is %d\n", i, mess[i]);

	}


  /*Operation max */
	for (i = 0; i < LONG; i++)
		res[i] = 0;

  for (i = 0; i < LONG; i++)
	 	mess[i] = nrandom();

	MPI_Reduce(mess, res, LONG, MPI_INT, MPI_MAX, root, MPI_COMM_WORLD);

	if (myrank == root) {
		printf("*** MAX OPERATION ***\n");
		for (i = 0; i < LONG; i++)
			printf("\tResult %d is %d\n", i, res[i]);
	}


  /*MIN operation */
	for (i = 0; i < LONG; i++)
		res[i] = 0;

  for (i = 0; i < LONG; i++)
    mess[i] = nrandom();

	MPI_Reduce(mess, res, LONG, MPI_INT, MPI_MIN, root, MPI_COMM_WORLD);

	if (myrank == root) {
		printf("*** MIN OPERATION ***\n");
		for (i = 0; i < LONG; i++)
			printf("\tResult %d is %d\n", i, res[i]);
	}


  /* User created sum */
	for (i = 0; i < LONG; i++)
		res[i] = 0;

  for (i = 0; i < LONG; i++)
		mess[i] = myrank;

	MPI_Op_create (user_fxn_sum, 1, &myopsum);
	MPI_Reduce(mess, res, LONG, MPI_INT, myopsum, root, MPI_COMM_WORLD);
	if (myrank == root) {
		printf("*** USER SUM OPERATION ***\n");
		for (i = 0; i < LONG; i++)
			printf("\tResult %d is %d\n", i, res[i]);
	}
	MPI_Op_free (&myopsum);


  /* SUB user operation */
	for (i = 0; i < LONG; i++)
		res[i] = 0;

  for (i = 0; i < LONG; i++)
    mess[i] = nrandom();

  MPI_Op_create (user_fxn_subs, 0, &myopsub);

	MPI_Allreduce(mess, res, LONG, MPI_INT, myopsub, MPI_COMM_WORLD);
	printf("*** SUB USER OPERATION (Process %d) ***\n", myrank);
	for (i = 0; i < LONG; i++)
		printf("\tResult %d is %d\n", i, res[i]);
  MPI_Op_free (&myopsub);


  MPI_Finalize();
  return MPI_SUCCESS;
}
