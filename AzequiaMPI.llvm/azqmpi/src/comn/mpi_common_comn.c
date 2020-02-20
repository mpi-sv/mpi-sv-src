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
#include <common.h>

#include <mpi.h>
#include <env.h>
#include <pthread.h>


  /*----------------------------------------------------------------/
 /               Declaration of private functions                  /
/----------------------------------------------------------------*/
PRIVATE int commCreateMCC (Mpi_P_Comm *comm);
PRIVATE int commFreeMCC   (Mpi_P_Comm *comm);


  /*----------------------------------------------------------------/
 /               Implementation of public interface                /
/----------------------------------------------------------------*/
/*
 * comm_create
 */
int comm_create (MPI_Comm intracomm, MPI_Group local_group, MPI_Group remote_group,
						int context, int type, void *errhnd, 
						MPI_Comm *newcomm)   {
  
  int  mpi_errno;
  
  
  CALL_FXN (PCS_commCreate (intracomm, local_group, remote_group, DEFAULT_ATTRIBUTES_COUNT, 
							context, type, errhnd, newcomm), MPI_ERR_COMM);
  
  commCreateMCC(*newcomm);
  
  
  return MPI_SUCCESS;
  
mpi_exception_unnest:
  return mpi_errno;
}


/*
 * comm_free
 */
int comm_free (MPI_Comm *comm) {

  int  mpi_errno;
  
  
  if (commMCCSupport(*comm)) 
    commFreeMCC(*comm);

  CALL_FXN (PCS_commDelete(comm), MPI_ERR_INTERN);
  
  
  return MPI_SUCCESS;
  
mpi_exception_unnest:
  return mpi_errno;
}


  /*-------------------------------------------------------/
 /         Implementation of private interface            /
/-------------------------------------------------------*/
/*
 *  commCreateMCC:
 *   Create multicore clusters auxiliary communicators for collective support
 *
 *   Created:
 *   - Group_leaders: Group containing leaders of each machine involved in comm.
 *          SIZE: Number of nodes involved in comm
 *          COPIES: One per leader rank (one per node involved)
 *   - Group_locals: Group containing locals to each machine in comm.
 *          SIZE: Number of nodes in a machine
 *          COPIES: One per rank in the machine
 *   - Node_Leaders: Array containing relative ranks (in leaders group) of each node
 *          SIZE: Total number of nodes
 *          COPIES: One per leader
 *
 *   Temporarily, node_leaders is created per rank in comm, but it is freed at the end of the function
 */
PRIVATE int commCreateMCC (Mpi_P_Comm *comm) {
  
  int            nodes_nr;         /* Number of nodes in cluster */
  int            nodes_use_nr = 0; /* Number of nodes really involved by the new leader communicator */
  int           *leaders;          /* Array of global rank leaders (for group leaders) */
  int           *locals;           /* Array of global rank locals (for group locals) */
  int           *node_leaders;     /* Array of relative rank leaders in each node (for leaders in other machine to know) */
  int            i, j, k; 
  int            global_rank;
  int            node;             /* Node (machine number from 0 to nodes_nr-1) in which rank is running */
  int            inlead   = FALSE; /* Am I a leader */
  int            rank_node;        /* Node where a rank is running (Azequia layer support) */
  int            context;          /* Context of the communicator from which create MCC communicators */
  int            comm_size;        /* Size of communicator */
  int            node_group_nr;    /* Number of ranks in the MPI_COMM_WORLD running on this machine */
  Mpi_P_Comm_t   comm_leaders;     /* Leaders communicator to create (one per leader) */
  Mpi_P_Comm_t   comm_locals;      /* Locals communicators to create (one per rank in comm) */
  Mpi_P_Group_t  commgroup;        
  Mpi_P_Group_t  group_leaders;
  Mpi_P_Group_t  group_locals;
  
  /* 0. Default values */
  comm->Comm_Leaders   = MPI_COMM_NULL;
  comm->Comm_Locals    = MPI_COMM_NULL;
  comm->MCC_Support    = FALSE;
  comm->SMP_Support    = FALSE;
  comm->Node_Leaders   = NULL;
  
  /* 1. Get data */
  context    = commGetContext(comm);
  commgroup  = commGetGroup(comm);
  comm_size  = commGetSize(comm);
  
  PCS_getEnv(&node, &nodes_nr, &node_group_nr);
  
  /* 2. No MCC communicators if only one machine */
  if (nodes_nr < 1)                                                             return COM_E_INTEGRITY;
  if (nodes_nr == 1) {
    /* 2.1. SMP operations can be used */
    if (comm_size > 1) { 
      comm->SMP_Support  = TRUE;
      comm->ReduceMyTurn = 0;
      if (commGetRank(comm) == 0) { 
        int i;
        comm->comm_root       = comm; 

        /* Initialise the Communicator Barrier */
        comm->Barrier.Counter = 0;
        comm->Barrier.Flag    = 0;
        for(i = 0; i < comm_size; i++)
          comm->Barrier.Sense[i].Sense   = 0;

        /* Initialize the communicator reduce spinLocks */
        for(i = 0; i < 128; i++) {
          comm->ReduceLock[i].Cerrojo = 0;
        }
        comm->ReduceRootWaitCnt = comm_size;
        comm->ReduceRootWait = comm_size - 1;
        comm->ReduceRootRecvBuf = NULL;
        comm->ReduceTurn        = 0;
      } 
      MPI_Bcast(&comm->comm_root, sizeof(MPI_Aint), MPI_BYTE, 0, comm);
    }
    return COM_E_OK;
  }
  
  /* 3. Allocate space */
  if (posix_memalign(&node_leaders, CACHE_LINE_SIZE, nodes_nr      * sizeof(int)))  return COM_E_EXHAUST;
  if (posix_memalign(&locals,       CACHE_LINE_SIZE, node_group_nr * sizeof(int)))    goto exception;
  
  for (i = 0; i < nodes_nr; i++) 
	node_leaders [i] = -1;
  
  /* 4. Create groups leaders and locals */
  k = 0;
  for (i = 0; i < comm_size; i++) {
	
	/* 4.1. Azequia layer gives the node where rank i is running */
	global_rank = groupGetGlobalRank(commgroup, i);
	PCS_getNode(global_rank, &rank_node);
    
	/* 4.2. Leaders of each node. -1 if node is not involved in comm */
	if (node_leaders[rank_node] == -1) {
      nodes_use_nr++;
	  if (getRank() == global_rank) inlead = TRUE;
	  
  	  node_leaders[rank_node] = global_rank;
	}
	
	/* 4.3. Locals to each node. Every rank creates its own locals group */
	if (node == rank_node) {
      locals[k++] = global_rank;
    }
	
  }
  
  /* 5. Do it need MCC support? 
   Conditions can be added, depending on particular architecture */
  if ((nodes_use_nr <= 1) || (nodes_use_nr == comm_size))                       goto exception_1;
  
  /* 6. Communicators for leaders in each machine involved */
  if (inlead) {
	
	/* 6.1. Leaders need create a groups of leaders in each node */
	if (posix_memalign(&leaders, CACHE_LINE_SIZE, nodes_use_nr  * sizeof(int))) goto exception_1;
	
	j = 0;
	for (i = 0; i < nodes_nr; i++) {
      if (node_leaders[i] != -1) {
		leaders[j] = node_leaders[i];
		/* 6.2. Node_leaders comes with global ranks. Fill it with relative ranks (to leaders group) */
		node_leaders[i] = j;
		j++;
	  } 
      if (j == nodes_use_nr) break;
    }
	
	
	/* 6.3. Create groups for leaders */
    if (0 > PCS_groupCreate(leaders, j, &group_leaders))                        goto exception_2;
	
	/* 6.4. Create communicator leaders */
	if (0 > PCS_commCreate (comm, group_leaders, NULL, 0, 
							context + 1, INTRACOMM, NULL, &comm_leaders))       goto exception_3;
	
	comm->Comm_Leaders  = comm_leaders;
	comm->Node_Leaders  = node_leaders;
	
	FREE(leaders);
	
  } else {
	/* 6.4. Ranks not in leaders do not need node_leaders array */
	FREE(node_leaders);
  }
  
  /* 7. Communicators locals to each machine involved */
  if (k > 1) {
    /* 7.1. Create groups with locals ranks to each node */
    if (0 > PCS_groupCreate(locals, k, &group_locals))                          goto exception_4;
    
    /* 7.2. Create communicator locals */
    if (0 > PCS_commCreate (comm, group_locals, NULL, 0, 
							context + 2, INTRACOMM, NULL, &comm_locals))        goto exception_5;
    comm->Comm_Locals  = comm_locals;
	
    /* 7.3. SMP operations can be used */
    comm_locals->SMP_Support = TRUE;
    if (commGetRank(comm_locals) == 0) { 
      int i;
      comm_locals->comm_root       = comm; 

      /* Initialise the Communicator Barrier */
      comm_locals->Barrier.Counter = 0;
      comm_locals->Barrier.Flag    = 0;
      for(i = 0; i < comm_size; i++)
        comm_locals->Barrier.Sense[i].Sense   = 0;
    }
    MPI_Bcast(&comm_locals->comm_root, sizeof(MPI_Aint), MPI_BYTE, 0, comm_locals);
  } 
  
  FREE(locals);
  
  /* 8. Return */  
  comm->MCC_Support = TRUE;
  
  return COM_E_OK;
  
exception_5:
  PCS_groupDelete(&group_locals);
  
exception_4:
  PCS_commDelete(comm_leaders);
  
exception_3:
  PCS_groupDelete(&group_leaders);
  
exception_2:
  FREE(leaders);
  
exception_1:
  FREE(locals);
  
exception:
  FREE(node_leaders);
  
  return COM_E_EXHAUST;
}


/*
 *  commFreeMCC:
 *   Free support for multicore collectives
 */
PRIVATE int commFreeMCC (Mpi_P_Comm *comm) {
  
  Mpi_P_Comm *leaders = commMCCGetLeaders(comm);
  Mpi_P_Comm *locals  = commMCCGetLocals(comm);
  
  if (leaders != (Mpi_P_Comm *)NULL) {
    leaders->Type = NOT_ALLOCATED;
	FREE(comm->Node_Leaders);
	PCS_commDelete(&leaders);
//    objFree(commtable->Comms, &leaders);
//	FREE(comm->Node_Leaders);
  }
  
  if (locals != (Mpi_P_Comm *)NULL) {
    locals->Type = NOT_ALLOCATED;
	PCS_commDelete(&locals);
    //objFree(commtable->Comms, &locals);
  }
  
  comm->Comm_Leaders = (Mpi_P_Comm *)NULL;
  comm->Comm_Locals  = (Mpi_P_Comm *)NULL;
  comm->MCC_Support  = FALSE;
  
  return COM_E_OK;
}  


