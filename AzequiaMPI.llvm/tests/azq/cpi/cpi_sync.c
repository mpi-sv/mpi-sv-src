/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   used by this module                                          *
 *----------------------------------------------------------------*/
#ifdef __OSI
#include <osi.h>
#endif
#include <azq.h>
#include <azq_types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/types.h>
#include <time.h>

/*----------------------------------------------------------------*
 *   Definition of private data                                   *
 *----------------------------------------------------------------*/
#define  PRINT_PERIOD  10000
#define  MESS_SIZE     1500

/*----------------------------------------------------------------*
 *   Implementation of private functions                          *
 *----------------------------------------------------------------*/
double f(double a) {
  return (4.0 / (1.0 + a * a));
}

double abstime (void) {

  struct timespec t;

  clock_gettime(CLOCK_REALTIME, &t);

  return ((double) t.tv_sec + 1.0e-9 * (double) t.tv_nsec);
}


int node_main(int argc, char *argv[]) {

  double    otherFunc[MESS_SIZE], func[MESS_SIZE];
  double    h, x, pi;
  double    PI25DT      = 3.141592653589793238462643;
  int       i, j, grpSize,
            precision,
            gix         = getGroup();
  int       myRank      = getRank();
  double    t_start = 0.0, t_end = 0.0;
  Addr      dst, src;
  Status    status;
  int       excpn;
  int       itr = 0;

  //fprintf(stdout, "Pi Operator: [%d  %d] (0x%x) \n", gix, myRank, (int)THR_self());

  if (argc != 3) {
    fprintf(stderr, "\nUse:  cpi  precision  num_ranks\n\n");
    exit(1);
  }

  precision = atoi(argv[1]);

  GRP_getSize(gix, &grpSize);

  dst.Group = getGroup();
  dst.Rank  = 0; 
  src.Group = getGroup();

  if (myRank == 0) 
    t_start = abstime();

  h   = 1.0 / (double) precision;
  pi  = 0.0;
  for (i = myRank + 1; i <= precision; i += grpSize) {
    x = h * ((double) i - 0.5);
    func[0] = f(x);
    if(myRank != 0) {
      if (0 > send(&dst, (char *)func, sizeof(double) * MESS_SIZE, 99))        {excpn = -1; goto exception;}
    }
    else {
      pi += func[0];
      for(j = 1; j < grpSize; j++) {
	src.Rank = j;
	if (0 > recv(&src, (char *)otherFunc, sizeof(double) * MESS_SIZE, 99, &status))    
                                                                               {excpn = -2; goto exception;}
        pi += otherFunc[0];

        if(itr % PRINT_PERIOD == 0) {
          time_t result;
          result = time(NULL);
          fprintf(stdout, "\t\t= R [%d %d] itr [%d] \ttime: %s\n", gix, myRank, itr, asctime(localtime(&result)));
          fflush(stdout);
        }
        itr++;

      } 
    }
  }
  if(myRank == 0) {
    pi *= h;
    t_end = abstime();
    fprintf(stdout, "[%d  %d] ::::  Pi    = %.16lf   Error = %.16lf   Time:   %lf\n", gix, myRank, pi, PI25DT - pi, t_end - t_start); 
    fflush(stdout);
  }

  return(0);

exception:
  fprintf(stdout, "::::::::::::::::: Pi Operator %d: Exception %d ::::::::::::::::\n", myRank, excpn);
  return(excpn);
}
