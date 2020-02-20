/*
 Author: Jared Klingenberger <klinge2@clemson.edu>

 I worked on this assignment alone. I consulted a man page for MPI for how to
 create a derived MPI type based on a C struct found at the following address:

 http://mpi.deino.net/mpi_functions/MPI_Type_create_struct.html
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h> /* for memset() */
#include <math.h>   /* for sqrt() */
#include <mpi.h>

#define WORKLOAD_SIZE 1024
#define STRUCT_LEN 2
#define NUM_TYPES 5
#define ZSCORE 0.130

typedef struct {
  unsigned int i;
  double f;
} workload;

void create_MPI_Struct(MPI_Datatype *t);
unsigned int sleeptime(int i);
void partition_scheme(workload *queue, int *partitions, int size);
void compute_workload(workload *w);

int main(int argc, char *argv[]){
  int rank, size;
  int *partitions = NULL, *displs = NULL;
  workload queue[WORKLOAD_SIZE];
  workload *local_queue = NULL;
  workload local_result[NUM_TYPES];
  workload *times = NULL;
  int local_size;
  int i, j;
  MPI_Datatype MPI_WORKLOAD;
  double start, stop; /* timing variables */
  MPI_Status status;

  memset(&local_result, 0, sizeof(local_result));

  MPI_Init(&argc,&argv);

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  create_MPI_Struct(&MPI_WORKLOAD);

  start = MPI_Wtime();

  // Seed rand for each process
  srand(time(NULL) + rank);

  // Generate workload of random ints in [0,4]
  if (rank == 0) {
    times = malloc(sizeof(workload) * size);
    partitions = malloc(sizeof(int) * size);
    displs = malloc(sizeof(int) * size);

    memset(times, 0, sizeof(workload) * size);
    memset(partitions, 0, sizeof(int) * size);
    memset(displs, 0, sizeof(int) * size);

    printf("Starting on %d workloads...\n", WORKLOAD_SIZE);

    for (i = 0; i < WORKLOAD_SIZE; i++)
      queue[i].i = rand() % NUM_TYPES;

    fflush(stdout);

    partition_scheme(queue, partitions, size);

    // create displacements array from partition sizes
    for (i = 1; i < size; i++)
      displs[i] += displs[i-1] + partitions[i-1];
  }

  // Send local partition sizes
  MPI_Scatter(partitions, 1, MPI_INT, &local_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

  // Allocate memory for local_queue now that we have a size
  local_queue = malloc(sizeof(workload) * local_size);
  printf("[%d] local_size = %d\n", rank, local_size);

  // Use MPI_Scatter to distribute the actual workloads
  MPI_Scatterv(queue, partitions, displs, MPI_WORKLOAD, local_queue, local_size, MPI_WORKLOAD, 0, MPI_COMM_WORLD);

  /* Begin computing workload */
  for (i = 0; i < local_size; i++) {
    workload *w = &local_queue[i];

    compute_workload(w);

    local_result[w->i].i += 1;
    local_result[w->i].f += w->f;
  }

  /* Send stats back to master */
  if (rank != 0) {
    MPI_Send(local_result, NUM_TYPES, MPI_WORKLOAD, 0, rank, MPI_COMM_WORLD);
  } else {
    /* Sum master work times */
    for (i = 0; i < NUM_TYPES; i++) {
      times[0].i += local_result[i].i;
      times[0].f += local_result[i].f;
    }

    workload recvbuf[NUM_TYPES];
    for (i = 1; i < size; i++) {
      MPI_Recv(recvbuf, NUM_TYPES, MPI_WORKLOAD, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

      for (j = 0; j < NUM_TYPES; j++) {
        local_result[j].i += recvbuf[j].i;
        local_result[j].f += recvbuf[j].f;
        times[status.MPI_SOURCE].i += recvbuf[j].i;
        times[status.MPI_SOURCE].f += recvbuf[j].f;
      }
    }

    printf("\n### Statistics ###\n");

    for (i = 0; i < NUM_TYPES; i++) {
      workload *w = &local_result[i];
      double avg = (w->i > 0) ? w->f / w->i : 0.0f;
      printf("Type %d:\tn=%d\ttot=%.3lf\tavg=%.3lf\n", i, w->i, w->f, avg);
    }

    printf("\n");

    for (i = 0; i < size; i++) {
      workload *w = &times[i];
      double avg = (w->i > 0) ? w->f / w->i : 0.0f;
      printf("Node %d:\tn=%d\ttot=%.3lf\tavg=%.3lf\n", i, w->i, w->f, avg);
    }

    stop = MPI_Wtime();
    printf("\nTotal execution time: %.3lf sec\n", stop - start);
    free(times);
    free(partitions);
    free(displs);
  }

  MPI_Finalize();

  free(local_queue);
  return 0;
}

void compute_workload(workload *w) {
  double begin, end;

  begin = MPI_Wtime();
  usleep(sleeptime(w->i));
  end = MPI_Wtime();

  w->f = end - begin;
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

/* Calculate partitioning scheme */
void partition_scheme(workload *queue, int *partitions, int size) {
  int i, cur, sum;
  float avg, avg_per_core, sigma, range;

  // Calculate mean
  sum = 0;
  for (i = 0; i < WORKLOAD_SIZE; i++)
    sum += queue[i].i;

  avg = (float)sum / WORKLOAD_SIZE;
  avg_per_core = (float)sum / (float)size;

  // Calculate standard deviation
  sigma = 0;
  for (i = 0; i < WORKLOAD_SIZE; i++) {
    float dist = queue[i].i - (float)avg;
    sigma += dist * dist;
  }
  sigma = sigma / (float)size;
  sigma = sqrt(sigma);
  range = ZSCORE * sigma + avg;

  // Decide partition sizes within range of mean
  i = 0;
  for (cur = 0; cur < size - 1; cur++) {
    int o;
    sum = 0;

    for (o = i; o < WORKLOAD_SIZE; o++) {
      if (sum + queue[o].i < avg_per_core + range)
        sum += queue[o].i;
      else break;
    }

    partitions[cur] = o - i;
    i += o - i;
  }
  partitions[cur] = WORKLOAD_SIZE - i;

  printf("Average: %f   std dev: %f   range: %f\n", avg_per_core, sigma, range);
}

void create_MPI_Struct(MPI_Datatype *t) {
  MPI_Datatype types[STRUCT_LEN] = {MPI_UNSIGNED, MPI_DOUBLE};
  int blocklen[STRUCT_LEN] = {1, 1};
  MPI_Aint disp[STRUCT_LEN];

  workload w;
  disp[0] = (void *)&w.i - (void *)&w;
  disp[1] = (void *)&w.f - (void *)&w;

  MPI_Type_create_struct(STRUCT_LEN, blocklen, disp, types, t);
  MPI_Type_commit(t);
}
