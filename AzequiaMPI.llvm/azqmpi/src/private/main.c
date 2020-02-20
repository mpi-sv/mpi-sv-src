/* _________________________________________________________________________
   |                                                                       |
   |  Azequia (embedded) Message Passing Interface   ( AzequiaMPI )        |
   |                                                                       |
   |  Authors: DSP Systems Group                                           |
   |           http://gsd.unex.es                                          |
   |           University of Extremadura                                   |
   |           Caceres, Spain                                              |
   |           jarico@unex.es                                              |
   |                                                                       |
   |  Date:    Sept 22, 2008                                               |
   |                                                                       |
   |  Description:                                                         |
   |                                                                       |
   |                                                                       |
   |_______________________________________________________________________| */


  /*----------------------------------------------------------------/
 /   Declaration of types and functions used by this module        /
/----------------------------------------------------------------*/
#include <config.h>
#include <pmi_interface.h>

#if defined (__OSI)
  #include <osi.h>
#else
  #include <pthread.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <sys/resource.h>
  #include <time.h>
  #include <limits.h>
  #include <sched.h>
#endif

#include <env.h>

#include <azq.h>
#include <inet.h>
#include <p_config.h>

#ifdef HAVE_LIBHWLOC
  #include <hwloc.h>
#endif

#include "mpise_comm.h"
#include "simple-hash.h"
/*----------------------------------------------------------------*
 *   Definition of private constants                              *
 *----------------------------------------------------------------*/
/* Default stack size for an operator */
#define  DEFAULT_STACK_SIZE  (PTHREAD_STACK_MIN * 32)


int * func_cnt_array=NULL;
arg_t ** arg_array=NULL;
hash_table *table= NULL;


/*----------------------------------------------------------------*
 *   Private functions implemented by this module                 *
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------*
 *   Referenced functions implemented by other module             *
 *----------------------------------------------------------------*/
/* Prototype operator function */
//extern int node_main__ (int argc, char *argv[]);
int node_main__ (int argc, char *argv[])
{
	klee_enable_sync_chk(0);
	int res= usermain(argc, argv);
	klee_disable_sync_chk(0);
	return res;
}
/*----------------------------------------------------------------*
 *   Definition of private data                                   *
 *----------------------------------------------------------------*/
/* Name of the operator */
static int  opr_nr;

  /*----------------------------------------------------------------/
 /            Implementation of private functions                  /
/----------------------------------------------------------------*/


  /*----------------------------------------------------------------/
 /   Implementation of public functions                            /
/----------------------------------------------------------------*/

      /*________________________________________________________________
     /                                                                  \
    |  AZQ_clnt                                                          |
    |                                                                    |
    |  Function called from Azequia for user to startup MPI nodes        |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
int AZQ_clnt() {
  int         gix    = GIX_NONE;
  int         i,j;
  int        *mchn;
  int        *retcode=NULL;
  int         ret;
  CommAttr    commAttr;
  int         nodes  = -1;
  //klee_disable_sync_chk(0);
#ifdef VERBOSE_MODE
  fprintf(stdout, "AZQ_clnt: init\n");
#endif
  
  /* 1. CPU id 0 launches the application */
  if (getCpuId() != 0)    return EXIT_SUCCESS;
  
#ifdef VERBOSE_MODE
  {
	time_t      curtime;
	struct tm  *loctime;
	
	fprintf(stdout, "-------------------------------------------------------\n");
	fprintf(stdout, "\tWellcome to AzequiaMPI %s \n", AZQMPI_VERSION);
	curtime = time (NULL);
	loctime = localtime (&curtime);
	fprintf(stdout, "\tDate:       %s", asctime (loctime));
	fprintf(stdout, "-------------------------------------------------------\n");
  }
#endif
  
  /* 1. How many nodes to run */
  if (0 > (nodes = PMII_getNodeNr()))                                           goto exception3;
  klee_set_mpi_ndcnt(nodes);
  
  /* 2. Set parameters for launching the application */
  if (NULL == (mchn = (int *) malloc (nodes * sizeof(int))))                    goto exception2;
  
  /* 3. Algorithm to assign MPI nodes to hosts */
  if (0 > PMII_spreadNodesOnMachines (mchn, nodes))                             goto exception;
  
  /* 4. Create and launch group of nodes */
  if(0 > (GRP_create (&gix, mchn, nodes, getCpuId())))                          goto exception;
  commAttr.Flags = COMMATTR_FLAGS_SLM_ENABLED;
  commAttr.Flags = 0;
#warning "azqmpi/private/main.c TIENE LOS FLAGS A 0 POR LA LFQ"

  /* 5. Herman:Set parameters for mpise: allocate mem to hold each operation parameters*/
  if(NULL== (arg_array= malloc(nodes*sizeof(arg_t *))))							  goto exception2;
  if(NULL== (func_cnt_array= calloc(nodes,sizeof(int))))					      goto exception2;


  for(i=0;i<nodes; i++){
	  if(NULL== (arg_array[i]= (arg_t*) calloc(MAX_MPI_CALLS, sizeof(arg_t))))
	  {
		  for(j=0;j<=i;j++){
			  free(arg_array[j]);
		  }
		  free(func_cnt_array);
		  goto exception2;
	  }
  }

  for(i = 0; i < nodes; i++) {
    if(0 > (GRP_join (gix, i, opr_nr, &commAttr)))                              goto exception;
  }

  if(0 > (GRP_start (gix)))                                                     goto exception2;
  if (NULL == (retcode = (int *) malloc (nodes * sizeof(int))))                 goto exception2;
  
  GRP_wait(gix, retcode);
  
  ret = 0;
  for (i = 0; i < nodes; i++) {
    if (retcode[i] != 0) {
      ret = retcode[i];
      break;
    }
  }
  GRP_shutdown();
  free(retcode);
  free(mchn);
  //klee_enable_sync_chk(0);
  return ret;
  
  
exception3:
  fprintf(stderr, "ERROR: (AZQ_clnt) Getting environment values\n");
  
exception2:
  fprintf(stderr, "ERROR: (AZQ_clnt) Allocating memory\n");
  
exception:
  fprintf(stderr, ">>> Exception raised in AZQ_clnt\n");
  GRP_shutdown();
  if(!retcode) free(retcode);
  free(mchn);
  //klee_enable_sync_chk(0);
  return(-1);
}

int mpse_finalnize()
{
	if(func_cnt_array) free(func_cnt_array);
	if(arg_array){
	   for(int i=0; i<PMII_getNodeNr();i++){
		   if(arg_array[i]->buffer)
			   free(arg_array[i]->buffer);
		   if(arg_array[i]->request)
			   free(arg_array[i]->request);
		   if(arg_array[i]->status)
		   			   free(arg_array[i]->status);
		   free(arg_array[i]);
	   }
	   free(arg_array);
	}
	delete_hash(table);
    return 0;
}
      /*________________________________________________________________
     /                                                                  \
    |    main                                                            |
    |    C main() function. All starts here                              |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
#ifdef main
#undef main
#endif

#if defined (__OSI)
int OSI_main(int argc, char *argv[]) {
#else
int main(int argc, char *argv[]) {
#endif
  REG_Entry     oprTable;
  /* 1. Fill the operators type table and register it */
  opr_nr = rand();
  oprTable.Name        = opr_nr;
  oprTable.Function    = node_main__;
  oprTable.Stack_Size  = DEFAULT_STACK_SIZE;
  oprTable.Argc        = argc;
  oprTable.Argv        = argv;

  //klee_disable_sync_chk(0);
  /* 2. Init Azequia */
  table=  (struct hash_table *) calloc (1, sizeof (struct hash_table));
  init_hash (table, 200);

  int res =AZQ_init(&oprTable, 1);
  if(0 > res) {
    //fprintf(stderr, "ERROR: [main] Cannot start AZEQUIA, errono %d\n", res);
    return(-1);
  }

#ifdef VERBOSE_MODE
  int           who    = RUSAGE_SELF;
  struct rusage usage;

  if ((getCpuId() == 0) && (0 == getrusage(who, &usage))) {
    fprintf(stdout, "Execution statistics\n");
    fprintf(stdout, "\tuser time used:                %lf\n", ((double) usage.ru_utime.tv_sec + 1.0e-9 * (double) usage.ru_utime.tv_usec));
    fprintf(stdout, "\tsystem time used:              %lf\n", ((double) usage.ru_stime.tv_sec + 1.0e-9 * (double) usage.ru_stime.tv_usec));
    fprintf(stdout, "\tpage reclaims:                 %ld\n", usage.ru_minflt);
    fprintf(stdout, "\tpage faults:                   %ld\n", usage.ru_majflt);
    fprintf(stdout, "\tvoluntary context switches:    %ld\n", usage.ru_nvcsw);
    fprintf(stdout, "\tinvoluntary context switches:  %ld\n", usage.ru_nivcsw);
  }
#endif
  //klee_enable_sync_chk(0);
  mpse_finalnize();
  return EXIT_SUCCESS;
}


