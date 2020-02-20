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
#include <env.h>
#include <errhnd.h>
#include <check.h>


/* Profiling */
#ifdef AZQMPI_PROFILING
#undef MPI_Comm_split
#define MPI_Comm_split  PMPI_Comm_split
#endif


/*
 *  MPI_Comm_split
 */
int MPI_Comm_split(MPI_Comm comm, int colour, int key, MPI_Comm *newcomm) {

  int           mpi_errno;
  int          *colours;
  int          *keys;
  int           i, j, index;
  int           tmpcolor, tmpkey;
  Mpi_P_Group  *group, *newgroup = MPI_GROUP_NULL;
  int           p_commNr;
  int           r_commNr;
  int           rank;
  int           size;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Comm_split (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_comm_type(comm, INTRACOMM))        goto mpi_exception;
#endif

  NEST_FXN_INCR();

  /* 1. Allocate colours and keys arrays */
  size = commGetSize(comm);
  CALL_FXN(MALLOC(colours, size * sizeof(int)), MPI_ERR_INTERN);
  CALL_FXN(MALLOC(keys,    size * sizeof(int)), MPI_ERR_INTERN);

  /* 2. Get the colour and keys of every process in comm */
  CALL_MPI_NEST(MPI_Allgather(&colour, 1, MPI_INT, colours, 1, MPI_INT, comm));
  CALL_MPI_NEST(MPI_Allgather(&key,    1, MPI_INT, keys,    1, MPI_INT, comm));

  /* 3. Find the number of processes in the group */
  group  = commGetLocalGroup(comm);

  /* 4. If we have a colour, prepare the group for the new communicator */
  if (colour != MPI_UNDEFINED) {
    /* 4.1. Fill the group with the members of my colour in rank order. It uses the same
            colours and keys arrays for save memory */
    index = 0;
    for (i = 0; i < size; i++) {
      if (colours[i] == colour) {
        colours[index] = groupGetGlobalRank (group, i);
        keys[index]    = keys[i];
        index++;
      }
    }

    /* 4.2. Sort the process in my colour's group based on keys. Bubble algorithm */
    for (i = 0; i < index; i++) {
      for (j = i + 1; j < index; j++) {
		if (keys[i] > keys[j]) {

		  tmpcolor   = colours[i];
		  tmpkey     = keys[i];

          colours[i] = colours[j];
		  keys[i]    = keys[j];

		  colours[j] = tmpcolor;
		  keys[j]    = tmpkey;
        }
	  }
    }

    /* 4.3. Now we've got in array those who should be part of the group */
    CALL_FXN(PCS_groupCreate(colours, index, &newgroup), MPI_ERR_INTERN);
  }

  /* 5. All processes in group take part in creating a communicator for only the group members. It
        create a communicator here and not call to MPI_Comm_create() function that check for
        some integrity not allowed here */
  *newcomm = MPI_COMM_NULL;

  /* 5.1. AllReduce operation for get a new communicator number. Tasks propose a number
          and the max is elected. All communicators tasks take part in the election */
  p_commNr = PCS_commGetNrMax() + 1;

  CALL_MPI_NEST(MPI_Allreduce (&p_commNr, &r_commNr, 1, MPI_INT, MPI_MAX, comm));

  /* 5.2. Caller task must be in the group in newcomm, else, it not create the communicator entry and
          return OK. This is valid too for MPI_UNDEFINED caller colour */
  if (colour == MPI_UNDEFINED)                   goto ret_ok;
  if (newgroup == MPI_GROUP_NULL)                goto ret_ok;
  if (0 > (rank = groupGetLocalRank(newgroup)))  goto ret_ok;

  /* 6. Only the task in new group create a new communicator entry */
  CALL_FXN (comm_create (comm, newgroup, NULL, r_commNr, INTRACOMM, NULL, newcomm), MPI_ERR_COMM);

  /* 7. Copy the DEFAULT attributes to the new communicator */
  CALL_FXN (PCS_keyCopyDfltAttr(comm, *newcomm), MPI_ERR_OTHER);
  
ret_ok:
  /* 8. Free allocated memory */
  FREE(colours);
  FREE(keys);

  NEST_FXN_DECR();

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Comm_split (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
  NEST_FXN_DECR();

mpi_exception:
  FREE(colours);
  FREE(keys);
  return commHandleError(comm, mpi_errno, "MPI_Comm_split");
}
