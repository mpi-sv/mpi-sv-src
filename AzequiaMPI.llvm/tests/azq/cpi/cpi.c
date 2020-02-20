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
#define  PRINT_PERIOD  100
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

  double    func[MESS_SIZE];
  double    h, x, pi;
  double    PI25DT      = 3.141592653589793238462643;
  int       i, j, grpSize,
            precision,
            gix         = getGroup();
  int       myRank      = getRank();
  double    t_start = 0.0, t_end = 0.0;
  Addr      dst, src;  
  int       excpn;
  int       itr = 0;
  Rqst     *rqst = NULL;

  //fprintf(stdout, "Pi Operator: [%d  %d] (0x%x) \n", gix, myRank, (int)THR_self());

  if (argc != 3) {
    fprintf(stderr, "\nUse:  cpi  precision  num_ranks\n\n");
    exit(1);
  }

  precision = atoi(argv[1]);
  GRP_getSize(gix, &grpSize);

  if (precision < grpSize) {
    precision = grpSize;
    if (myRank == 0) 
      fprintf(stdout, "Precision must equal or greater than %d (size of group)\n", grpSize);
  }

  if (myRank == 0) 
    fprintf(stdout, "Running CPI with precision %d and group size: %d\n", precision, grpSize);

  double   *otherFunc [grpSize]; 
  Status    status    [grpSize];
  Rqst_t    all_rqst  [grpSize];  

  if (myRank == 0) {

    for (j = 1; j < grpSize; j++) otherFunc[j] = (double *) malloc (sizeof(double) * MESS_SIZE);
    otherFunc[0] = NULL;

    if (NULL == (rqst = (Rqst_t) malloc (sizeof(Rqst) * grpSize))) {
      fprintf(stderr, " :::::::::: CPI (%x) (R): Malloc Exception %d :::::::::: free ... ", (int)THR_self(), -6);
      return(-6);
    }

  }
  

  dst.Group = getGroup();
  dst.Rank  = 0; 
  src.Group = getGroup();

  if (myRank == 0) 
    t_start = abstime();

  all_rqst[0] = NULL;
  for(j = 1; j < grpSize; j++) {
    all_rqst[j] = &rqst[j];
  }

  h   = 1.0 / (double) precision;
  pi  = 0.0;
  for (i = myRank + 1; i <= precision; i += grpSize) {
    x = h * ((double) i - 0.5);
    func[0] = f(x);
    if(myRank != 0) {

      if (0 > send(&dst, (char *)func, sizeof(double) * MESS_SIZE, 99))        {excpn = -1; goto exception;}

    } else {

      pi += func[0];
      for(j = 1; j < grpSize; j++) {
	src.Rank = j;
	if (0 > arecv(&src, (char *)otherFunc[j], sizeof(double) * MESS_SIZE, 99, all_rqst[j]))    
                                                                               {excpn = -2; goto exception;}
      }
      waitall(all_rqst, grpSize, status);

      for(j = 1; j < grpSize; j++)
        pi += otherFunc[j][0];
    
      if(itr % PRINT_PERIOD == 0) {
        time_t result;
        result = time(NULL);
        fprintf(stdout, "\t\t= R [%d %d] itr [%d] \ttime: %s", gix, myRank, itr, asctime(localtime(&result)));
        fflush(stdout);
      }
      itr++;

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
