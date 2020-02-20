#if defined(__OSI)
  #include <osi.h>
#else
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
#endif


#include "mpi.h"


#define  BUF_LEN  100

int main (int argc, char **argv) {

  int           myid, numprocs;
  MPI_Status    status;
  int           i;
  int           position;
  int           ent[10];
  char          car[8];
  double        dob[2];
  char          sendbuf[BUF_LEN];
  char          recvbuf[BUF_LEN];
  int           count, nelem;
  int           tot_pck_sz, pck_sz;


  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);


  if (myid == 0) {

    for (i = 0; i < 10; i++)  ent[i] = i + 100;
    for (i = 0; i <  5; i++)  car[i] = i + 75;
    for (i = 0; i <  2; i++)  dob[i] = ((double)i + 5) / 0.3;

    tot_pck_sz = 0;
    MPI_Pack_size(10, MPI_INT,    MPI_COMM_WORLD, &pck_sz);
    tot_pck_sz += pck_sz;
    MPI_Pack_size( 5, MPI_CHAR,   MPI_COMM_WORLD, &pck_sz);
    tot_pck_sz += pck_sz;
    MPI_Pack_size( 2, MPI_DOUBLE, MPI_COMM_WORLD, &pck_sz);
    tot_pck_sz += pck_sz;
    printf("Packing size: %d\n", tot_pck_sz);

    position = 0;
    MPI_Pack(ent, 10, MPI_INT,    sendbuf, BUF_LEN, &position, MPI_COMM_WORLD);
    printf("[SEND]  position: %d\n", position);
    MPI_Pack(car,  5, MPI_CHAR,   sendbuf, BUF_LEN, &position, MPI_COMM_WORLD);
    printf("[SEND]  position: %d\n", position);
    MPI_Pack(dob,  2, MPI_DOUBLE, sendbuf, BUF_LEN, &position, MPI_COMM_WORLD);
    printf("[SEND] Final position: %d\n", position);

    MPI_Send(sendbuf, position, MPI_PACKED, 1, 34, MPI_COMM_WORLD);

  } else if (myid == 1) {

    MPI_Recv(recvbuf, BUF_LEN, MPI_PACKED, 0, 34, MPI_COMM_WORLD, &status);
    MPI_Get_count(&status, MPI_PACKED, &count);
    MPI_Get_elements(&status, MPI_PACKED, &nelem);
    printf("Received %d / %d elements\n", count, nelem);

    position = 0;
    MPI_Unpack(recvbuf, BUF_LEN, &position, ent, 10, MPI_INT,    MPI_COMM_WORLD);
    printf("[RECV]  position: %d\n", position);
    MPI_Unpack(recvbuf, BUF_LEN, &position, car,  5, MPI_CHAR,   MPI_COMM_WORLD);
    printf("[RECV]  position: %d\n", position);
    MPI_Unpack(recvbuf, BUF_LEN, &position, dob,  2, MPI_DOUBLE, MPI_COMM_WORLD);
    printf("[RECV] Final position: %d\n", position);

    for (i = 0; i < 10; i++)  printf("%d  ",  ent[i]); printf("\n");
    for (i = 0; i <  5; i++)  printf("%c  ",  car[i]); printf("\n");
    for (i = 0; i <  2; i++)  printf("%lf  ", dob[i]); printf("\n");
  }

  MPI_Finalize();
  return MPI_SUCCESS;
}
