
#if defined(__OSI)
  #include <osi.h>
#else
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <time.h>
#endif

#include <azq.h>


#define  MAX_SIZE  (16384)
#define  INC_SIZE  256
#define  MAX_LIST  ((MAX_SIZE / INC_SIZE) + 1)
#define  NUM_MSGS  10000

#define WARMUP 1000


char statsfile[128];
double list_0[MAX_LIST];
double list_1[MAX_LIST];
double list_max[MAX_LIST];
double list_min[MAX_LIST];


double MPI_Wtime (void) {

  struct timespec t;

  clock_gettime(CLOCK_REALTIME, &t);

  return ((double) t.tv_sec + 1.0e-9 * (double) t.tv_nsec);
}


int latency(int *param) {

  int numprocs, myid;
  char *buf;
  Status status;
  double t_start, t_end, t_final;
  FILE *f;
  int i;
  int size;
  int msg;
  Addr dst;


  if (NULL == (buf = (char *) malloc (MAX_SIZE))) {
    perror ("Couldn't allocate data");
    exit (1);
  }

  myid= getRank();

  /* Warm up */
  if (myid == 0) {
    printf ("Warming up...\n");
    fflush (stdout);
  }

  for (size = 0; size <= MAX_SIZE; size += INC_SIZE) {
    if (myid == 0) {
      dst.Group = getGroup();
      dst.Rank = 1;
      for (i = 0; i < WARMUP; i++) {
        send(&dst, buf, size, 100);
        recv(&dst, buf, size, 101, &status);
      }
    }
    if (myid == 1) {
      dst.Group = getGroup();
      dst.Rank = 0;
      for (i = 0; i < WARMUP; i++) {
        recv(&dst, buf, size, 100, &status);
        send(&dst, buf, size, 101);
      }
    }
  }

  if (myid == 1) {
    for (i = 0; i < MAX_LIST; i++) {
      list_max[i] = 0;
      list_min[i] = 9999999;
    }
  }

  if (myid == 0) {
    printf ("Working ...\n");
    fflush (stdout);
  }

  if (myid == 1) {

    printf ("Writing data...\n");
    fflush (stdout);
    sprintf (statsfile, "%s.lat", "eval");
    if (NULL == (f = fopen (statsfile, "w"))) {
      perror ("syseval: Couldn't open stats file");
      exit (1);
    }
  }

 
  i = 0;
  for (size = 0; size <= MAX_SIZE; size += INC_SIZE) {

    if (myid == 0) {
      t_start = MPI_Wtime();
      for (msg = 0; msg < NUM_MSGS; msg++) {
                    send(&dst, buf, size, 0);
		    //MPI_Send (buf, size, MPI_BYTE, 1, 0, MPI_COMM_WORLD);
                    recv(&dst, buf, size, 1, &status);
		    //MPI_Recv (buf, size, MPI_BYTE, 1, 1, MPI_COMM_WORLD, &status);
      }
      t_end = MPI_Wtime ();
      list_0[i++] = t_end - t_start;
    }

    if (myid == 1) {
      t_start = MPI_Wtime();
      for (msg = 0; msg < NUM_MSGS; msg++) {
        //t_final = MPI_Wtime();
        recv(&dst, buf, size, 0, &status);
 		  //MPI_Recv (buf, size, MPI_BYTE, 0, 0, MPI_COMM_WORLD, &status);
        send(&dst, buf, size, 1);
		  //MPI_Send (buf, size, MPI_BYTE, 0, 1, MPI_COMM_WORLD);
        //t_final = MPI_Wtime() - t_final;
        //if (list_max[i] < t_final) list_max[i] = t_final;
        //if (list_min[i] > t_final) list_min[i] = t_final;
      }
      t_end = MPI_Wtime ();
      list_1[i++] = t_end - t_start;
    }
  }

  if (myid == 1) {
    for (i = 0; i < MAX_LIST; i++) {
	    //fprintf (f, "%d  %e\n", i * INC_SIZE, list_0[i]);
	    //fprintf (f, "%f\t%f\t%f\n", list_1[i], list_max[i], list_min[i]);
	    fprintf (f, "%d\t%f\n", i * INC_SIZE, list_1[i] * 100);
	    printf ("%d\t%f\n", i * INC_SIZE, list_1[i] * 100);
    }
    fclose (f);
  }

  //MPI_Finalize ();
  return 0;
}

