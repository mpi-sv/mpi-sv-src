#include <azq.h>

#include "c_args.h"


/*----------------------------------------------------------------*
 *   Implementation of private functions                          *
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------*
 *   Implementation of operator body                              *
 *----------------------------------------------------------------*/
int sum (int *param) {

  int                myRank, numoper;
  unsigned long int  tstart, tend;
  int                iter;
  Addr               destId;
  int                value;
  Status             status;
  int                i;
  int vect_orig[MAX_SIZE];

  myRank = getRank();
  GRP_getSize(getGroup(), &numoper);

  fprintf(stdout, "SUM operator: Rank %d of %d", myRank, numoper);

  //TSK_sleep(10);

  for (i = 0; i < MAX_SIZE; i++)
    vect_orig[i] = i + myRank;

  tstart = CLK_getltime();
  
  destId.Group    = getGroup();
  for (iter = 0; iter < NUM_ITER; iter++) {
    destId.Rank = myRank + 1;
    send(&destId, (char *)vect_orig, MAX_SIZE * sizeof(int), 74);
	
    recv(&destId, (char *)&value, sizeof(int), 76, &status);
	//TSK_sleep(1);
  }
	
  //fprintf(stdout, "Rank  %d  Valor  %d", myRank, value);
  fprintf(stdout, "Valor  %d", value);

  tend = CLK_getltime();
  fprintf(stdout, "\nRank %d terminating ... \n", myRank); 
  fprintf(stdout, "TIME %d \n", tend - tstart); 

  return 0;

exception:
  return -1;
}
