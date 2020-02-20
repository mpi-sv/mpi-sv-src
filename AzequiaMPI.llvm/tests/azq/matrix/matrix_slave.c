#include <azq.h>

#include "c_args.h"



/*----------------------------------------------------------------*
 *   Implementation of private functions                          *
 *----------------------------------------------------------------*/


/*----------------------------------------------------------------*
 *   Implementation of operator body                              *
 *----------------------------------------------------------------*/
 

int matrix_slave (int *param) {

  int         myRank;
  int         i, k;
  float      *row;
  float      *column;
  float       value;
  int         tag = 0;
  Status      status;
  int         nslaves;
  int         iter;
  Addr        destId, sourceId;


  myRank = getRank();
  
  /* There are a master and a set of slaves doing work */
  GRP_getSize(getGroup(), &nslaves);
  nslaves--;

  fprintf(stdout, "Matrix slave operator: Rank %d", myRank);

  row    = (float *) malloc (ROWS * sizeof(float));
  column = (float *) malloc (COLUMNS * sizeof(float));
   
  sourceId.Group = getGroup();
  sourceId.Rank  = 0;
  destId.Group   = getGroup();
  destId.Rank    = 0;
    
  for (iter = 0; iter < NUM_ITER; iter++) {
    
    for (k = 0; k < (COLUMNS * ROWS / nslaves); k++) {

	  fprintf(stdout, "%d -> Recibiendo ROW / COLUMN %d \n", myRank, k);
      //MPI_Recv(row,    ROWS,    MPI_FLOAT, 0, tag, MPI_COMM_WORLD, &status);
	  recv(&sourceId, (char *)row, ROWS * sizeof(float), tag, &status);
      //MPI_Recv(column, COLUMNS, MPI_FLOAT, 0, tag, MPI_COMM_WORLD, &status);
	  recv(&sourceId, (char *)column, COLUMNS * sizeof(float), tag, &status);

      value = 0.0;
      for (i = 0; i < ROWS; i++)
          value = value + (row[i] * column[i]);
      
      //MPI_Send(&value, 1, MPI_FLOAT, 0, tag, MPI_COMM_WORLD);
	  fprintf(stdout, "%d -> Enviando VALOR %d \n", myRank, k);
	  send(&destId, (char *)&value, sizeof(float), tag);
    }
  }

  

  /*for (iter = 0; iter < 1; iter++) {
    sourceId.Rank = 0;
    recv(&sourceId, (char *)row, sizeof(float), tag, &status);
  }*/

    
  //fprintf(stdout, "Freeing space of data\n");
  //TSK_sleep(10000);
  free(row);
  free(column);

  //if (myRank == 3) TSK_sleep(20);  
  fprintf(stdout, "Rank %d terminating\n", myRank);
  return 0;

exception:
  return -1;
}
