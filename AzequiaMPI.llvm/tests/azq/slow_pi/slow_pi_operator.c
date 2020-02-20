/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   used by this module                                          *
 *----------------------------------------------------------------*/
#include <stdio.h>
#include <azq.h>

/*----------------------------------------------------------------*
 *   Definition of private data                                   *
 *----------------------------------------------------------------*/


/*----------------------------------------------------------------*
 *   Implementation of private functions                          *
 *----------------------------------------------------------------*/
double f(a)
double a;
{
	return (4.0 / (1.0 + a * a));
}


     /*----------------------------------------------------------*\
    |    Pi                                                        |
    |                                                              |
    |                                                              |
     \*----------------------------------------------------------*/
int slowPi(int *param)
{
  double           otherFunc, pi, h, x, func;
  double           PI25DT      = 3.141592653589793238462643;
  int              excpn       = 0;
  int              i, j, grpSize,
                   precision   = param[0],
                   gix         = getGroup();
  int              myRank      = getRank();
  static char     *where       = "Pi";

  fprintf(stdout, "\nPi: Rank %d, Precision %d\n", myRank, precision);
  if (precision == 0) {
    excpn = -1;
    goto exception;
  }
  GRP_getSize(gix, &grpSize);

  h   = 1.0 / (double) precision;
  pi  = 0.0;
  for (i = myRank + 1; i <=precision; i += grpSize) {
    x = h * ((double) i - 0.5);
    func = f(x);
    if(myRank != 0) {
      fprintf(stdout, "Pi: Rank %d, Send %d...\n", myRank, i);
      if(0 > (excpn = GC_send((char *)&func, sizeof(double), 0, 0, 0)))          goto exception;
    }
    else {
      pi += func;
      for(j = 1; j < grpSize; j++) {
        fprintf(stdout, "\tPi: Rank %d, Receive %d from Rank %d...\n", myRank, i, j);
        if(0 > (excpn = GC_recv((char *)&otherFunc, sizeof(double), j, 0, 0, 0)))  goto exception;
        pi += otherFunc;
      }  
    }
  }
  if(myRank == 0) {
    pi *= h;
    fprintf(stdout, "\nPi = %.16lf. Error = %.16lf\n", pi, PI25DT - pi);
  }
  return(0);

exception:
  fprintf(stdout, "::::::::::::::::: Pi Operator: Exception %d ::::::::::::::::\n", excpn);
  return(excpn);
}
