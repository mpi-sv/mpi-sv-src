
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
  Rqst     rqst;
  Rqst_t   req;
  Rqst_t   all_rqst;
  Rqst_t   all_req[16];
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

  if (myid == (numprocs - 1)) {
    all_rqst = (Rqst *)malloc(sizeof(Rqst) * numprocs);
    for (i = 0; i < (numprocs - 1); i++) {
      all_req[i] = NULL;
      all_req[i] = &all_rqst[i];
    }
  }

  buf = 0;



  for (k = 0; k < 10000000; k++) {

    if (myid != (numprocs - 1)) {

      dst.Group = getGroup();
      dst.Rank  = numprocs - 1;

      buf = myid;

      //fprintf(stdout, "%d doing send ... (value: %d)\n", myid, buf);
      if (0 > asend(&dst, (char *)&buf, sizeof(int), 99, &rqst)) {err = -3; goto error;}

      /*tslp.tv_sec = 0;
      tslp.tv_nsec = (rand () % 10000000);
      nanosleep(&tslp,NULL);*/

      req = &rqst;
      //fprintf(stdout, "%d Request: 0x%x (&: 0x%x)\n", myid, req, &req);
      if (0 > wait(&req, NULL)) {err = -1; goto error;}
      //fprintf(stdout, "%d Enviado %d bytes to %d\n", myid, sizeof(int), dst.Rank);
      //buf++;

      //if (0 > send(&dst, (char *)&buf, sizeof(int), 99)) {err = -3; goto error;}



    } else {

      /*tslp.tv_sec = 0;
      tslp.tv_nsec = 5000000;
      nanosleep(&tslp,NULL);*/

      for (i = 0; i < numprocs - 1; i++) {
        src.Group = getGroup();
        src.Rank  = i;
        //src.Rank  = ADDR_RNK_ANY;

        arecv(&src, (char *)&bufrecv[i], sizeof(int), 99, all_req[i]);
        //fprintf(stdout, "Receiving from %d.  Request: 0x%x (address: 0x%x)\n", i, all_req[i], &all_req[i]);

      }

      for (i = 0; i < (numprocs - 1); i++) {
        waitany(all_req, numprocs - 1, &idx, &st);
        //fprintf(stdout, "Recibido %d bytes from %d, value: %d\n", st.Count, st.Src.Rank, bufrecv[0]);

        if (idx != bufrecv[idx])
          fprintf(stdout, "\nERROR. Recibido: %d\n", bufrecv[idx]);
      }

      if (!(k % 10000)) fprintf(stdout, "Iter: %d\n", k);

    }
  }

  sleep(1);
  if (myid == (numprocs - 1)) {
    free(all_rqst);
  }
  fprintf(stdout, "Thread  %d  of  %d  terminating ... (iter %d)\n", myid, numprocs, iter);

  return 0;

error:
  fprintf(stdout, "Thread  %d  terminating with error: %d\n", myid, err);
}

