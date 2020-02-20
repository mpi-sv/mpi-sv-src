
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
  Rqst  *rqst, *req, *req2;
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
  Rqst   srq[16];
  Rqst_t rp[16];
  int    err = 0;
  struct timespec tslp;


  GRP_getSize(getGroup(), &numprocs);
  myid = getRank();

  if (myid == (numprocs - 1)) {
    for (i = 0; i < (numprocs - 1); i++)
      rp[i] = &srq[i];
    //for (j = 0; j < numprocs; j++) fprintf(stdout, "Request %d is 0x%x\n", j, &srq[j]);
  }

  for (k = 0; k < 1000000; k++) {

    if (myid < (numprocs - 1)) {

      buf = myid;

      dst.Group = getGroup();
      dst.Rank  = numprocs - 1;

      /*tslp.tv_sec = 0;
      tslp.tv_nsec = rand() % 4000000;
      nanosleep(&tslp,NULL);*/

      //fprintf(stdout, "%d doing send ... (value: %d)\n", myid, buf);
      if (0 > send(&dst, (char *)&buf, sizeof(int), 99)) {err = -3; goto error;}

    } else {

      for (j = 0; j < (numprocs - 1); j++) {
        src.Group = getGroup();
        src.Rank  = j;
        //src.Rank  = ADDR_RNK_ANY;

        //if (0 > nblk_recv(&src, (char *)&bufrecv[j], sizeof(int), 99, &srq[j])) {err = -2; goto error;}
        //fprintf(stdout, "nblk_recv is going to be called with request 0x%x\n", rp[j]);fflush(stdout);
        if (0 > nblk_recv(&src, (char *)&bufrecv[j], sizeof(int), 99, rp[j])) {err = -2; goto error;}
        //fprintf(stdout, "Arecv returns request %d as 0x%x\n", j, &srq[j]);
      }

      /*tslp.tv_sec = 0;
      tslp.tv_nsec = rand() % 1000000;
      nanosleep(&tslp,NULL);*/

      for (j = 0; j < (numprocs - 1); j++) {
        //fprintf(stdout, "Doing wait (rqst vector: 0x%x) ...\n", srq);
        //if (0 > tmd_wait(&srq, numprocs - 1, &idx, &st, COM_FOREVER)) {err = -1; goto error;}
        if (0 > tmd_wait(rp, numprocs - 1, &idx, &st, COM_FOREVER)) {err = -1; goto error;}
        iter++;
        //fprintf(stdout, "Index %d  (iter  %d)\n", idx, iter);
        //fprintf(stdout, "Recibido %d bytes from %d, value: %d\n", st.Count, st.Src.Rank, bufrecv[idx]);

        if (idx >= 0) {
          if ((bufrecv[idx] != idx)) {fprintf(stdout, "\nERROR  0. Recibido: %d\n", bufrecv[idx]);break;}
          //if(turn != idx) fprintf(stdout, "  **************  CAMBIO (%d / %d)  **************\n", turn, iter);
          //turn = ((turn + 1) % (numprocs - 1));
        } /*else {
           fprintf(stdout, "  =========  NULL rqst vector (%d / %d)  ========\n", turn, iter);

        }*/
      }
    }
    if (!(k % 1000) && (myid == 0)) fprintf(stdout, "Iter: %d\n", k);
  }

  if (myid == (numprocs - 1)) {
    sleep(1);
    fprintf(stdout, "Thread  %d  of  %d  terminating ... (iter %d)\n", myid, numprocs, iter);
  }


  return 0;

error:
  fprintf(stdout, "Thread  %d  terminating with error: %d\n", myid, err);
}

