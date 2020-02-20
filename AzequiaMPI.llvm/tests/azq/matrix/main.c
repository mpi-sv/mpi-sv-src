#include <azq.h>


/*----------------------------------------------------------------*
 *   Global variables                                             *
 *----------------------------------------------------------------*/
AZQ_Param azqParam;

/*----------------------------------------------------------------*
 *   Define the Azequia Routing Table                             *
 *----------------------------------------------------------------*/
//#define PROCESSOR_NR  3

//int Routed_Send[PROCESSOR_NR][PROCESSOR_NR] =  { { SHM } };

// 3 Procesadores ANILLO SIMPLE
//int Routed_Send[PROCESSOR_NR][PROCESSOR_NR] = 
//                                               { { SHM,   SDB0,  SDB1}  ,  
//                                                 { SDB1,  SHM,   SDB0}  ,
//                                                 { SDB0,  SDB1,  SHM}   };

//int Routed_Send[PROCESSOR_NR][PROCESSOR_NR] = 
//                                               { {  SHM ,  SDB0 }  ,  
//                                                 { SDB1 ,  SHM  }  };
/*----------------------------------------------------------------*
 *   Define the Azequia Operators Table                           *
 *----------------------------------------------------------------*/
extern int matrix_master (int *param);
extern int matrix_slave  (int *param);

#define MSLAVE_OPERATOR   0x21827df5
#define MMASTER_OPERATOR  0x21827e87

int oprTable[] =
{
  /* Name                     Body Function            Stack size
     ----                     -------------            ---------- */
  MSLAVE_OPERATOR,         (int) matrix_slave,       PTHREAD_STACK_MIN,
  MMASTER_OPERATOR,        (int) matrix_master,      PTHREAD_STACK_MIN,
};


/*----------------------------------------------------------------*
 *   createPI                                                     *
 *----------------------------------------------------------------*/
#define OPERATORS      4

int createMatrix (int *gix) {

  int       i;
  int       param;
  int       minPrio;
  int       paramSize;
  int       groupId = GIX_NONE;
  int       mchn[OPERATORS + 1];
  int       prio[OPERATORS + 1];
  CommAttr  commAttr;

  fprintf(stdout, "createMatrix: %d slaves", OPERATORS);

  minPrio = 1; //THR_getMinPrio();
  param = 1213;
  paramSize = sizeof(int);

  /* set the priority */
  for (i = 0; i < OPERATORS + 1; i++)
    //prio[i] = (i == 0) ? minPrio + 1 : minPrio;
	prio[i] = minPrio;

  /* set the destination machine */
  for (i = 0; i < OPERATORS + 1; i++) {
    //mchn[i] = 0;
    mchn[i] = i % 2;
	//mchn[i] = (i % PROCESSOR_NR);
	//mchn[i] = (i % PROCESSOR_NR) ? 0 : 1;
	//mchn[i] = 0;
	//if (i > 4) mchn[i] = 2;
	//else mchn[i] = 1;
  }
  mchn[0] = 0;

  //commAttr.Protocols |= COM_SLM;
  //commAttr.SLM_FrgNr = 10;

  //commAttr.Protocols |= COM_RRV;
  //commAttr.RRV_Limit = 0;

  /* launch the operators */
  //OPR_register(&oprTable2, 2);

  if(0 > GRP_create(&groupId, mchn, OPERATORS + 1, getCpuId()))      goto exception;
  
  commAttr.Flags = 0; //COMMATTR_FLAGS_SLM_ENABLED;
  fprintf(stdout, "Launching master ...");
  if(0 > GRP_join(groupId, 0, MMASTER_OPERATOR, prio[0], &param, paramSize, &commAttr))    goto exception;
  fprintf(stdout, "Launching slave ...");
  for(i = 1; i < OPERATORS + 1; i++) {
    if(0 > GRP_join(groupId, i, MSLAVE_OPERATOR, prio[i], &param, paramSize, &commAttr))   goto exception;
  }

  if(0 > GRP_start(groupId))                                                    goto exception;

  *gix = groupId;
  fprintf(stdout, "- createMatrix Launched -");
  return 0;

exception:
  fprintf(stdout, "\n>>> Exception raised in createMaster <<<\n");
  GRP_kill(groupId);
  return -1;
}

/*----------------------------------------------------------------*
 *   AZQ_main                                                     *
 *----------------------------------------------------------------*/
int AZQ_main (void) {

  int j, gix, retCode[OPERATORS + 1];

  OPR_register(&oprTable, 2);

  /* Only one processor launches applications */
  if (getCpuId() != 0)                                                          return 0;

  if (0 > createMatrix(&gix))                                                   goto exception;
  if (0 > GRP_wait(gix, retCode))                                               goto exception;

  for(j = 0; j < OPERATORS + 1; j++)
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
