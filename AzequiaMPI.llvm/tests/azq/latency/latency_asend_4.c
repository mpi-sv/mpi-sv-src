
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

  int      numprocs, myid;
  Rqst    *rqst;
  Addr     dst, src;
  int      buf;
  int      bufrecv[COUNT];
  Status   status;
  Status   st;
  int      total;
  int      i, j, k;
  int      idx;
  int      iter = 0;
  int      turn = 0;
  int      err = 0;
  struct timespec tslp;


  GRP_getSize(getGroup(), &numprocs);
  myid = getRank();

  //if (myid != 0)
  //  rqst = (Rqst *)malloc(sizeof(Rqst));
  buf = 0;

  for (k = 0; k < 100000000; k++) {

    if (myid != 0) {

      dst.Group = getGroup();
      dst.Rank  = 0;

      //fprintf(stdout, "%d doing send ... (value: %d)\n", myid, buf);
      if (0 > asend(&dst, (char *)&buf, sizeof(int), 99, &rqst)) {err = -3; goto error;}

      /*tslp.tv_sec = 0;
      tslp.tv_nsec = (rand () % 1000000);
      nanosleep(&tslp,NULL);*/

      if (0 > waitany(&rqst, 1, &idx, NULL)) {err = -1; goto error;}
      //fprintf(stdout, "%d Enviado %d bytes to %d\n", myid, sizeof(int), dst.Rank);

      buf++;

    } else {

      /*tslp.tv_sec = 0;
      tslp.tv_nsec = 500000;
      nanosleep(&tslp,NULL);*/

      for (i = 0; i < numprocs - 1; i++) {
        src.Group = getGroup();
        src.Rank  = i + 1;
        //src.Rank  = ADDR_RNK_ANY;

        tmd_recv(&src, (char *)bufrecv, sizeof(int), 99, &st, COM_FOREVER);
        //fprintf(stdout, "Recibido %d bytes from %d, value: %d\n", st.Count, st.Src.Rank, bufrecv[0]);

        if (k != bufrecv[0])
          fprintf(stdout, "\nERROR. Recibido: %d\n", bufrecv[0]);
      }

      if (!(k % 100000)) {fprintf(stdout, "Iter: %d\n", k);}
    }
  }

  sleep(1);
  //if (myid != 0)
  //  free(rqst);
  fprintf(stdout, "Thread  %d  of  %d  terminating ... (iter %d)\n", myid, numprocs, iter);

  return 0;

error:
  fprintf(stdout, "Thread  %d  terminating with error: %d\n", myid, err);
}

