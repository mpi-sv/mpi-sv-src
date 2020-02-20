/* _________________________________________________________________________
   |                                                                       |
   |  Azequia Message Passing Interface   ( AzequiaMPI )                   |
   |                                                                       |
   |  Authors: DSP Systems Group                                           |
   |           http://gsd.unex.es                                          |
   |           University of Extremadura                                   |
   |           Caceres, Spain                                              |
   |           jarico@unex.es                                              |
   |                                                                       |
   |  Date:    Jan 22, 2011                                                |
   |                                                                       |
   |  Description:                                                         |
   |                                                                       |
   |                                                                       |
   |_______________________________________________________________________| */


  /*----------------------------------------------------------------/
 /   Declaration of types and functions used by this module        /
/----------------------------------------------------------------*/
#include <pmi_interface.h>
#include <pmi.h>

#include <config.h>

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

/*----------------------------------------------------------------*
 *   Definition of private constants                              *
 *----------------------------------------------------------------*/

/* Thread not bind to a processor */
#define THR_NO_BIND  (-1)


/*----------------------------------------------------------------*
 *   Private functions implemented by this module                 *
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------*
 *   Referenced functions implemented by other module             *
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------*
 *   Definition of private data                                   *
 *----------------------------------------------------------------*/

/* Module initialized */
static int initialized = FALSE;

/* Number of MPI nodes */
static int  nodeNr = 0;

/* -perhost parameter to mpiexec. 
   Number of nodes in this host before jump to the next host */
static int  nodesPerHost = 1;

/* Number of hosts in the ring */
static int  hostNr = 0;

/* -bysocket parameter to mpiexec
   Bind nodes by socket in round robin */
static int  bySocket = 0;

/* -no-binding parameter to mpiexec
   No bind nodes to cores. Run them free */
static int  noBinding = 0;


  /*----------------------------------------------------------------/
 /            Implementation of private functions                  /
/----------------------------------------------------------------*/

      /*________________________________________________________________
     /                                                                  \
    |    get_puPerHost                                                   |
    |                                                                    |
    |    Get the number of logical processors in each host of the ring   |
    |    An homgeneous cluster is assumed                                |
    |                                                                    |
     \____________/  ___________________________________________________/
                 / _/
                /_/
               */
#ifdef HAVE_LIBHWLOC
static int get_puPerHost() {
  
  int               depth;
  int               puPerHostNr;
  hwloc_topology_t  topology;
  
  hwloc_topology_init(&topology);
  hwloc_topology_load(topology);
  depth       = hwloc_get_type_or_below_depth(topology, HWLOC_OBJ_CORE);
  puPerHostNr = hwloc_get_nbobjs_by_depth    (topology, depth);
  hwloc_topology_destroy(topology);
  
  return puPerHostNr;
}
#endif

  /*----------------------------------------------------------------/
 /   Implementation of public functions                            /
/----------------------------------------------------------------*/

int PMII_init() {

  char  *p;
  int    spawned;
  int    err;
  
  
  if (PMI_SUCCESS != (err = PMI_Init(&spawned))) {
	fprintf(stderr, "PMII_Init failed with error = %d\n", err);
	goto exception;
  }   
  
  /* 1. Get environment values */
  if (NULL == (p = getenv("AZQMPI_NODES")))                                     goto exception;
  nodeNr = atoi(p);
  if (nodeNr <= 0)                                                              goto exception;
  
  if (NULL != (p = getenv("AZQMPI_PERHOST"))) {
    nodesPerHost = atoi(p);
    if (nodesPerHost <= 0)                                                      goto exception;
  }
  
  if (NULL == (p = getenv("AZQMPI_HOSTS")))                                     goto exception;
  hostNr = atoi(p);
  if (hostNr <= 0)                                                              goto exception;
  
  if (NULL == (p = getenv("AZQMPI_BYSOCKET")))                                  goto exception;
  bySocket = atoi(p);

  if (NULL == (p = getenv("AZQMPI_NO_BINDING")))                                goto exception;
  noBinding = atoi(p);

  
  return EXIT_SUCCESS;

exception:
  fprintf(stderr, "%s [%d]: ERROR:\n", __FILE__, __LINE__);
  return (EXIT_FAILURE);
}


void  PMII_finalize () {
  PMI_Finalize();
}



int PMII_getNodeNr () {
  
 /* if (!initialized) {
	initialized = TRUE;
	if (0 > getEnvValues())   return EXIT_FAILURE;
  }
  */
  return nodeNr;
}


int PMII_spreadNodesOnMachines (int *mchn, int nodenr) {
  
  int  i;
  
  for(i = 0; i < nodenr; i++) {
    mchn[i] = (i / nodesPerHost) % hostNr;
  }
  
#ifdef VERBOSE_MODE
  {
	int k;
	fprintf(stdout, "-------------------------------------------------------\n");
	fprintf(stdout, "Hosts in the ring:   %d\n", hostNr);
#ifdef HAVE_LIBHWLOC
	fprintf(stdout, "PUs per Host:        %d\n", get_puPerHost());
#endif
	fprintf(stdout, "MPI Nodes per Host:  %d\n", nodesPerHost);
	fprintf(stdout, "MPI nodes:           %d\n", nodenr);
	for (k = 0; k < nodenr; k++)
	  fprintf(stdout, "host[node %d]  =  %d\n", k, mchn[k]);
	fprintf(stdout, "-------------------------------------------------------\n"); 
	fflush(stdout);
  }
#endif
// Added by Herman. Fix stupid exception raised when this function do not return things...
  return 0;
  
}



#ifdef HAVE_LIBHWLOC
pthread_mutex_t  grp_hwloc_mtx = PTHREAD_MUTEX_INITIALIZER;
#include <hwloc.h>
#endif

int PMII_setBindParams (Placement_t place, int globalrank, int localrank) {
  
#ifdef HAVE_LIBHWLOC
  static
  hwloc_topology_t  topology;
  static
  int               pusPerHost;
  static
  int               socksPerHost;
  static
  int               hwloc_initialised = 0;
  hwloc_cpuset_t    cpuset;
  int               pu = -1;
  int               depth;
  int               socketnr = -1;
  char             *p;
  int               punr;
  static
  int               pusPerSocket;
  
  
  if (noBinding) {
#ifdef __HWLOC_VERBOSE
	
	fprintf(stdout, "Node [%d] at [Machine#%d] Not Bind\n", 
			globalrank, getCpuId());
  
#endif  
	return 0;
  }
  
  pthread_mutex_lock(&grp_hwloc_mtx);
  
  if(!hwloc_initialised) {
	hwloc_topology_init(&topology);
	hwloc_topology_load(topology);
	
	depth        = hwloc_get_type_or_below_depth(topology, HWLOC_OBJ_CORE);
	pusPerHost   = hwloc_get_nbobjs_by_depth    (topology, depth);
	
	depth        = hwloc_get_type_or_below_depth(topology, HWLOC_OBJ_SOCKET);
	socksPerHost = hwloc_get_nbobjs_by_depth    (topology, depth);
		
	pusPerSocket = pusPerHost / socksPerHost;
	
	hwloc_initialised = 1;
  }
  
  
  /* ALGORITHM to calculate the PU assigned to an AZEQUIA node given its local rank:
   *    PU(localRank) = localRank % PUS_PERHOST
   */
  if (bySocket) {
	
	socketnr = localrank % socksPerHost;
	punr     = (localrank / socksPerHost) % pusPerSocket;
	pu = (socketnr * pusPerSocket) + punr;
	
  } else {
	
	socketnr = (localrank / pusPerSocket) % socksPerHost;
	pu = localrank % pusPerHost;
	
  }
  
#ifdef __HWLOC_VERBOSE
  
  fprintf(stdout, "Node [%d] at [Machine#%d,Socket#%d,Core#%d]\n", 
		  localrank, getCpuId(), socketnr, pu);
  
#endif
  
  place->Socket     = socketnr;
  place->Core       = pu;
  place->LocalRank  = localrank;
  place->GlobalRank = globalrank;
  
  
  pthread_mutex_unlock(&grp_hwloc_mtx);
#endif
  
  return 0;
  
}



/*________________________________________________________________
 /                                                                  \
 |    bindThr2Processor                                               |
 |                                                                    |
 |    Bind the thread to the logical processor corresponding to its   |
 |    rank                                                            |
 |                                                                    |
 \____________/  ___________________________________________________/
 / _/
 /_/
 */
#ifdef HAVE_LIBHWLOC
pthread_mutex_t  hwloc_mtx = PTHREAD_MUTEX_INITIALIZER;
#include <hwloc.h>
#endif

int PMII_bindSelf(Placement_t place) {
  
#ifdef HAVE_LIBHWLOC
  static
  hwloc_topology_t  topology;
  static
  int               hwloc_initialised = 0;
  hwloc_cpuset_t    cpuset;
  hwloc_obj_t       obj;
  int               pu;
  int               depth;
  int               err = THR_E_OK;
  
  
  if (noBinding) {
#ifdef __HWLOC_VERBOSE
	
	fprintf(stdout, "Node [%d] at [Machine#%d] Not Bind\n", 
			place->GlobalRank, getCpuId());
	
#endif  
	return 0;
  }
  
  pthread_mutex_lock(&hwloc_mtx);
  
  if(!hwloc_initialised) {
    hwloc_topology_init(&topology);
    hwloc_topology_load(topology);
    hwloc_initialised = 1;
  }
  
  
  if(place->Binded != THR_NO_BIND) {

	pu = place->Core;
	
    /* Put the thread on its intended logical processor pu */
	obj = hwloc_get_obj_by_type(topology, HWLOC_OBJ_CORE, pu);
	
	if (obj) {
	  cpuset = hwloc_cpuset_dup(obj->cpuset);
	  hwloc_cpuset_singlify(cpuset);
	  
	  if (0 > (err = hwloc_set_thread_cpubind(topology, pthread_self(), cpuset,
											  HWLOC_CPUBIND_STRICT | HWLOC_CPUBIND_THREAD))) {
		
#ifdef __HWLOC_VERBOSE
		
		char *str;
		
		hwloc_cpuset_asprintf(&str, cpuset);
		fprintf(stdout, "FAILED to bind Node [%d] to [Machine#%d,Core#%d] (Error: %d) (cpuset: %s)\n", 
				place->GlobalRank, getCpuId(), pu, err, str);
		fflush(stdout);
		free(str);
		
#endif
		
		pthread_mutex_unlock(&hwloc_mtx);
		return -1;
	  }
	  
#ifdef __HWLOC_VERBOSE
	  
	  char *str;
	  
	  hwloc_cpuset_asprintf(&str, cpuset);
	  fprintf(stdout, "hwloc_set_thread_cpubind return: %d  (cpuset: %s)\n", err, str);
	  free(str);
	  
	  fprintf(stdout, "Node [%d] at [Machine#%d,Core#%d]\n", place->GlobalRank, getCpuId(), pu);
	  
#endif
	  
	  hwloc_cpuset_free(cpuset);
	}
  }
  
  pthread_mutex_unlock(&hwloc_mtx);
#endif
  
  return 0;
}


void PMII_setUnBind(Placement_t place) {
  place->Binded = THR_NO_BIND;
}

