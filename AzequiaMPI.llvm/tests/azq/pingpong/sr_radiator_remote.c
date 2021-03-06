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
#define  ITERA         100000

/*----------------------------------------------------------------*
 *   Implementation of private functions                          *
 *----------------------------------------------------------------*/
double abstime (void) {

  struct timespec t;

  clock_gettime(CLOCK_REALTIME, &t);

  return ((double) t.tv_sec + 1.0e-9 * (double) t.tv_nsec);
}


int do_sender (int bufSize) {

  int      myid;
  int      gix;
  int      itr  = 0;
  int      i;
  Addr     dst;
  int      excpn;
  double   now_1;
  double   now_0;
  int     *value;

  myid = getRank();
  gix  = getGroup();
#ifdef __DEBUG
  fprintf(stdout, "Sender(%x). [%d, %d]\n", (unsigned int)(THR_self()), getGroup(), myid);
#endif

  now_0 = abstime();

  /* Get the memory for the data to be sent */
  if(NULL == (value = (int *)malloc(bufSize))) {
    fprintf(stdout, "::::::::::::::::: Bug Node: Malloc Exception %d ::::::::::::::::\n", -5);
    return(-5);
  }

  for(i = 0; i < bufSize / sizeof(int); i++) {
    value[i] = i;
  }

  /* Configure destination address */
  dst.Group = getGroup();
  dst.Rank  = 0;

  while (1) {
#ifdef __DEBUG
    if(itr % PRINT_PERIOD == 0)  fprintf(stdout, "\t\t= S %d ============== itr [%d]  ...\n", myid, itr);
#endif

    for(i = 0; i < bufSize / sizeof(int); i++)
      value[i] = myid;

    
    if(0 > (excpn = timed_send(&dst, (char *)&value[0], bufSize, 99, 0, COM_FOREVER))) {
      fprintf(stderr, "\n:: S :::::::::: Sender(%x). [%d  %d]: Exception %d :::::::::: n", (int)(THR_self()), gix, myid, excpn);
      free(value);
      return(excpn);
    }

    //usleep(1000);

    if (++itr == ITERA)
      break;
  }

  if(value)
    free(value);

  now_1 = abstime();

#ifdef __DEBUG
  //fprintf(stdout, "\nmilliseconds %lf\n", (now_1 - now_0));
  fprintf(stdout, "Sender(%x). [%d, %d].  BYE\n", (unsigned int)(THR_self()), getGroup(), myid);
#endif

  return 0;
}




int do_receiver (int bufSize) {
    
  Status     status;
  int        myid;
  int        gix;
  int        itr = 0;
  double     now_1;
  double     now_0;
  int       *buf;
  Addr       src;
  int        excpn;
  int        i;
  int        numprocs;


  myid = getRank();
  gix  = getGroup();
  GRP_getSize(getGroup(), &numprocs);
#ifdef __DEBUG
  fprintf(stdout, "Receiver(%x). [%d, %d]\n", (unsigned int)(THR_self()), getGroup(), myid);
#endif

  now_0 = abstime();

  if(NULL == (buf = (int *)malloc(bufSize))) {
    fprintf(stdout, "::::::::::::::::: Bug Node: Malloc Exception %d ::::::::::::::::\n", -5);
    return(-5);
  }


  /* Configure source address */
  src.Group = getGroup();
  src.Rank  = ADDR_RNK_ANY;

  while (1) {

#ifdef __DEBUG
    if(itr % PRINT_PERIOD == 0)  fprintf(stdout, "\t\t= R %d ============== itr [%d]  ...\n", myid, itr);
#endif

  for(i = 0; i < bufSize / sizeof(int); i++) {
    buf[i] = -1;
  }

  //usleep(10000);
  if(0 > (excpn = timed_recv(&src, (char *)buf, bufSize, 99, &status, COM_FOREVER))) {
    fprintf(stderr, "\n:: R :::::::::: Receiver(%x). [%d  %d]: Exception %d :::::::::: n", (int)(THR_self()), gix, myid, excpn);
    return(excpn);
  }
      
#ifdef __CHECK
    /* ---------- 2. Run the ALGORITHM --------- */
    for(i = 0; i < bufSize / sizeof(int); i++) {
      if(buf[i] != status.Src.Rank) {
        fprintf(stderr, "\n :::::::::: Radiator (%x). USR %d Fail!!. Itr = %d/Value = %d :::::::::: \n", (int)THR_self(), i, itr, buf[i]);
        exit(1);
      }
    }
#endif

    if (++itr == (ITERA * (numprocs - 1)))
      break;
    
  }
  now_1 = abstime();
#ifdef __DEBUG
  fprintf(stdout, "\nmilliseconds %lf\n", (now_1 - now_0));
  fprintf(stdout, "Receiver(%x). [%d, %d].  BYE\n", (unsigned int)(THR_self()), getGroup(), myid);
#endif

  return(0);
}



int node_main (int argc, char *argv[]) {

  int   myid;
  int   numprocs;
  int   bufsize;

  GRP_getSize(getGroup(), &numprocs);
  myid = getRank();

  bufsize = atoi(argv[1]);

  srand(83247134);

#ifdef __DEBUG
  fprintf(stdout, "[%d , %d] \n", getGroup(), myid);
#endif

  if (myid != 0)  do_sender(bufsize);
  else            do_receiver(bufsize);

  return 0;
}

