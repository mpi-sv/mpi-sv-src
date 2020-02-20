/*
 Author: Jared Klingenberger <klinge2@clemson.edu>

 I worked on this assignment alone.

 I consulted a man page for MPI for how to create a derived MPI type based on a
 C struct found at the following address:

 http://mpi.deino.net/mpi_functions/MPI_Type_create_struct.html

 and the man page for MPI for how to tell if a message is waiting to be received
 on the receive queue (non-blocking):

 http://mpi.deino.net/mpi_functions/MPI_Iprobe.html
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h> /* for memset() */
#include <mpi.h>

#define WORKLOAD_SIZE 8
#define STRUCT_LEN 3
#define NUM_TYPES 5
#define MASTER 0

typedef int bool;
#define TRUE 1
#define FALSE 0

typedef struct {
  unsigned int uid;
  unsigned int i;
  double f;
} workload;

MPI_Datatype MPI_WORKLOAD;
workload stats_types[NUM_TYPES];
workload *stats_workers;

void create_MPI_Struct(MPI_Datatype *t);
unsigned int sleeptime(int i);
void compute_workload(workload *w);
int working(int queue_head);
void update_stats(workload *w, int id);
void copy_workload(workload *src, workload *dest);
void send_workload(workload *w, int to);
int recv_workload(workload *w, int from);
int handle_workers(workload *queue, int *queue_head, bool blocking, int size);
int workload_waiting();
void finish_worker(int worker, int *finished);

int main(int argc, char *argv[]){
  int rank, size;
  workload queue[WORKLOAD_SIZE]; // allocate 1024 tasks
  workload local_workload;
  int i, queue_head = 0, finished = 0;
  double start, stop; /* timing variables */

  MPI_Init(&argc,&argv);

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  create_MPI_Struct(&MPI_WORKLOAD);

  start = MPI_Wtime();

  // Seed rand for each process
  srand(time(NULL) + rank);

  /* Set up program execution */
  if (rank == MASTER) {
    stats_workers = malloc(sizeof(workload) * size); // the size of a task times the number of processes
    memset(stats_workers, 0, sizeof(workload) * size); // set the value in stats_workers be zero

    printf("Starting on %d workloads...\n", WORKLOAD_SIZE);

    // Generate workload of random ints in [0,4]
    for (i = 0; i < WORKLOAD_SIZE; i++) {
      queue[i].uid = i;
      queue[i].i = rand() % NUM_TYPES;
      queue[i].f = 0.0f;
    }

    // Deal initial round robin workloads
    for (i = 0; i < size; i++) {
      if (i == MASTER) { //do not allocate any task to master
        continue;
      } else if (working(queue_head)) { // test whether the tasks have been finished!
        send_workload(&queue[i], i); // send a task to a slave and increse queue_head
        queue_head++;
      } else {
        workload w;
        w.uid = -1;
        w.f = 0.0f;
        send_workload(&w, i); // the case that tasks < slaves, so the remaining slaves do not need to do any thing! 
        finish_worker(i, &finished);  //the number of finished slaves; a slave finished means i has been notified to terminate
      }
    }
  } 
  // now it is the slave code
  else {
   // recv_workload(&local_workload, MASTER);
MPI_Status stat;
    MPI_Recv(&local_workload, 1, MPI_WORKLOAD, MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
    if (local_workload.uid == -1)
      queue_head = -1;
  }

  /* Main computation loop */
  while (rank != MASTER && working(queue_head)) { // a while loop for slave to repetitively doing task
    compute_workload(&local_workload);

    // Return results to master
    send_workload(&local_workload, MASTER);
   // if (recv_workload(&local_workload, MASTER) == -1)
   //   queue_head = -1;
MPI_Status stat;
    MPI_Recv(&local_workload, 1, MPI_WORKLOAD, MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
    if (local_workload.uid == -1)
      queue_head = -1;
  }

  /* Continue listening until all finished */
  while (rank == MASTER && finished < size - 1) {
    int added = handle_workers(queue, &queue_head, TRUE, size); //receive the local results, the return value is the number of finished slaves
    finished += added;
    if (added > 0)
      printf("[%d] %d/%d finished\n", rank, finished, size - 1);
  }

  if (rank == MASTER) {
    /* Print out statistics */
    printf("\n### Statistics ###\n");
    for (i = 0; i < NUM_TYPES; i++) {
      workload *w = &stats_types[i];
      double avg = (w->i > 0) ? w->f / w->i : 0.0f;
      printf("Type %d:\tn=%d\ttot=%.3lf\tavg=%.3lf\n", i, w->i, w->f, avg);
    }
    printf("\n");
    for (i = 0; i < size; i++) {
      workload *w = &stats_workers[i];
      double avg = (w->i > 0) ? w->f / w->i : 0.0f;
      printf("Node %d:\tn=%d\ttot=%.3lf\tavg=%.3lf\n", i, w->i, w->f, avg);
    }

    stop = MPI_Wtime();
    printf("\nTotal execution time: %.3lf sec\n", stop - start);
    free(stats_workers);
  }

  printf("[%d] Done.\n", rank);
  MPI_Finalize();

  return 0;
}

int working(int queue_head) {
  return (queue_head >= 0 && queue_head < WORKLOAD_SIZE) ? TRUE : FALSE;
}

void update_stats(workload *w, int id) {
  stats_workers[id].i++;
  stats_workers[id].f += w->f;

  stats_types[w->i].i++;
  stats_types[w->i].f += w->f;
}

void copy_workload(workload *src, workload *dest) {
  dest->uid = src->uid;
  dest->i = src->i;
  dest->f = src->f;
}

void send_workload(workload *w, int to) {
  MPI_Send(w, 1, MPI_WORKLOAD, to, 0, MPI_COMM_WORLD);
}

int recv_workload(workload *w, int from) {
  MPI_Status stat;
  MPI_Recv(w, 1, MPI_WORKLOAD, from, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);

  int id = -1;
  if (working(w->uid))
    id = stat.MPI_SOURCE;
  return id;
}

void compute_workload(workload *w) {
  double begin, end;

  begin = MPI_Wtime();
  usleep(sleeptime(w->i));
  end = MPI_Wtime();

  w->f = end - begin;
}

int workload_waiting() {
  int flag;
  MPI_Status stat;

  MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &stat); //flag will be true if the tested message is available, otherwise false. 

  return flag;
}

int handle_workers(workload *queue, int *queue_head, bool blocking, int size) {
  int id, finished = 0;
  workload w;
  /* Check for waiting messages in the queue */
  //while (workload_waiting() || blocking) {
	if(*queue_head<WORKLOAD_SIZE){
	 id = recv_workload(&w, MPI_ANY_SOURCE); // obtain the process's id whose result was just received.	
	 copy_workload(&w, &queue[w.uid]);
         update_stats(&w, id);
	 send_workload(&queue[(*queue_head)++], id);
	 return 0;
	} else{
	   for(int i=1; i<size; i++){
            MPI_Status stat;
            MPI_Recv(&w, 1, MPI_WORKLOAD, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
            finish_worker(i, &finished);
           w.uid = -1;
           w.f = 0.0f;
		 //  send_workload(&w, i);
          MPI_Send(&w, 1, MPI_WORKLOAD, i, 0, MPI_COMM_WORLD);
	   }
	  return size-1;
	}
  //}	
	  
   /* id = recv_workload(&w, MPI_ANY_SOURCE); // obtain the process's id whose result was just received.
    if (id != -1) {
      copy_workload(&w, &queue[w.uid]);
      update_stats(&w, id);
      if (working(*queue_head))
        send_workload(&queue[(*queue_head)++], id); // increase the number of completed tasks!
      else {
        finish_worker(id, &finished);
        w.uid = -1;
        w.f = 0.0f;
        send_workload(&w, id); // notify the slaves that all tasks have been finished! 
      }
    }

    if (blocking) // always block, so finished always be at most 1!
      break;
  }
  return finished;*/
}

void finish_worker(int id, int *finished) {
  *finished += 1;
}

/* Sleep time in us */
unsigned int sleeptime(int i) {
  unsigned int ret = 0;
  switch (i) {
    case 0:
      ret = 100000 + rand() % 2900000;
      break;
    case 1:
      ret = 2000000 + rand() % 3000000;
      break;
    case 2:
      ret = 1000000 + rand() % 5000000;
      break;
    case 3:
      ret = 5000000 + rand() % 2500000;
      break;
    case 4:
      ret = 7000000 + rand() % 2000000;
      break;
  }
  return ret;
}

void create_MPI_Struct(MPI_Datatype *t) {
  MPI_Datatype types[STRUCT_LEN] = {MPI_UNSIGNED, MPI_UNSIGNED, MPI_DOUBLE};
  int blocklen[STRUCT_LEN] = {1, 1, 1};
  MPI_Aint disp[STRUCT_LEN];

  workload w;
  disp[0] = (void *)&w.uid - (void *)&w;
  disp[1] = (void *)&w.i - (void *)&w;
  disp[2] = (void *)&w.f - (void *)&w;

  //MPI_Type_create_struct(STRUCT_LEN, blocklen, disp, types, t);
FUNCTION(MPI_Type_struct)(STRUCT_LEN, blocklen, disp, types, t);
  MPI_Type_commit(t);
}
