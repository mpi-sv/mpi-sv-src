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


/*----------------------------------------------------------------*
 *   Definition of private constants                              *
 *----------------------------------------------------------------*/
#define  PRINT_PERIOD  1000
#define  ITERA         1000

/*----------------------------------------------------------------*
 *   Implementation of private functions                          *
 *----------------------------------------------------------------*/
double abstime (void) {

  struct timespec t;

  clock_gettime(CLOCK_REALTIME, &t);

  return ((double) t.tv_sec + 1.0e-9 * (double) t.tv_nsec);
}


#define MAX_PEND_RQST  2

int do_sender (int bufSize) {

  int      myid;
  int      gix;
  int      itr  = 0;
  Addr     dst;
  int      excpn;
  double   now_1,t0,t1;
  double   now_0;
  int     *buf0;
  int     *buf1;
  int      i;

  myid = getRank();
  gix  = getGroup();

#ifdef __DEBUG
  fprintf(stdout, "Sender(%x). [%d, %d]\n", (unsigned int)(THR_self()), getGroup(), myid);
#endif

  now_0 = abstime();

  fprintf(stdout, "\n(TX) BUFSIZE: %d\n", bufSize);

  /* Get the memory for the data to be sent */
  if(NULL == (buf0 = (int *)malloc(bufSize))) {
    fprintf(stdout, "::::::::::::::::: Bug Node: Malloc Exception %d ::::::::::::::::\n", -5);
    return(-5);
  }
  if(NULL == (buf1 = (int *)malloc(bufSize))) {
    fprintf(stdout, "::::::::::::::::: Bug Node: Malloc Exception %d ::::::::::::::::\n", -5);
    return(-5);
  }


  for(i = 0; i < bufSize / sizeof(int); i++) {
    buf0[i] = 0;
    buf1[i] = 0;
  }

  /* Configure destination address */
  dst.Group = getGroup();
  dst.Rank  = myid - 1;

  while (1) {

    if(itr % PRINT_PERIOD == 0)  fprintf(stdout, "\t\t= S [%d %d] ============== itr [%d]  ...\n", gix, myid, itr);
    fflush(stdout);

    t0 = abstime();

    if(0 > (excpn = send(&dst, (char *)&buf0[0], bufSize, 99))) {
      fprintf(stderr, "\n:: S :::::::::: Sender(%x). [%d  %d]: Exception %d :::::::::: n", (int)(THR_self()), gix, myid, excpn);
      free(buf0);
      return(excpn);
    }

    t1 = abstime();
    printf("TX: %d -- %lf\n\n", itr, t1-t0);




    t0 = abstime();

    if(0 > (excpn = recv(&dst, (char *)&buf1[0], bufSize, 99, NULL))) {
      fprintf(stderr, "\n:: S :::::::::: Sender(%x). [%d  %d]: Exception %d :::::::::: n", (int)(THR_self()), gix, myid, excpn);
      free(buf1);
      return(excpn);
    }

    t1 = abstime();


    //if (!(itr % 100))
    printf("RX: %d -- %lf\n", itr, t1-t0);





    itr++;
    if (itr == ITERA)
      break;
  }

  if(buf0) free(buf0);
  if(buf1) free(buf1);

  now_1 = abstime();

#ifdef __DEBUG
  fprintf(stdout, "\nmilliseconds %lf\n", (now_1 - now_0));
  fprintf(stdout, "Sender(%x). [%d, %d].  BYE\n", (unsigned int)(THR_self()), getGroup(), myid);
#endif

  return 0;
}




int do_receiver (int bufSize) {
    
  int        myid;
  int        gix;
  int        itr = 0;
  double     now_1, t0,t1;
  double     now_0;
  int       *buf0,*buf1;
  Addr       src;
  int        excpn;
  int        tag;

  myid = getRank();
  gix  = getGroup();

  fprintf(stdout, "(RX) BUFSIZE: %d\n", bufSize);

#ifdef __DEBUG
  fprintf(stdout, "Receiver(%x). [%d, %d]\n", (unsigned int)(THR_self()), getGroup(), myid);
#endif

  now_0 = abstime();

  if(NULL == (buf0 = (int *)malloc(bufSize))) {
    fprintf(stdout, "::::::::::::::::: Bug Node: Malloc Exception %d ::::::::::::::::\n", -5);
    return(-5);
  }

  if(NULL == (buf1 = (int *)malloc(bufSize))) {
    fprintf(stdout, "::::::::::::::::: Bug Node: Malloc Exception %d ::::::::::::::::\n", -5);
    return(-5);
  }

  /* Configure source address */
  src.Group = getGroup();
  src.Rank  = myid + 1;

  while (1) {

    if(itr % PRINT_PERIOD == 0)  fprintf(stdout, "\t\t= R [%d %d] ============== itr [%d]  ...\n", gix, myid, itr);
    fflush(stdout);

    t0 = abstime();

    tag = 99;
    if(0 > (excpn = recv(&src, (char *)buf0, bufSize, tag, NULL))) {
      fprintf(stderr, "\n:: R :::::::::: Receiver(%x). [%d  %d]: Exception %d :::::::::: n", (int)(THR_self()), gix, myid, excpn);
      return(excpn);
    }

    t1 = abstime();
    printf("RX: %d -- %lf\n\n", itr, t1-t0);






    t0 = abstime();


    if(0 > (excpn = send(&src, (char *)buf1, bufSize, tag))) {
      fprintf(stderr, "\n:: R :::::::::: Receiver(%x). [%d  %d]: Exception %d :::::::::: n", (int)(THR_self()), gix, myid, excpn);
      return(excpn);
    }


    t1 = abstime();


    //if (!(itr % 100)) 
    printf("TX: %d -- %lf\n\n", itr, t1-t0);




    if (++itr == ITERA)
      break;
    
  }
  now_1 = abstime();

  time_t result;
  result = time(NULL);

  fprintf(stdout, "\nmilliseconds %lf\n", (now_1 - now_0));
  fprintf(stdout, "Receiver(%x). [%d, %d].  time: %s  -  BYE\n", (unsigned int)(THR_self()), getGroup(), myid, asctime(localtime(&result)));

  return(0);
}



int node_main (int argc, char *argv[]) {

  int   myid;
  int   numprocs;
  int   bufsize;

  GRP_getSize(getGroup(), &numprocs);
  myid = getRank();

  bufsize = atoi(argv[1]);

  srand(abstime());

  fprintf(stdout, "[%d , %d] \n", getGroup(), myid);

  if (myid % 2)  do_sender(bufsize);
  else           do_receiver(bufsize);

  return 0;
}

