/*extern int   send       (const Addr_t dst, char     *buf,
                                           int       cnt,
                                           int       mode,
                                           int       tag,
                                           Rqst     *rqst,
                                           unsigned  timeout);
extern int   recv       (const Addr_t src, char     *buf,
                                           int       cnt,
                                           int       mode,
                                           int       tag,
                                           Rqst     *rqst,
                                           unsigned  timeout,
                                           Status   *status);*/

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
int Pi(int *param)
{
  double           myPi, otherPi, pi, h, sum, x;
  double           PI25DT      = 3.141592653589793238462643;
  int              excpn       = 0;
  int              i, grpSize,
                   precision   = param[0], 
                   pulsos      = param[1],
                   gix         = getGroup();
  int              myRank      = getRank();
  static char     *where       = "Pi";

  fprintf(stdout, "\nPi Operator: Rank %d, Precision %d, Pulsos %d\n", myRank, precision, pulsos);
  if (precision == 0) {
    excpn = -1;
    goto exception;
  }
  GRP_getSize(gix, &grpSize);

  h = 1.0 / (double) precision;
  sum = 0.0;
  for (i = myRank + 1; i <=precision; i += grpSize) {
    x = h * ((double) i - 0.5);
    sum += f(x);
  }
  myPi = h * sum;

  if(myRank != 0) {
    if(0 > (excpn = GC_send((char *)&myPi, sizeof(double), 0, 0, 0)))          goto exception;
  }
  else {
    pi = myPi;
    for(i = 1; i < grpSize; i++) {
      if(0 > (excpn = GC_recv((char *)&otherPi, sizeof(double), i, 0, 0, 0)))  goto exception;
      pi += otherPi;
    }  
    fprintf(stdout, "\nPi = %.16lf. Error = %.16lf\n", pi, PI25DT - pi);  
  }
  return(0);

exception:
  fprintf(stdout, "::::::::::::::::: Pi Operator: Exception %d ::::::::::::::::\n", excpn);
  return(excpn);
}
