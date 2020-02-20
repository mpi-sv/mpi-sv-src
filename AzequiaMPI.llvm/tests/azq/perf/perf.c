/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   used by this module                                          *
 *----------------------------------------------------------------*/
#ifdef __OSI
#include <osi.h>
#endif
#include <stdio.h>
#include <azq.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sched.h>



/*----------------------------------------------------------------*
 *   Definition of private data                                   *
 *----------------------------------------------------------------*/
#define  MAX_SIZE      (1024 * 8)
#define  ITERATIONS    1000
#define  NUM_SAMPLES   128
#define  WARM_UP_ITER  (ITERATIONS / 100)
#define  SENDER        (getRank() % 2)
#define  RECEIVER      (!(getRank() % 2))
#define  INCR          (MAX_SIZE / NUM_SAMPLES)

#define  _DEBUG
#define  LINEAR
#define  BW
#define  MOD            (ITERATIONS / 100)
#define  ONE_PROCESSOR  0   /* 0 for not funning on 1 proc, 1 for running it on 1 proc. too */


void pingping(int size, void *s_buffer, void *r_buffer, int iterations, double *time);
void pingpong(int size, void *s_buffer, void *r_buffer, int iterations, double *time);
double Wtime (void);

/*----------------------------------------------------------------*
 *   Definition of functions                                      *
 *----------------------------------------------------------------*/
int operator_main(int argc, char **argv) {

  int      myid;
  int      numprocs;
  Addr     src, dst;
  char    *s_buffer, *r_buffer;
  double   time_sender, time, time_final;
  int      size;
  double   bw[NUM_SAMPLES + 1];
  int      i = 0, j, k;

#ifdef  ONE_PROCESSOR
  unsigned int cpuaff;
#endif

  myid = getRank();
  GRP_getSize(getGroup(), &numprocs);

  if (getRank() == 0) {
    size = 0;
    fprintf(stdout, "SIZES\n");
    while (size <= MAX_SIZE) {
      fprintf(stdout, "%d\n", size);
#ifdef LINEAR
      size += INCR;
#else
      if (size == 0)  size = 1;
      else size <<= 1;
#endif
    }
  }

  usleep(100);

  for (k = 0; k <= ONE_PROCESSOR; k++) {

    i = 0;

    if (k == ONE_PROCESSOR) {
      if (RECEIVER) fprintf(stdout, "\nOne processor test\n");
      cpuaff = 0x0001;
      if (0 > sched_setaffinity(0, 4, &cpuaff)) {
        fprintf(stdout, "ERROR: sched_setaffinity\n");
        exit(1);
      }
    } else {
      if (RECEIVER) fprintf(stdout, "\nSeveral processor test\n");
    } 

    /* 1. PING */
    if (RECEIVER) fprintf(stdout, "\nPING\n");
    size = 0;
    while (size <= MAX_SIZE) {
      s_buffer = (char *)malloc(size);
      r_buffer = (char *)malloc(size);

      /* Warm up = 10 iterations */
      pingping(size, s_buffer, r_buffer, WARM_UP_ITER, &time);

      /* Test */
      pingping(size, s_buffer, r_buffer, ITERATIONS, &time);

      if (RECEIVER) {
        src.Group = getGroup();
        src.Rank  = myid + 1;
        recv(&src, (char *)&time_sender, sizeof(double), 99, NULL);
#ifdef _DEBUG
        fprintf(stdout, "\n[RECEIVER %d].  Time of sender  %lf  and mine  %lf\n", myid, time, time_sender);
#endif
        time_final = (time + time_sender) / 2.0;
        fprintf(stdout, "%lf\n", time_final);
        bw[i++] = ((double)size / time_final) * ((double)ITERATIONS / (double)(1024 * 1024));
      } else {
        dst.Group = getGroup();
        dst.Rank = myid - 1;
        send(&dst, (char *)&time, sizeof(double), 99);
      }

#ifdef LINEAR
      size += INCR;
#else
      if (size == 0)  size = 1;
      else size <<= 1;
#endif

      free(s_buffer);
      free(r_buffer);
    }

#ifdef BW
    if (RECEIVER) {
      fprintf(stdout, "\nBandwidth\n");
      for (j = 0; j < (i - 1); j++)
        fprintf(stdout, "%.2lf\n", bw[j]);
    }
#endif
  
    if (RECEIVER) fprintf(stdout, "\nPING-PONG\n");

    /* 1. PING-PONG */
    i = 0;
    size = 0;
    while (size <= MAX_SIZE) {
      s_buffer = (char *)malloc(size);
      r_buffer = (char *)malloc(size);

      /* Warm up = 10 iterations */
      pingpong(size, s_buffer, r_buffer, WARM_UP_ITER, &time);

      /* Test */
      pingpong(size, s_buffer, r_buffer, ITERATIONS, &time);

      if (RECEIVER) {
        src.Group = getGroup();
        src.Rank  = myid + 1;
        recv(&src, (char *)&time_sender, sizeof(double), 99, NULL);
#ifdef _DEBUG
        fprintf(stdout, "\n[RECEIVER %d].  Time of sender  %lf  and mine  %lf\n", myid, time, time_sender);
#endif
        time_final = (time + time_sender) / 2.0;
        fprintf(stdout, "%lf\n", time_final);
        bw[i++] = ((double)(size * ITERATIONS) / time_final) / (double)(1024 * 1024);
      } else {
        dst.Group = getGroup();
        dst.Rank = myid - 1;
        send(&dst, (char *)&time, sizeof(double), 99);
      }

#ifdef LINEAR
      size += INCR;
#else
      if (size == 0)  size = 1;
      else size <<= 1;
#endif

      free(s_buffer);
      free(r_buffer);
    }

#ifdef BW
    if (RECEIVER) {
      fprintf(stdout, "\nBandwidth\n");
      for (j = 0; j < (i - 1); j++)
        fprintf(stdout, "%.2lf\n", bw[j]);
    }
#endif
  
  }

  return EXIT_SUCCESS;
}


void pingping(int size, void *s_buffer, void *r_buffer, int iterations, double *time) {

  double t1, t2;
  int    i;
  int rank;
  int s_tag, r_tag;
  Addr dst, src;

  rank = getRank();
  s_tag = 15;
  r_tag = 15;

#ifdef _DEBUG
  fprintf(stdout, "PING.  Sending %d messages of %d size\n", iterations, size);
#endif

  if (SENDER) {
    dst.Group = getGroup();
    dst.Rank = rank - 1;
      
    t1 = Wtime();
    for(i = 0; i < iterations; i++) {
      send(&dst, (char *)s_buffer, size, s_tag);
#ifdef _DEBUG
      if (!(i % MOD)) fprintf(stdout, "[SENDER %d].  Sending message %d \n", rank, i);
#endif
    }
    t2 = Wtime();
    *time = (t2 - t1);

  } else {

    src.Group = getGroup();
    src.Rank = rank + 1;

    t1 = Wtime();
    for(i = 0; i < iterations; i++) {
      recv(&src, (char *)r_buffer, size, r_tag, NULL);
#ifdef _DEBUG
      if (!(i % MOD)) fprintf(stdout, "[RECEIVER %d].  Receiving message %d \n", rank, i);
#endif
    }
    t2 = Wtime();
    *time = (t2 - t1);
  }
}


void pingpong(int size, void *s_buffer, void *r_buffer, int iterations, double *time) {

  double t1, t2;
  int    i;
  int rank;
  int s_tag, r_tag;
  Addr dst, src;

  rank = getRank();
  s_tag = 15;
  r_tag = 15;

#ifdef _DEBUG
  fprintf(stdout, "PING.  Sending %d messages of %d size\n", iterations, size);
#endif

  if (SENDER) {
    dst.Group = getGroup();
    dst.Rank = rank - 1;
    src.Group = getGroup();
    src.Rank = rank - 1;
      
    t1 = Wtime();
    for(i = 0; i < iterations; i++) {
      send(&dst, (char *)s_buffer, size, s_tag);
#ifdef _DEBUG
      if (!(i % MOD)) fprintf(stdout, "[SENDER %d].  Sending message %d \n", rank, i);
#endif
      recv(&src, (char *)r_buffer, size, r_tag, NULL);
    }
    t2 = Wtime();
    *time = (t2 - t1);

  } else {

    src.Group = getGroup();
    src.Rank = rank + 1;
    dst.Group = getGroup();
    dst.Rank = rank + 1;

    t1 = Wtime();
    for(i = 0; i < iterations; i++) {
      recv(&src, (char *)r_buffer, size, r_tag, NULL);
#ifdef _DEBUG
      if (!(i % MOD)) fprintf(stdout, "[RECEIVER %d].  Receiving message %d \n", rank, i);
#endif
      send(&dst, (char *)s_buffer, size, s_tag);
    }
    t2 = Wtime();
    *time = (t2 - t1);
  }
}



double Wtime (void) {

  struct timespec t;

  clock_gettime(CLOCK_REALTIME, &t);

  return ((double) t.tv_sec + 1.0e-9 * (double) t.tv_nsec);
}
