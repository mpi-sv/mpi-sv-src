#if defined(__OSI)
  #include <osi.h>
#else
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
#endif


#include "mpi.h"


#define SIZE          13
#define USER_BUFFER   (SIZE * 64)
#define NUMITER       1000
#define PRINT_MSG     1

char ALPHABET [25] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
                      'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
                      'u', 'v', 'x', 'y', 'z' };


int main (int argc, char **argv) {

  int             myid, numprocs;
  MPI_Status      status;
  int             i, k;
  char           *buf, *batt;
  int             cnt;
  double          tend, tstart;
  MPI_Request     req_send;
  MPI_Request     req_recv;


  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);

  printf("Process %d starting ... \n", myid);

  if (NULL == (buf = (char *)malloc(SIZE * sizeof(char)))) {
    perror("malloc");
    exit(-1);
  }

  if (myid == 0) {

    if (NULL == (batt = (char *)malloc(USER_BUFFER * sizeof(char) + 1))) {
      perror("malloc batt");
      exit(-1);
    }
    //batt++;

    MPI_Buffer_attach(batt, (USER_BUFFER * MPI_BSEND_OVERHEAD) + (USER_BUFFER * sizeof(char)));

    tstart = MPI_Wtime();
    MPI_Bsend_init(buf, SIZE, MPI_CHAR, 1, 82, MPI_COMM_WORLD, &req_send);

    for (k = 0; k < NUMITER; k++) {

      for (i = 0; i < SIZE; i++)
        buf[i] = ALPHABET[(k + i) % 25];

      MPI_Start(&req_send);
      MPI_Wait(&req_send, &status);

    }

    MPI_Request_free(&req_send);
    tend = MPI_Wtime();
    printf("TIME: %.4f\n", tend - tstart);

    MPI_Buffer_detach(&batt, &cnt);
    free(batt);

  } else if (myid == 1) {

    MPI_Recv_init(buf, SIZE, MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &req_recv);

    for (k = 0; k < NUMITER; k++) {

      for (i = 0; i < SIZE; i++)
        buf[i] = 0;

      MPI_Start(&req_recv);
      MPI_Wait(&req_recv, &status);

      for (i = 0; i < SIZE; i++) {
        if (buf[i] != ALPHABET[(k + i) % 25]) {
          printf("*** ERROR: data received incorrect (%d / %d)***\n", buf[i], ALPHABET[(k + i) % 25]);
          sleep(1);
          exit(-1);
        }
      }

      MPI_Get_count(&status, MPI_CHAR, &cnt);
      if (!(k % PRINT_MSG)) printf("(%d) Received correctly %d CHAR from %d with tag %d\n", k, cnt, status.MPI_SOURCE, status.MPI_TAG);
    }

    MPI_Request_free(&req_recv);
  }

  free(buf);

  MPI_Finalize();
  return(0);
}

