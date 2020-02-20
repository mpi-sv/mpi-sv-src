#include <azq.h>

#include "c_args.h"



/*----------------------------------------------------------------*
 *   Implementation of private functions                          *
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------*
 *   Implementation of operator body                              *
 *----------------------------------------------------------------*/
 
int applyalg(int *vect, int size) {

  int i;
  int add = 0;

  for (i = 0; i < size; i++)
    add += vect[i];
}

int res (int *param) {

  int                myRank, numoper;
  unsigned long int  tstart, tend;
  int                iter;
  Addr               srcId;
  int                value;
  Status             status;
  int vect[MAX_SIZE];


  myRank = getRank();
  GRP_getSize(getGroup(), &numoper);

  fprintf(stdout, "RES operator: Rank %d of %d", myRank, numoper);


  tstart = CLK_getltime();

  srcId.Group    = getGroup();
  for (iter = 0; iter < NUM_ITER; iter++) {
    srcId.Rank = myRank - 1;
	recv(&srcId, (char *)vect, MAX_SIZE * sizeof(int), 74, &status);
	value = applyalg(vect, MAX_SIZE);
	
    send(&srcId, (char *)&value, sizeof(int), 76);
	if (!(iter % 100)) {
	  fprintf(stdout, "Rank  %d  Valor  %d", myRank, value);
	  fprintf(stdout, "          Iter   %d", iter);
	}
  }
	
  //fprintf(stdout, "Rank  %d  Valor  %d", myRank, value);

  tend = CLK_getltime();
  fprintf(stdout, "\nRank %d TIME: %d\n", myRank, (int)(tend - tstart)); 

  return 0;

exception:
  return -1;
}
