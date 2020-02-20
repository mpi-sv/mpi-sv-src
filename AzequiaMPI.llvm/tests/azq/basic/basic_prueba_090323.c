/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   used by this module                                          *
 *----------------------------------------------------------------*/
#include <azq.h>

#ifdef __OSI
#include <osi.h>
#endif
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>


/*----------------------------------------------------------------*
 *   Definition of private data                                   *
 *----------------------------------------------------------------*/
//#define  SIZE         ((8192 * 2) * sizeof(int))
#define  SIZE         (20)
#define  NUM_MSGS     10000000
#define  MOD          10000

/*----------------------------------------------------------------*
 *   Definition of private functions                              *
 *----------------------------------------------------------------*/
double abstime (void) {

  struct timespec t;

  clock_gettime(CLOCK_REALTIME, &t);

  return ((double) t.tv_sec + 1.0e-9 * (double) t.tv_nsec);
}


/*----------------------------------------------------------------*
 *   Definition of functions                                      *
 *----------------------------------------------------------------*/
int operator_main(int argc, char **argv) {

  int      myid;
  int      numprocs;
  Addr     src, dst;
  int     *buf;
  int      i, j, k;
  double   t_start, t_end;
  Status   status;
  int      total = 0, total_r, result;
  int      received[512];
  int      mymchn;
  int      ret;


  myid = getRank();
  GRP_getSize(getGroup(), &numprocs);
  mymchn = INET_getCpuId();

  if (myid == 0) {fprintf(stdout, "Process %d of %d starting ... (self: 0x%x)\n", myid, numprocs, (int)THR_self());fflush(NULL);}
  //else sleep(1);

  t_start = abstime();

  buf = (int *)malloc(SIZE * sizeof(int));

  //if (!(myid % 2))  sleep(1);
  //srand(myid);
  if (myid != 0) {
    total = 0;
    for (j = 0; j < SIZE; j++) {
      //buf[j] = rand() % 100;
      //total = total + buf[j];
      buf[j] = myid;
      total = total + buf[j];
    }
    //fprintf(stdout, "Process %d: total %d\n", myid, total);fflush(NULL);
  }
  //sleep(1);

  for (i = 0; i < 128; i++)
    received[i] = 0;

  //fprintf(stdout, "Process %d of %d starting ... (self: 0x%x)\n", myid, numprocs, (int)THR_self());fflush(NULL);

  for (i = 0; i < NUM_MSGS; i++) {

    if (myid == 0) { /* Receptor */

      src.Group = getGroup();

      for (k = 1; k < numprocs; k++) {
        //src.Rank = k;
        src.Rank = ADDR_RNK_ANY;

        //fprintf(stdout, "[%d] Recibiendo msg %d de %d ...\n", getRank(), i, src.Rank);
        ret = recv(&src, (char *)buf, SIZE * sizeof(int), 99, &status);

        /*fprintf(stdout, "[%d] Recibido msg %d de %d con total: %d (%d)\n", getRank(), i, status.Src.Rank, buf[0], ret);
        if (buf[0] != i) {
          exit(1);
        }*/

        total_r = 0;
        for (j = 0; j < status.Count / sizeof(int); j++) {
          total_r = total_r + buf[j];
        }

        if (!(received[status.Src.Rank] % MOD)) {
          fprintf(stdout, "[%d] Recibido msg %d de %d con total: %d (%d)\n", getRank(), received[status.Src.Rank], status.Src.Rank, total_r, ret);
          fflush(NULL);
        }
        received[status.Src.Rank] += 1;

        src.Rank = status.Src.Rank;
        //src.Rank = k;
        //fprintf(stdout, "[%d] Enviando respuesta %d a %d ...\n", myid, i, src.Rank);
        send(&src, (char *)&total_r, sizeof(int), 87);
        //fprintf(stdout, "[%d] Enviada respuesta %d a %d ...\n", myid, i, src.Rank);

      }

    } else { /* Emisores */

      dst.Group = getGroup();
      dst.Rank  = 0;

      //buf[0] = i;

      //fprintf(stdout, "[%d] Enviando msg %d ...\n", myid, i);
      send(&dst, (char *)buf, SIZE * sizeof(int), 99);
      //fprintf(stdout, "[%d] Enviado msg %d \n", myid, i);

      dst.Group = getGroup();
      dst.Rank  = 0;

      //fprintf(stdout, "[%d] Recibiendo repuesta %d ...\n", myid, i);
      ret = recv(&dst, (char *)&result, sizeof(int), 87, &status);
      //fprintf(stdout, "[%d] Recibida respuesta %d (%d)\n", myid, i, ret);

      if (result != total) fprintf(stdout, "[%d / %d] ERROR en total: %d y recibido:  %d  (%d)\n", getRank(), i, total, result, ret);

    }
  }

  free(buf);

  t_end = abstime();
  if (myid == 0) { fprintf(stdout, "TIME: %lf\n" , t_end - t_start);}

  fprintf(stdout, "I am the %d rank of %d processes finalizing ...\n", myid, numprocs);
  fflush(stdout);

  return(0);
}
