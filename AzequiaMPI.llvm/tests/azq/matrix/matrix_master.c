#include <azq.h>

#include "c_args.h"



float A[ROWS][COLUMNS];
float B[ROWS][COLUMNS]; /* Stored in columns */
float C[ROWS][COLUMNS];


/*----------------------------------------------------------------*
 *   Implementation of private functions                          *
 *----------------------------------------------------------------*/


/*----------------------------------------------------------------*
 *   Implementation of operator body                              *
 *----------------------------------------------------------------*/
int matrix_master (int *param) {

  int                myRank;
  int                i, j, k;
  float              value;
  int                tag = 0;
  int                dst;
  Status             status;
  unsigned long int  tstart, tend;
  int                nslaves;
  int                iter;
  Addr               destId, sourceId;


  //if (NULL == (slm_buffer = (char *) malloc(SLM_FRAGMENT_SIZE * 10)))           goto exception;

  //THR_buffer_attach(slm_buffer);

  myRank = getRank();
  
  /* There are a master and a set of slaves doing work */
  GRP_getSize(getGroup(), &nslaves);
  nslaves--;

  fprintf(stdout, "Matrix master operator: Rank %d", myRank);

  fprintf(stdout, "Generating A & B matrixes ...");
    srand(12345);
    for (i = 0; i < ROWS; i++) {
      for (j = 0; j < COLUMNS; j++) {
        A[i][j] = (float)rand() / (float)rand();
        B[i][j] = (float)rand() / (float)rand();
      }
    }
#ifdef _PRNT
    fprintf(stdout, "\nMatrix A: ");
    for (i = 0; i < ROWS; i++) {
      for (j = 0; j < COLUMNS; j++) 
        fprintf(stdout, "%.4f  ", A[i][j]);
    }

    fprintf(stdout, "\nMatrix B: ");
    for (j = 0; j < COLUMNS; j++) {
      for (i = 0; i < ROWS; i++) 
        fprintf(stdout, "%.4f  ", B[i][j]);
    }
#endif    
    
    fprintf(stdout, "Calculating C matrix\n");

    tstart = CLK_getltime();
  
    destId.Group    = getGroup();
	sourceId.Group  = getGroup();

    for (iter = 0; iter < NUM_ITER; iter++) {    
      for (j = 0; j < COLUMNS; j++) {
        for (i = 0; i < ROWS; i++) {
          dst = (i % nslaves) + 1;
		  fprintf(stdout, "0 -> Enviando ROW %d COLUMN %d a SLAVE %d", i, j, dst);
		  destId.Rank = dst;
		  send(&destId, (char *)A[i], ROWS * sizeof(float), tag);
		  send(&destId, (char *)B[i], COLUMNS * sizeof(float), tag);

          if (dst == nslaves) {
            for (k = 1; k <= nslaves; k++) {
      		  sourceId.Rank = k;
			  fprintf(stdout, "0 -> Recibiendo VALOR de SLAVE %d\n", k);
			  recv(&sourceId, (char *)&value, sizeof(float), tag, &status);
              C[(i - nslaves + k)][j] = value;
            }
          }
        }
      }
	  fprintf(stdout, "Iter: %d", iter); 
    }
	

	/*for (iter = 0; iter < 1; iter++)
	for (i = 1; i <= nslaves; i++) {
	  destId.Rank = i;
	  send(&destId, (char *)A[i], sizeof(float), tag);
	}*/
    
    tend = CLK_getltime();
    fprintf(stdout, "Time: %d\n", (tend - tstart) / iter); 
    
#ifdef _PRNT
    fprintf(stdout, "Matrix C: ");
    for (i = 0; i < ROWS; i++) {
      for (j = 0; j < COLUMNS; j++) 
        fprintf(stdout, "%.4f  ", C[i][j]);
    }
#endif       


  return 0;

exception:
  return -1;
}
