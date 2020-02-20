#include <azq.h>


/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   used by this module                                          *
 *----------------------------------------------------------------*/
#include <std.h>
#include <log.h>

#include <sundance.h>

extern LOG_Obj stdout;

/*----------------------------------------------------------------*
 *   For DSP compatibility (to eliminate)                         *
 *----------------------------------------------------------------*/
#include <net.h>

#define PROCESSOR_NR  2
int Routed_Send[PROCESSOR_NR][PROCESSOR_NR] =

                /*    P0      P1     P2
                     ----    ----   ----   *
       /* P0 *   { { SHM,   SDB0,  SDB0}  ,  
       /* P1 *     { SDB0,  SHM,   SDB0}  ,
       /* P2 *     { SDB0,  SDB0,  SHM}  
                  }; 

                /*    P0      P1   
                     ----    ----  */
       /* P0 */   { {  SHM ,  SDB0 }  ,  
       /* P1 */     { SDB1 ,  SHM  }  }; 

                /*    P0      
                     ----    */
       /* P0 * { {  SHM } }; */


AZQ_Topology azqTopology;
AZQ_OprTable azqOprTable;
AZQ_Param azqParam;

/*----------------------------------------------------------------*
 *   Operator's main in another file                              *
 *----------------------------------------------------------------*/
extern int slowPi             (int *param);

/*----------------------------------------------------------------*
 *   Operators to run                                             *
 *----------------------------------------------------------------*/
#define SLOW_PI_OPERATOR             0x21827df5

int oprTable[] = {
       /* Name                     Body Function            Stack size
          ----                     -------------            ---------- */
        SLOW_PI_OPERATOR,          (int)slowPi,             UPTHREAD_STACK_MIN * 4,

       /* The last one is like this */
         0,                       (int)NULL,              0,
};


/*----------------------------------------------------------------*
 *   Creating an Azequia application                              *
 *----------------------------------------------------------------*/


      /*________________________________________________________________
     /                                                                  \
    |    createPi                                                        |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
#define PI_MAX        16
#define PRECISION     (1024 * 128 * 2)
//#define PRECISION     (128)
#define PULSOS        1

int createPi (int *gix) {

  int       groupId = GIX_ERROR;
  int       i;
  int       mchn[PI_MAX];
  int       prio[PI_MAX];
  int       paramSize;
  int       precision;
  CommAttr  commAttr = DEFAULT_COMM_ATTR;

  LOG_printf(&stdout, "createPi:\n");
  precision = PRECISION;
  paramSize = sizeof(int);


  for(i = 0; i < PI_MAX; i++) {
    prio[i]  = sched_get_priority_min(SCHED_FIFO) + 2;
    //mchn[i]  = (i % 2)? 1 : 0;
	mchn[i]  = (i % 2) ? 2 : 0;
	//mchn[i]  = 0;
	//mchn[i]  = 1;
  }
  prio[0]  = sched_get_priority_min(SCHED_FIFO) + 3;
  mchn[0] = 0;
  //prio[0] = sched_get_priority_min(SCHED_FIFO) + 4;

  //commAttr.Protocols |= COM_SLM;
  //commAttr.SLM_FrgNr = 5;
  
  commAttr.Protocols |= COM_RRV;
  commAttr.RRV_Limit = 0;
  
  if(0 > GRP_create (&groupId, mchn, PI_MAX, getCpuId(), &commAttr))           goto exception;
  for(i = 0; i < PI_MAX; i++)
    if(0 > GRP_join(groupId, i, SLOW_PI_OPERATOR, prio[i], 
                             (void *)&precision, 
                             paramSize))                                       goto exception;
  if(0 > GRP_start(groupId))                                                   goto exception;
  *gix = groupId;
  LOG_printf(&stdout, "- createPi Launched -\n");
  return(0);

exception:
  LOG_printf(&stdout, ">>> Exception raised in createPi\n");
  GRP_kill(groupId);
  return(-1);
}



int AZQ_main(void) {

  int gix;
  int j;
  int retCode[PI_MAX];


  /* 1. Only one processor launches applications */
  if(getCpuId() != 0)
    return(0);

  /* 2. Launch applications */
  if (0 > createPi(&gix))                                                      goto exception;
  if (0 > GRP_wait(gix, retCode))                                              goto exception;

  for(j = 0; j < PI_MAX; j++) {
    if(retCode[j])
      LOG_printf(&stdout, "AZQ_main: Ret[%d] = %d\n", j, retCode[j]);
  }
  return(0);

exception:
 LOG_printf(&stdout, "\n>>> Exception raised in AZQ_main\n");
  return(-1);
}


/*----------------------------------------------------------------*
 *   Main                                                         *
 *----------------------------------------------------------------*/
int main(int argc, char **argv) {

  /* Package the operator table and the routing table */
  azqTopology.ProcessorNr   = PROCESSOR_NR;
  azqTopology.RouteTable    = Routed_Send;

  azqOprTable = oprTable;

  azqParam.NetParams     = NULL; 
  //azqParam.NetParams     = &azqTopology;
  azqParam.OperatorTable = &azqOprTable;

  /* 1. Init Azequia */
  if(0 > AZQ_init(&azqParam)) {
    LOG_printf(&stdout, "main: Cannot start AZEQUIA\n");
    return -1;
  }

  return 0;
}
