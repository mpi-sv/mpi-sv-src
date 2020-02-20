/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   used by this module                                          *
 *----------------------------------------------------------------*/
#ifdef __OSI
#include <osi.h>
#endif
#include <azq.h>
#include <azq_types.h>
#undef wait
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*----------------------------------------------------------------*
 *   Number of operator to run                                    *
 *----------------------------------------------------------------*/
#define NUM_OPERATOR     2

/*----------------------------------------------------------------*
 *   Operator's main in another file                              *
 *----------------------------------------------------------------*/
extern int operator_main  (void *param);

/*----------------------------------------------------------------*
 *   Operators to run                                             *
 *----------------------------------------------------------------*/
#define OPERATOR             0xbf3af287

REG_Entry oprTable[] = {
       /* Name               Body Function            Stack size
          ----               -------------            ---------- */
       { OPERATOR,           operator_main,           (32 * 1024) }
};

/*----------------------------------------------------------------*
 *   Creating an Azequia application                              *
 *----------------------------------------------------------------*/

int AZQ_clnt(int argc, char **argv) {

  int        retCode  [NUM_OPERATOR];
  int        i;
  int        mchn     [NUM_OPERATOR];
  CommAttr   CommAttr;
  int        groupId  = GIX_NONE;


  /* 2. Only one processor launches applications */
  if(getCpuId() != 0) {
    fprintf(stdout, "CPU %d. I do not lauch anything\n", getCpuId()); fflush(NULL);
    return 0;
  }

  /* 3. Launch applications */
  for(i = 0; i < NUM_OPERATOR; i++) {
    mchn     [i] = i % INET_getNodes();
  }

  if(0 > GRP_create (&groupId, mchn, NUM_OPERATOR, getCpuId()))                goto exception2;
  for(i = 0; i < NUM_OPERATOR; i++) {
    CommAttr.Flags = COMMATTR_FLAGS_SLM_ENABLED;

    if(0 > GRP_join(groupId, i, OPERATOR, NULL, 0, &CommAttr))                 goto exception;

  } 
  if(0 > GRP_start(groupId))                                                   goto exception;

  if(0 > GRP_wait(groupId, retCode))                                           goto exception;

  for (i = 0; i < NUM_OPERATOR; i++)
    if (retCode[i] != 0) {
      fprintf(stdout, "AZQ_main: finishing with errors: %d  %d\n", i, retCode[i]);
      return(retCode[i]);
    }

  //fprintf(stdout, "AZQ_main: End\n");
  GRP_shutdown();

  return 0;

exception:
  fprintf(stdout, ">>> Exception raised in AZQ_main\n");
  GRP_kill(groupId);
  return(-1);

exception2:
  fprintf(stdout, ">>> Exception raised in AZQ_main creating group\n");
  return(-2);
}


/*----------------------------------------------------------------*
 *   Main                                                         *
 *----------------------------------------------------------------*/
int main(int argc, char **argv) {

  /* Init Azequia */
  if(0 > AZQ_init((REG_Entry *)&oprTable, 1, argc, argv)) {
    fprintf(stdout, "main: Cannot start AZEQUIA\n");
    return(-1);
  }

#ifdef __DEBUG
  fprintf(stdout, "main: End.\n\n");
#endif

  return(0);
}

