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
   |  Date:    Feb 15, 2010                                                |
   |                                                                       |
   |  Description:                                                         |
   |                                                                       |
   |                                                                       |
   |_______________________________________________________________________| */

  /*----------------------------------------------------------------/
 /   Declaration of public functions implemented by this module    /
/----------------------------------------------------------------*/
#include <env.h>

  /*----------------------------------------------------------------/
 /   Declaration of types and functions used by this module        /
/----------------------------------------------------------------*/
#if defined(__OSI)
  #include <osi.h>
#else
  #include <stdio.h>
  #include <string.h>
  #include <pthread.h>
#endif

#include <thr.h>
#include <grp.h>
#include <com.h>
#include <azq_types.h>

#include <errhnd.h>
#include <p_config.h>

#include <p_group.h>

  /*-------------------------------------------------------/
 /                   Private constants                    /
/-------------------------------------------------------*/
/* Private module error codes */
PRIVATE char *e_names[8] = { /*  0 */ "PCS_E_OK",
                             /*  1 */ "PCS_E_EXHAUST",
                             /*  2 */ "PCS_E_INTEGRITY",
                             /*  3 */ "PCS_E_TIMEOUT",         /* This order has to be consistent */
                             /*  4 */ "PCS_E_INTERFACE",       /* with env.h                      */
                             /*  5 */ "PCS_E_SYSTEM",
                             /*  6 */ "PCS_E_DISABLED",
                             /*  7 */ "PCS_E_INTERN"
                           };

  /*-------------------------------------------------------/
 /           Private structs for applications             /
/-------------------------------------------------------*/
/* Table of endpoints */
struct ProcTable {
  Process      *Procs;  /* Table of Processes (endpoints are implemented as threads in AzqMPI */
  int           Size;  
  int           Count;  /* How many processes are initialized now */
};

  /*-------------------------------------------------------/
 /                    Private data                        /
/-------------------------------------------------------*/
PRIVATE ProcTable       *EndPoints;
PRIVATE int              initialized  = FALSE;
PRIVATE pthread_mutex_t  appmtx       = PTHREAD_MUTEX_INITIALIZER;

  /*-------------------------------------------------------/
 /      Private functions implemented by this module      /
/-------------------------------------------------------*/
PRIVATE int   pcsAllocTable           (ProcTable_t *pcstable, int nrprocs);
PRIVATE int   pcsFreeTable            (ProcTable_t *pcstable);

/* There are some global data to all endpoints (threads) in the same node */
PRIVATE int   pcsCreateGlobalData     ();
PRIVATE int   pcsDeleteGlobalData     ();

/* Most data is private to an andpoint (thread) */
PRIVATE int   pcsCreatePrivateData    (Process *pcs, int thread_level_support);
PRIVATE int   pcsDeletePrivateData    (Process *pcs);

  /*----------------------------------------------------------------------/
 /    Implementation of private functions for application management     /
/----------------------------------------------------------------------*/
/**
 *  pcsAllocTable
 *    Allocate the table of processes
 */
PRIVATE int pcsAllocTable (ProcTable_t *pcstable, int nrprocs) {

  ProcTable *pcst;
  Process   *pcs;

  /* 1. Allocate process table structures */
  if (posix_memalign((void *)&pcs, CACHE_LINE_SIZE, nrprocs * sizeof(Process)))
    return PCS_E_EXHAUST;

  memset(pcs, 0, nrprocs * sizeof(Process));
  
  if (posix_memalign((void *)&pcst, CACHE_LINE_SIZE, sizeof(ProcTable))) {
    free(pcs);
    return PCS_E_EXHAUST;
  }

  /* 2. Set fields */
  pcst->Procs = pcs;
  pcst->Size  = nrprocs;
  pcst->Count = 0;

  /* 3. Table of groups to return */
  *pcstable = pcst;

  return (PCS_E_OK);
}


/**
 *  pcsFreeTable
 *    Free the table of processes
 */
PRIVATE int pcsFreeTable (ProcTable_t *pcstable) {

  Process  *pcst;

  pcst = (*pcstable)->Procs;

  free(pcst);
  free(*pcstable);

  *pcstable = (ProcTable *)NULL;

  return PCS_E_OK;
}


/**
 *  pcsCreateGlobalData
 *    This function creates the data shared between all endpoints (threads) in 
 *    the same node. Sharing data can improve memory use. By now, data shared are:
 *    
 *      - Array of members of group WORLD
 *      - Default datatypes
 *      - Default collective operations
 *
 *    Table of MPI Processes and access to each endpoint entry is created too.
 *
 */
PRIVATE int pcsCreateGlobalData () {  

  char  *where   = "pcsCreateGlobalData";
  int    err;
  int    nrprocs;

  /* 1. This pthread key is used for each thread (endpoint) accesses its entry 
     in the table */
  if(pthread_key_create(&key_self, NULL))                                       
	return PCS_E_SYSTEM;
  
  /* 2. Allocate the process table for the application */
  GRP_getSize(getGroup(), &nrprocs);
  if (0 > (err = pcsAllocTable(&EndPoints, nrprocs)))                           goto exception1;
 
  /* 3. Create data shared between endpoints in a node */
  if (0 > groupCreateDefault())                                                 goto exception2;
  if (0 > dtypeCreateDefault())                                                 goto exception3;
  if (0 > copsCreateDefault())                                                  goto exception4;

  return PCS_E_OK;
  
exception4:
  dtypeDeleteDefault();
  
exception3:
  groupDeleteDefault();
  
exception2:
  pcsFreeTable(EndPoints);

exception1:
  pthread_key_delete(key_self);
  
exception:
  XPN_print(err);
  return(err);
}


/**
 *  pcsDeleteGlobalData
 *    Delete global data shared between threads (endpoints)
 *
 */
PRIVATE int pcsDeleteGlobalData () {

  /* 1. Delete global MPI data */
  copsDeleteDefault();
  dtypeDeleteDefault();
  groupDeleteDefault();

  /* 2. Delete table of processes (endpoints) */
  pcsFreeTable(&EndPoints);

  /* 3. Delete pthread key for accessing table entries */
  pthread_key_delete(key_self);
 
  return PCS_E_OK;
}


/**
 *  pcsCreatePrivateData
 *
 *    Most data in AzqMPI is private to a thread (endpoint). 
 */
PRIVATE int pcsCreatePrivateData (Process *pcs, int thread_level_support) {

  char             *where     = "pcsCreatePrivateData";
  int               err;


  /* 1. Error handlers table */
  if (0 > (err = errhndAllocTable (&pcs->ErrHndTable)))                              goto exception;

  /* 2. Communicators table */
  if (0 > (err = commAllocTable (&pcs->CommTable)))                                  goto exception1;
  
  /* 3. Groups table */
  if (0 > (err = groupAllocTable (&pcs->GroupTable)))                                  goto exception2;

  /* 4. Collective operations table */
  if (0 > (err = copsAllocTable (&pcs->CopsTable)))                                  goto exception3;

  /* 5. Requests */
  if (0 > (err = rqstAllocTable (&pcs->RqstTable)))                                  goto exception4;

  /* 6. Keys table */
  if (0 > (err = keysAllocTable (&pcs->KeysTable)))                                  goto exception5;

  /* 7. Derived user-defined datatypes */
  if (0 > (err = dtypeAllocTable (&pcs->DTypeTable)))                                goto exception6;

  /* 8. Set the pthread key pointing to self entry in the endpoints table */
  pthread_setspecific(key_self, pcs);

  /* 9. Nestlevel counts the nest level in function calls for a thread */
  pcs->NestLevel = 0;
  
  /* 10. Level of thread supported */
  pcs->ThreadLevelSupported = thread_level_support;

  pcs->Initialized = TRUE;


  return PCS_E_OK;

exception6:
  keysFreeTable(&pcs->KeysTable);

exception5:
  rqstFreeTable(&pcs->RqstTable);

exception4:
  copsFreeTable(&pcs->CopsTable);

exception3:
  groupFreeTable(&pcs->GroupTable);

exception2:
  commFreeTable(&pcs->CommTable);

exception1:
  errhndFreeTable(&pcs->ErrHndTable);

exception:
  XPN_print(err);
  return(err);
}


/**
 *  pcsDeletePrivateData
 */
PRIVATE int pcsDeletePrivateData (Process *pcs) {

  if (!pcs->Initialized)                         return PCS_E_INTEGRITY;

  errhndFreeTable (&pcs->ErrHndTable);
  commFreeTable   (&pcs->CommTable);
  groupFreeTable  (&pcs->GroupTable);
  copsFreeTable   (&pcs->CopsTable);
  rqstFreeTable   (&pcs->RqstTable);
  keysFreeTable   (&pcs->KeysTable);
  dtypeFreeTable  (&pcs->DTypeTable);

  /* MPI 1.2: is_init return true after init, even if finalize is called
  pcs->Initialized = FALSE; */

  return PCS_E_OK;
}


  /*-----------------------------------------------------------------/
 /   Implementation of exported interface for processes management  /
/-----------------------------------------------------------------*/
/**
 *
 *  PCS_init()
 *    Initialize all the structures needed by an endpoint.
 *    
 *    And endpoint is a process in MPI, and a thread in AzqMPI. 
 *    AzqMPI preserves the name "Process" for an endpoint.
 *    
 *    Endpoints has its own private data for MPI objects (communicators,
 *    groups, datatypes, etc.), but some of the defaults objects can be
 *    shared between endpoints because are implemented as threads. This
 *    can save memory.
 *    A process containing several endpoints (threads) is run in each node
 *    or machine.
 *
 */
int PCS_init(int thread_level_support) {
  
  char        *where        = "PCS_init";
  Process	  *new_pcs;
  int          err;
  Thr_t        me           = (Thr_t)pthread_getspecific(key);//self();
  
  /* 1. Data shared between threads in the same process */
  pthread_mutex_lock(&appmtx);
  
  if (!initialized) {
    initialized = TRUE;
	/* 1. Only the first thread create the global data */
  	if (0 > (err = pcsCreateGlobalData()))                                      goto exception;	
  }
  
  /* 2. Check if threads is already initialized */
  new_pcs = &EndPoints->Procs[getRank()];
  if (new_pcs->Initialized)                                                     {err = PCS_E_INTEGRITY;
	                                                                             goto exception;}
  EndPoints->Count++;
  
  pthread_mutex_unlock(&appmtx);
  
  /* 3. Create data private to an endpoint */
  if(0 > (err = pcsCreatePrivateData(new_pcs, thread_level_support)))           goto exception;
  //me->RqstTable = (void *)(new_pcs->RqstTable);
  
  return PCS_E_OK;
  
exception:
  pthread_mutex_unlock(&appmtx);
  XPN_print(err);
  return(err);
}


/**
 *
 *  PCS_finalize()
 *    Delete all data for a process/endpoint/thread
 *
 */
int PCS_finalize() {

  char        *where = "PCS_finalize";

  /* 1. Delete process private data */
  if (0 > pcsDeletePrivateData(PCS_self()))                                     goto exception;

  /* 2. Delete process global data (only the last process) */  
  pthread_mutex_lock(&appmtx);
  
  EndPoints->Count--;
  if (EndPoints->Count == 0) {
    if (0 > pcsDeleteGlobalData())                                                goto exception;
  }
  
  pthread_mutex_unlock(&appmtx);

  
  return PCS_E_OK;

exception:
  XPN_print(PCS_E_INTEGRITY);
  return(PCS_E_INTEGRITY);
}


/**
 *  PCS_isInit
 */
int PCS_isInit () {

  Process *pcs = PCS_self();

  if (pcs == NULL)  return 0;

  return pcs->Initialized;
}



/*
 *  PCS_getEnv
 *
 *   Return execution environment of an endpoint
 */ 
int PCS_getEnv(int *node, int *nodes_nr, int *node_group_nr) {
  
  *node     = getCpuId();
  *nodes_nr = INET_getNodes();
  GRP_getLocalSize(getGroup(), node_group_nr);
  
  return PCS_E_OK;
}


/*
 *  PCS_getNode
 *
 *    Return node where a global rank (MPI_COMM_WORLD) is running 
 */ 
int PCS_getNode(int global_rank, int *rank_node) {
  
  GRP_getMchn(getGroup(), global_rank, rank_node);
  
  return PCS_E_OK;
}



/**
 *  PCS_groupSet
 */
int PCS_groupSet (int grp_index, Mpi_P_Group *grp) {
  
  if      (grp_index == GROUP_WORLD)
    PCS_self()->GroupTable->World = grp;
  else if (grp_index == GROUP_EMPTY)
    PCS_self()->GroupTable->Empty = grp;
  else if (grp_index == GROUP_SELF)
    PCS_self()->GroupTable->Self = grp;
  else 
	return PCS_E_INTEGRITY;

  return PCS_E_OK;
}


/*
 *  PCS_commDelete
 */
int PCS_commDelete (Mpi_P_Comm_t *comm) {

  Process           *pcs = PCS_self();
  Mpi_P_Errhandler  *errhnd;

  commGetErrHnd(*comm, (void *)&errhnd);
  errhndDelete(pcs->ErrHndTable, &errhnd);
  return commDelete(pcs->CommTable, comm);
}


/*
 *  PCS_commSet
 *    Set the default communicators
 */
int PCS_commSet (int comm_index, Mpi_P_Comm *comm) {
  
  if (comm_index == COMM_WORLD)
    PCS_self()->CommTable->World = comm;
  else if (comm_index == COMM_SELF)
    PCS_self()->CommTable->Self = comm;
  else 
	return PCS_E_INTEGRITY;

  
  return PCS_E_OK;
}


/*
 *  PCS_errhndSet
 */ 
int PCS_errhndSet (int errhnd_index, Mpi_P_Errhandler *errhnd) {

  if      (errhnd_index == FATAL_ERROR)
    PCS_self()->ErrHndTable->Fatal  = errhnd;
  else if (errhnd_index == RETURN_ERROR)
    PCS_self()->ErrHndTable->Return = errhnd;
  else 
	return PCS_E_INTEGRITY;
 
  return PCS_E_OK;
}



