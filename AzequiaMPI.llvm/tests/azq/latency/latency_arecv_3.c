
#if defined(__OSI)
  #include <osi.h>
#else
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <time.h>
#endif

#include <azq.h>


#define  COUNT  256


int latency(int *param) {

  int    numprocs, myid;
  Rqst  *rqst;
  Addr   dst, src;
  int    buf;
  int    bufrecv[COUNT];
  Status status;
  Status st;
  int    total;
  int    i, j, k;
  int    idx;
  int    iter = 0;
  int    turn = 0;
  int    err = 0;
  struct timespec tslp;


  GRP_getSize(getGroup(), &numprocs);
  myid = getRank();


  for (k = 0; k < 100000000; k++) {

    if (myid == (numprocs - 1)) {

      buf = k;

      for (i = 0; i < (numprocs - 1); i++) {
        dst.Group = getGroup();
        dst.Rank  = i;

        /*tslp.tv_sec = 0;
        tslp.tv_nsec = rand() % 4000000;
        nanosleep(&tslp,NULL);*/

        //fprintf(stdout, "%d doing send ... (value: %d)\n", myid, buf);
        if (0 > send(&dst, (char *)&buf, sizeof(int), 99)) {err = -3; goto error;}
      }

      if (!(k % 100000)) fprintf(stdout, "Iter: %d\n", k);

    } else {

      src.Group = getGroup();
      src.Rank  = numprocs - 1;
      //src.Rank  = ADDR_RNK_ANY;

      if (0 > async_recv(&src, (char *)&bufrecv[0], sizeof(int), 99, &rqst)) {err = -2; goto error;}
      //fprintf(stdout, "Arecv returns request %d as 0x%x\n", j, &srq[j]);

      /*tslp.tv_sec = 0;
      tslp.tv_nsec = rand() % 1000000;
      nanosleep(&tslp,NULL);*/

      if (0 > tmd_wait(&rqst, 1, &idx, &st, COM_FOREVER)) {err = -1; goto error;}
      //fprintf(stdout, "Index %d  (iter  %d)\n", idx, iter);
      //fprintf(stdout, "Recibido %d bytes from %d, value: %d\n", st.Count, st.Src.Rank, bufrecv[idx]);

      if (bufrecv[0] != k) {fprintf(stdout, "\nERROR  0. Recibido: %d\n", bufrecv[0]);break;}

    }
  }

  if (myid == (numprocs - 1)) {
    sleep(1);
    fprintf(stdout, "Thread  %d  of  %d  terminating ... (iter %d)\n", myid, numprocs, iter);
  }


  return 0;

error:
  fprintf(stdout, "Thread  %d  terminating with error: %d\n", myid, err);
}

