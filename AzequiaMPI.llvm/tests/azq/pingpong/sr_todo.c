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
//#define  PRINT_PERIOD  100000000
//#define  ITERA         100000000

#define  PRINT_PERIOD  10000
#define  ITERA         100000

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
  int      i;
  Addr     dst;
  int      excpn;
  double   now_1;
  double   now_0;
  int     *buf0;
  int     *buf1;
  Rqst     rqst[MAX_PEND_RQST];
  Rqst_t   rqst_ptr[MAX_PEND_RQST];
  Rqst     req;
  Rqst_t   req_ptr;
  Rqst     rqst_per;
  Rqst_t   rqst_ptent;
  int      sz;


  myid = getRank();
  gix  = getGroup();
#ifdef __DEBUG
  fprintf(stdout, "Sender(%x). [%d, %d]\n", (unsigned int)(THR_self()), getGroup(), myid);
#endif

  now_0 = abstime();

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

  /* Persistent request */
  rqst_ptent = &rqst_per;
  psend_init(&dst, (char *)buf0, bufSize, 99, rqst_ptent);


  while (1) {
#ifdef __DEBUG
    if(itr % PRINT_PERIOD == 0)  fprintf(stdout, "\t\t= S [%d %d] ============== itr [%d]  ...\n", gix, myid, itr);
    fflush(stdout);
#endif
/*
    for(i = 0; i < bufSize / sizeof(int); i++) {
      buf0[i] = itr;
      buf1[i] = itr + 1;
    }
*/
/*
    rqst_ptr[0] = &rqst[0];
    if(0 > (excpn = asend(&dst, (char *)&buf0[0], bufSize, 99, rqst_ptr[0]))) {
      fprintf(stderr, "\n:: S :::::::::: Sender(%x). [%d  %d]: Exception %d :::::::::: n", (int)(THR_self()), gix, myid, excpn);
      free(buf0);
      return(excpn);
    }

    rqst_ptr[1] = &rqst[1];
    if(0 > (excpn = asend(&dst, (char *)&buf1[0], bufSize, 99, rqst_ptr[1]))) {
      fprintf(stderr, "\n:: S :::::::::: Sender(%x). [%d  %d]: Exception %d :::::::::: n", (int)(THR_self()), gix, myid, excpn);
      free(buf1); free(buf0);
      return(excpn);
    }

    if(0 > (excpn = timed_wait(&rqst_ptr[1], AZQ_STATUS_IGNORE, COM_FOREVER))) {
      fprintf(stderr, "\n:: S :::::::::: Sender(%x). [%d  %d]: Exception %d :::::::::: n", (int)(THR_self()), gix, myid, excpn);
      free(buf1); free(buf0);
      return(excpn);
    }

    if(0 > (excpn = timed_wait(&rqst_ptr[0], AZQ_STATUS_IGNORE, COM_FOREVER))) {
      fprintf(stderr, "\n:: S :::::::::: Sender(%x). [%d  %d]: Exception %d :::::::::: n", (int)(THR_self()), gix, myid, excpn);
      free(buf1); free(buf0);
      return(excpn);
    }
*/
/*
    req_ptr = &req;
    if(0 > (excpn = asend(&dst, (char *)&buf0[0], bufSize, 99, req_ptr))) {
      fprintf(stderr, "\n:: S :::::::::: Sender(%x). [%d  %d]: Exception %d :::::::::: n", (int)(THR_self()), gix, myid, excpn);
      free(buf0);
      return(excpn);
    }

    if(0 > (excpn = send(&dst, (char *)&buf1[0], bufSize, 99))) {
      fprintf(stderr, "\n:: S :::::::::: Sender(%x). [%d  %d]: Exception %d :::::::::: n", (int)(THR_self()), gix, myid, excpn);
      free(buf1);
      return(excpn);
    }

    if(0 > (excpn = timed_wait(&req_ptr, AZQ_STATUS_IGNORE, COM_FOREVER))) {
      fprintf(stderr, "\n:: S :::::::::: Sender(%x). [%d  %d]: Exception %d :::::::::: n", (int)(THR_self()), gix, myid, excpn);
      free(buf0);
      return(excpn);
    }
*/

/*
    if(0 > (excpn = send(&dst, (char *)&buf0[0], bufSize, 99))) {
      fprintf(stderr, "\n:: S :::::::::: Sender(%x). [%d  %d]: Exception %d :::::::::: n", (int)(THR_self()), gix, myid, excpn);
      free(buf0);
      return(excpn);
    }


/*
    if(0 > (excpn = psend_start(rqst_ptent))) {
      fprintf(stderr, "\n:: R :::::::::: Sender(%x). [%d  %d]: Exception %d in psend :::::::::: \n", (int)(THR_self()), gix, myid, excpn);
      return(excpn);
    }

    if(0 > (excpn = wait(&rqst_ptent, AZQ_STATUS_IGNORE)))  {
      fprintf(stderr, "\n:: R :::::::::: Sender(%x). [%d  %d]: Exception %d in psend-wait :::::::::: \n", (int)(THR_self()), gix, myid, excpn);
      return(excpn);
    }
*/

/*
    do {
      sz = rand() % bufSize;
    } while ((sz < sizeof(int)) || (sz > (bufSize - sizeof(int))));

    for(i = 0; i < sz / sizeof(int); i++) {
      buf0[i] = itr;
    }
    buf0[i] = 0;
    fprintf(stdout, "\n:: S ooooooooo Sender(%x). [%d  %d]: Msg of %d :::::::::: \n", (int)(THR_self()), gix, myid, sz);
    if(0 > (excpn = send(&dst, (char *)&buf0[0], sz + sizeof(int), 99))) {
      fprintf(stderr, "\n:: S :::::::::: Sender(%x). [%d  %d]: Exception %d :::::::::: \n", (int)(THR_self()), gix, myid, excpn);
      free(buf0);
      return(excpn);
    }
*/

/*
    do {
      sz = rand() % bufSize;
    } while ((sz < sizeof(int)) || (sz > (bufSize - sizeof(int))));

    sz >>= 2;
    sz <<= 2;

    for(i = 0; i < sz / sizeof(int); i++) {
      buf0[i] = itr;
    }
    buf0[i - 1] = 0;
*/
    //fprintf(stdout, "\n:: S ============ Sender(%x). [%d  %d]: Size: %d :::::::::: n", (int)(THR_self()), gix, myid, sz);
    if(0 > (excpn = send(&dst, (char *)&buf0[0], bufSize, 99))) {
      fprintf(stderr, "\n:: S :::::::::: Sender(%x). [%d  %d]: Exception %d :::::::::: n", (int)(THR_self()), gix, myid, excpn);
      free(buf0);
      return(excpn);
    }

    //itr += MAX_PEND_RQST;
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
  int        tag;
  Rqst       req;
  Rqst_t     req_ptr;
  Rqst       rqst_per;
  Rqst_t     rqst_ptent;
  int        cnt;


  myid = getRank();
  gix  = getGroup();
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
  src.Rank  = myid + 1;

  /* Persistent request */
  tag = 99;
  rqst_ptent = &rqst_per;
  precv_init(&src, (char *)buf, bufSize, tag, rqst_ptent);


  while (1) {

#ifdef __DEBUG
    if(itr % PRINT_PERIOD == 0)  fprintf(stdout, "\t\t= R [%d %d] ============== itr [%d]  ...\n", gix, myid, itr);
    fflush(stdout);
#endif

/*
  for(i = 0; i < bufSize / sizeof(int); i++) {
    buf[i] = -1;
  }
*/

/*
  tag = 99;
  if(0 > (excpn = timed_recv(&src, (char *)buf, bufSize, tag, &status, COM_FOREVER))) {
    fprintf(stderr, "\n:: R :::::::::: Receiver(%x). [%d  %d]: Exception %d :::::::::: n", (int)(THR_self()), gix, myid, excpn);
    return(excpn);
  }
*/

/*
  tag = 99;
  req_ptr = &req;
  if(0 > (excpn = arecv(&src, (char *)buf, bufSize, tag, req_ptr))) {
    fprintf(stderr, "\n:: S :::::::::: Sender(%x). [%d  %d]: Exception %d :::::::::: n", (int)(THR_self()), gix, myid, excpn);
    free(buf);
    return(excpn);
  }

  if(0 > (excpn = timed_wait(&req_ptr, &status, COM_FOREVER))) {
    fprintf(stderr, "\n:: S :::::::::: Sender(%x). [%d  %d]: Exception %d :::::::::: n", (int)(THR_self()), gix, myid, excpn);
    free(buf);
    return(excpn);
  }
*/

/*
  if(0 > (excpn = precv_start(rqst_ptent))) {
    fprintf(stderr, "\n:: R :::::::::: Receiver(%x). [%d  %d]: Exception %d in precv :::::::::: n", (int)(THR_self()), gix, myid, excpn);
    return(excpn);
  }

  if(0 > (excpn = wait(&rqst_ptent, &status)))  {
    fprintf(stderr, "\n:: R :::::::::: Receiver(%x). [%d  %d]: Exception %d in precv-wait :::::::::: n", (int)(THR_self()), gix, myid, excpn);
    return(excpn);
  }
*/

/*
  tag = 99;

  if (0 > (excpn = timed_probe(&src, tag, &status, COM_FOREVER))) {
    fprintf(stderr, "\n:: R :::::::::: Receiver(%x). [%d  %d]: Exception %d :::::::::: \n", (int)(THR_self()), gix, myid, excpn);
    return(excpn);
  }
  
  cnt = status.Count;
  fprintf(stdout, "\n:: R ooooooooo Receiver(%x). [%d  %d]: Msg of %d bytes :::::::::: \n", (int)(THR_self()), gix, myid, cnt);
  if(0 > (excpn = timed_recv(&src, (char *)buf, cnt, tag, &status, COM_FOREVER))) {
    fprintf(stderr, "\n:: R :::::::::: Receiver(%x). [%d  %d]: Exception %d :::::::::: \n", (int)(THR_self()), gix, myid, excpn);
    return(excpn);
  }

  cnt -= sizeof(int);

  for(i = 0; i < cnt / sizeof(int); i++) {
    if(buf[i] != itr) {
      fprintf(stderr, "\n :::::::::: Radiator (%x). USR %d Fail!!. Itr = %d/Value = %d :::::::::: \n", (int)THR_self(), i, itr, buf[i]);
      exit(1);
    }
  }
  if (buf[i] != 0) {
    fprintf(stderr, "\n :::::::::: Radiator (%x). MARK %d Fail!!. Itr = %d/Value = %d :::::::::: \n", (int)THR_self(), i, itr, buf[i]);
    exit(1);
  }
*/

/*
  tag = 99;
  req_ptr = &req;
  if(0 > (excpn = arecv(&src, (char *)buf, bufSize, tag, req_ptr))) {
    fprintf(stderr, "\n:: S :::::::::: Sender(%x). [%d  %d]: Exception %d :::::::::: n", (int)(THR_self()), gix, myid, excpn);
    free(buf);
    return(excpn);
  }

  if(0 > (excpn = timed_wait(&req_ptr, &status, COM_FOREVER))) {
    fprintf(stderr, "\n:: S :::::::::: Sender(%x). [%d  %d]: Exception %d :::::::::: n", (int)(THR_self()), gix, myid, excpn);
    free(buf);
    return(excpn);
  }
*/
  tag = 99;
  if(0 > (excpn = timed_recv(&src, (char *)buf, bufSize, tag, &status, COM_FOREVER))) {
    fprintf(stderr, "\n:: R :::::::::: Receiver(%x). [%d  %d]: Exception %d :::::::::: n", (int)(THR_self()), gix, myid, excpn);
    return(excpn);
  }

  cnt = (status.Count - sizeof(int)) / sizeof(int);
/*
  fprintf(stdout, "\n:: R ========== Receiver(%x). [%d  %d]: Msg of %d bytes (cnt %d):::::::::: \n", (int)(THR_self()), gix, myid, status.Count, cnt);

  for(i = 0; i < cnt; i++) {
    if(buf[i] != itr) {
      fprintf(stdout, "\n ========== Radiator (%x). USR %d Fail!!. Itr = %d/Value = %d :::::::::: \n", (int)THR_self(), i, itr, buf[i]);
      exit(1);
    }
  }
  if (buf[cnt] != 0) {
    fprintf(stdout, "\n :========= Radiator (%x). MARK %d Fail!!. Itr = %d/Value = %d :::::::::: \n", (int)THR_self(), i, itr, buf[i]);
    exit(1);
  }
*/

#ifdef __CHECK_
    /* ---------- 2. Run the ALGORITHM --------- */
    for(i = 0; i < bufSize / sizeof(int); i++) {
      if(buf[i] != itr) {
        fprintf(stdout, "\n :::::::::: Radiator (%x). USR %d Fail!!. Itr = %d/Value = %d :::::::::: \n", (int)THR_self(), i, itr, buf[i]);
        exit(1);
      }
    }
#endif

    if (++itr == ITERA)
      break;
    
  }
  now_1 = abstime();
#ifdef __DEBUG
  time_t result;
  result = time(NULL);

  fprintf(stdout, "\nmilliseconds %lf\n", (now_1 - now_0));
  fprintf(stdout, "Receiver(%x). [%d, %d].  time: %s  -  BYE\n", (unsigned int)(THR_self()), getGroup(), myid, asctime(localtime(&result)));

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

  srand(abstime());

#ifdef __DEBUG
  fprintf(stdout, "[%d , %d] \n", getGroup(), myid);
#endif

  if (myid % 2)  do_sender(bufsize);
  else           do_receiver(bufsize);

  return 0;
}

