#include <azq.h>


/*----------------------------------------------------------------*
 *   Global variables                                             *
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------*
 *   Define the Azequia Routing Table                             *
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------*
 *   Define the Azequia Operators Table                           *
 *----------------------------------------------------------------*/
extern int sum  (int *param);
extern int res  (int *param);

#define SUM_OPERATOR   0x21827df5
#define RES_OPERATOR   0x21827e87

int oprTable[] =
{
  /* Name                     Body Function            Stack size
     ----                     -------------            ---------- */
  SUM_OPERATOR,         (int) sum,             PTHREAD_STACK_MIN * 4,
  RES_OPERATOR,         (int) res,             PTHREAD_STACK_MIN * 4,
};


/*----------------------------------------------------------------*
 *   createVec                                                    *
 *----------------------------------------------------------------*/
#define OPERATORS      2

int createVec (int *gix) {

  int       i;
  int       param;
  int       minPrio;
  int       paramSize;
  int       groupId = GIX_NONE;
  int       mchn[OPERATORS];
  int       prio[OPERATORS];
  CommAttr  commAttr;

  fprintf(stdout, "createVec: %d slaves", OPERATORS);

  minPrio = 1; 
  param = 1213;
  paramSize = sizeof(int);

  /* set the priority */
  for (i = 0; i < OPERATORS; i++)
	prio[i] = minPrio;

  /* set the destination machine */
  for (i = 0; i < OPERATORS; i++) {
    if ((i == 0) || (i == 3)) mchn[i] = 0;
	else if ((i == 1) || (i == 2)) mchn[i] = 1;
	else mchn[i] = 0;
  }
 
  /* launch the operators */
  if(0 > GRP_create(&groupId, mchn, OPERATORS, getCpuId()))      goto exception;
  
  commAttr.Flags = 0; //COMMATTR_FLAGS_SLM_ENABLED;
  for (i = 0; i < OPERATORS; i++) {
    if(0 > GRP_join(groupId, i, SUM_OPERATOR, prio[i], &param, paramSize, &commAttr))   goto exception;
	i++;
    if(0 > GRP_join(groupId, i, RES_OPERATOR, prio[i], &param, paramSize, &commAttr))   goto exception;
  }

  if(0 > GRP_start(groupId))                                                    goto exception;

  *gix = groupId;
  fprintf(stdout, "- createVec Launched -");
  return 0;

exception:
  fprintf(stdout, "\n>>> Exception raised in createVec <<<\n");
  GRP_kill(groupId);
  return -1;
}

/*----------------------------------------------------------------*
 *   AZQ_main                                                     *
 *----------------------------------------------------------------*/
int AZQ_main (void) {

  int j, gix, retCode[OPERATORS];

  OPR_register(&oprTable, 2);

  /* Only one processor launches applications */
  if (getCpuId() != 0)                                                          return 0;

  if (0 > createVec(&gix))                                                      goto exception;
  if (0 > GRP_wait(gix, retCode))                                               goto exception;

  for(j = 0; j < OPERATORS; j++)
    fprintf(stdout, "AZQ_main: Operator %d returns with code %d", j, retCode[j]);

  return 0;

exception:
  fprintf(stdout, "\n>>> Exception raised in AZQ_main <<<\n");
  return -1;
}

/*----------------------------------------------------------------*
 *   Main                                                         *
 *----------------------------------------------------------------*/
int main (int argc, char **argv) {

  int ent = 0;

  if (argc == 2) {
    ent = atoi(argv[1]);  // AZQ_rt
  }

  /* Init Azequia */
  if(0 > AZQ_init(&ent, argc, argv)) {
    fprintf(stdout, "main: Cannot start AZEQUIA\n");
    return(-1);
  }

  /* Wait for AZQ_main termination */
  AZQ_wait();

  return 0;
}
