/* 
 *  (C) 2010 by University of Extremadura
 *      Juan A. Rico (jarico@unex.es)
 */

/* 
 * This is program that tests bsend when multiple threads invoking  the function.
 */

#include <stdio.h>
#include <stdlib.h>

#undef wait
#include "mpi.h"


#define  MSG_SIZE     1024

#define  DST_RANK     0
#define  SRC_RANK     1

#define  MAX_ITER     10000

#define  NUM_THREADS  20

#define  MAX_RQSTS    2


void errorhandler (MPI_Comm *comm, int *err, ...) {
  
  fprintf (stdout, "ERROR ");
  
  return;
}


void recv_messages() {
  
  int          *msg;
  int           i, j, k;
  int           rank;
  MPI_Request   req[MAX_RQSTS];
  
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);
  
  msg = (int *) malloc (MSG_SIZE * sizeof(int));
  
  for (j = 0; j < MSG_SIZE; j++) msg[j] = 0;
  
  for (i = 0; i < MAX_ITER; i++) {
	
	for (k = 0; k < MAX_RQSTS; k++) {
	  MPI_Irecv (msg, MSG_SIZE, MPI_INT, MPI_ANY_SOURCE, 99, MPI_COMM_WORLD, &req[k]);
	}
	
	for (k = 0; k < MAX_RQSTS; k++) {
	  MPI_Wait (&req[k], MPI_STATUS_IGNORE);
	}
	
	//if (!(i % (MAX_ITER / 10))) fprintf(stdout, "Thread %p send %d messages\n", pthread_self(), i);
  }
  
  free(msg);
  
  //fprintf (stdout, "[%d] END of sending - THREAD %p\n", rank, pthread_self());
  
}


void send_messages() {
  
  int          *msg;
  int           i, j, k;
  int           rank;
  MPI_Request   req[MAX_RQSTS];
  
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);
  
  msg = (int *) malloc (MSG_SIZE * sizeof(int));
  
  for (j = 0; j < MSG_SIZE; j++) msg[j] = 199;
  
  for (i = 0; i < MAX_ITER; i++) {
	
	for (k = 0; k < MAX_RQSTS; k++) {
	  if (MPI_SUCCESS != MPI_Isend (msg, MSG_SIZE, MPI_INT, DST_RANK, 99, MPI_COMM_WORLD, &req[k]))
        fprintf(stdout, "ERROR en MPI_Isend\n");
	}
	
	for (k = 0; k < MAX_RQSTS; k++) {
	  if (MPI_SUCCESS != MPI_Wait (&req[k], MPI_STATUS_IGNORE))
		fprintf(stdout, "ERROR en MPI_Wait  Thread: %p\n", (void *)pthread_self());
	}
	
	//MPI_Send (msg, MSG_SIZE, MPI_INT, DST_RANK, 99, MPI_COMM_WORLD);
	//if (!(i % (MAX_ITER / 10))) fprintf(stdout, "Thread %p send %d messages\n", pthread_self(), i);
  }
  
  free(msg);
  
  //fprintf (stdout, "[%d] END of sending - THREAD %p\n", rank, pthread_self());
  
}


void run_receivers () {
  
  pthread_t  thr[NUM_THREADS];
  int        errval;
  int        i;
  
  fprintf(stdout, "______ RUN RECEIVERS _______________________________\n");
  
  for (i = 0; i < NUM_THREADS; i++) {
    pthread_create(&thr[i], NULL, recv_messages, 0);
  }
  
  for (i = 0; i < NUM_THREADS; i++) {
    pthread_join(thr[i], &errval);
  }
}



void run_senders () {
  
  pthread_t  thr[NUM_THREADS];
  int        errval;
  int        i;
  
  fprintf(stdout, "______ RUN SENDERS _______________________________\n");
  
  for (i = 0; i < NUM_THREADS; i++) {
    pthread_create(&thr[i], NULL, send_messages, 0);
  }
  
  for (i = 0; i < NUM_THREADS; i++) {
    pthread_join(thr[i], &errval);
  }
}


int main (int argc, char **argv) {
  
  int            *msg;
  int             rank;
  int             provided;
  MPI_Errhandler  errhnd;
  double          t_start, t_end;
  MPI_Request     rqst;
  
  
  /*MPI_Init_thread (&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
  
  if (provided != MPI_THREAD_MULTIPLE) {
	fprintf(stdout, "Required MPI_THREAD_MULTIPLE, provided: %d\n", provided);
	MPI_Finalize();
	return 0;
  }*/
  MPI_Init(&argc, &argv);
  
  //MPI_Comm_create_errhandler (errorhandler, &errhnd);
  //MPI_Comm_set_errhandler (MPI_COMM_WORLD, errhnd);
  
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);
  
  msg = (int *) malloc (MSG_SIZE * sizeof(int));
  
  if (rank == DST_RANK) {
	
	//run_receivers();
	
	while (1) {
	  //MPI_Irecv (msg, MSG_SIZE, MPI_INT, MPI_ANY_SOURCE, 99, MPI_COMM_WORLD, &req);
	  //MPI_Wait(&req, MPI_STATUS_IGNORE);
	  MPI_Recv (msg, MSG_SIZE, MPI_INT, MPI_ANY_SOURCE, 99, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	  //sleep(1);
	  if (msg[0] == 0) break;
	}
	
	fprintf (stdout, "[%d] END of receiving\n", rank);
	
  } else if (rank == SRC_RANK) {
	
	t_start = MPI_Wtime();
	
	//run_senders();
	
	msg[0] = 0;
	//MPI_Send (msg, MSG_SIZE, MPI_INT, DST_RANK, 99, MPI_COMM_WORLD);
	if (MPI_SUCCESS != MPI_Isend (msg, MSG_SIZE, MPI_INT, DST_RANK, 99, MPI_COMM_WORLD, &rqst))
	  fprintf(stdout, "ERROR en MPI_Isend\n");
	if (MPI_SUCCESS != MPI_Wait(&rqst, MPI_STATUS_IGNORE))
	  fprintf(stdout, "ERROR en MPI_Wait\n");
	
	t_end = MPI_Wtime();
	
	fprintf (stdout, "[%d] END of sending.  Time: %.6f\n", rank, t_end - t_start);
	
  }
  
  free(msg);
  
  //MPI_Errhandler_free (&errhnd);
  
  MPI_Finalize();
  return 0;
}
