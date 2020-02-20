#include <azq.h>
#include <azq_types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/types.h>

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define LOOPS  5000

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

double abstime (void)
{
  struct timespec t;

  clock_gettime(CLOCK_REALTIME, &t);

  return ((double) t.tv_sec + 1.0e-9 * (double) t.tv_nsec);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int do_job (int bufSize)
{
  int      myid;
  int      gix;
  Addr     left, right;
  double   t0,t1;
  char    *buf;
  int      i, j;
  Rqst    *rqst;
  Rqst_t  *p_rqst;
  int      nodes;

  myid = getRank();
  gix  = getGroup();

  sleep(1);
  printf("\n\n\n");

  printf("Operator(%x). [%d, %d]\n", (unsigned int)(THR_self()), getGroup(), myid);

  /* 1. Get the memory for the data to be sent and received */
  if (NULL == (buf = (char *)malloc(bufSize * 4)))
  {
    perror("malloc(0)");
    exit(1);
  }

  /* 2. Get the memory for all requests */
  if (NULL == (rqst = (Rqst *)malloc(sizeof(Rqst) * 2)))
  {
    perror("malloc(1)");
    exit(1);
  }

  if (NULL == (p_rqst = (Rqst_t *)malloc(sizeof(Rqst_t) * 2)))
  {
    perror("malloc(2)");
    exit(1);
  }

  for (i=0; i<2; i++)
    p_rqst[i] = &rqst[i];

  /* 3. Configure source and destination addresses */
  GRP_getSize(getGroup(), &nodes);
  left.Group = right.Group = getGroup();
  left.Rank = (myid == 0) ? nodes - 1 : myid - 1;
  right.Rank = (myid == (nodes - 1)) ? 0 : myid + 1;

  printf("Nodes:%d\n", nodes);
  printf("Left.Rank: %d\n", left.Rank);
  printf("Right.Rank: %d\n", right.Rank);

  for (i=131072; i<=bufSize; i+=i)
  {
    /////////////////////////////////////////////////////////////////////////////// get t0
    t0 = abstime();

    for (j=0; j<LOOPS; j++)
    {
      /////////////////////////////////////////////////////////////////////////////// asend(1)
      if (0 > asend(&right, (char *) &buf[0], i, j, &rqst[0]))
      {
       perror("asend");
       exit(1);
      }

      //printf("asend (1)\n");

      /////////////////////////////////////////////////////////////////////////////// asend(2)
      if (0 > asend(&left, (char *) &buf[bufSize], i, j, &rqst[1]))
      {
        perror("asend");
        exit(1);
      }

      //printf("asend (2)\n");

      /////////////////////////////////////////////////////////////////////////////// recv(1)
      if (0 > recv(&left, (char *) &buf[bufSize*2], i, j, NULL))
      {
        perror("recv");
        exit(1);
      }

      //printf("recv(1)\n");

      /////////////////////////////////////////////////////////////////////////////// recv(2)
      if (0 > recv(&right, (char *) &buf[bufSize*3], i, j, NULL))
      {
        perror("recv");
        exit(1);
      }

      //printf("recv(2)\n");

      /////////////////////////////////////////////////////////////////////////////// waitall
      if (0 > waitall(p_rqst, 2, NULL))
      {
        perror("waitall");
        exit(1);
      }

      //printf("waitall\n");
      if (!(j % 100))
      {
        printf("%d\n", j);
      }
    }

    /////////////////////////////////////////////////////////////////////////////// get t1
    t1 = abstime();
    printf("%d -- %lf\n", i, t1-t0);

    //if (i==0) i=1024;
  }

  if (buf)
    free(buf);

  return 0;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int node_main (int argc, char *argv[])
{
  int   myid;
  int   numprocs;
  int   bufsize;

  GRP_getSize(getGroup(), &numprocs);
  myid = getRank();

  bufsize = atoi(argv[1]);

  do_job(bufsize);

  return 0;
}
