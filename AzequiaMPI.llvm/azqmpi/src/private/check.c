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
 /   Declaration of public functions implemented by this module    /
/----------------------------------------------------------------*/
#include <check.h>

  /*----------------------------------------------------------------/
 /   Declaration of types and functions used by this module        /
/----------------------------------------------------------------*/
#ifdef CHECK_MODE

/* For using MPI error codes */
#include <mpi.h>

#include <com.h>

#include <env.h>
#include <errhnd.h>
#include <p_key.h>
#include <p_rqst.h>
#include <p_status.h>
#include <p_comn.h>

  /*----------------------------------------------------------------/
 /   Implementation of public functions                            /
/----------------------------------------------------------------*/
/*
 *  check_tag():
 *   Check if the tag field in a message is correct
 */
int check_tag (int tag) {

  if (tag < 0)                                   return MPI_ERR_TAG;

  return MPI_SUCCESS;
}


/*
 *  check_group():
 *   Check if the group field in a message is correct
 */
int check_group (Mpi_P_Group *group) {

  if (group == MPI_GROUP_NULL)                   return MPI_ERR_GROUP;

  return MPI_SUCCESS;
}


/*
 *  check_comm():
 *   Check if the comm field in a message is correct
 */
int check_comm (Mpi_P_Comm *comm) {

  if (comm == (Mpi_P_Comm *)MPI_COMM_NULL)       return MPI_ERR_COMM;

  return MPI_SUCCESS;
}


/*
 *  check_source():
 *   Check if the source rank field in a message is correct
 */
int check_source (int source, Mpi_P_Group *group) {

  int size;


  if (source == MPI_ANY_SOURCE)                  return MPI_SUCCESS;

  size = groupGetSize(group);
  if ((source < 0) || (source >= size))          return MPI_ERR_RANK;

  return MPI_SUCCESS;
}


/*
 *  check_dest():
 *   Check if the dest rank field in a message is correct
 */
int check_dest (int dest, Mpi_P_Group *group) {

  int size;

  size = groupGetSize(group);
  if ((dest < 0) || (dest >= size))              return MPI_ERR_RANK;

  return MPI_SUCCESS;
}


/*
 *  check_rank_comm():
 *   Check if the rank is in the communicator group
 */
int check_rank_comm (int rank, Mpi_P_Comm *comm) {
  return check_dest_comm(rank, comm);
}


/*
 *  check_dest_comm():
 *   Check if the dest rank field in a message is in the communicator group
 */
int check_dest_comm (int dest, Mpi_P_Comm *comm) {

  Mpi_P_Group  *group;
  int           size;

  if (MPI_GROUP_NULL == (group = commGetGroup(comm)))
                                                 return MPI_ERR_GROUP;

  size = groupGetSize(group);
  if ((dest < 0) || (dest >= size))              return MPI_ERR_GROUP;

  return MPI_SUCCESS;
}


/*
 *  check_dest_comm():
 *   Check if the dest rank field in a message is in the communicator group
 */
int check_source_comm (int source, Mpi_P_Comm *comm) {

  Mpi_P_Group  *group;
  int           size;

  if (source == MPI_ANY_SOURCE)                  return MPI_SUCCESS;

  if (MPI_GROUP_NULL == (group = commGetLocalGroup(comm)))
                                                 return MPI_ERR_RANK;

  size = groupGetSize(group);
  if ((source < 0) || (source >= size))          return MPI_ERR_RANK;

  return MPI_SUCCESS;
}


/*
 *  check_root():
 *   Check if the root in a collective operation belongs to the communicator used
 */
int check_root (int root, Mpi_P_Comm *comm) {

  Mpi_P_Group  *group;
  int           size;

  if (MPI_GROUP_NULL == (group = commGetLocalGroup(comm)))
                                                 return MPI_ERR_GROUP;

  size = groupGetSize(group);
  if ((root < 0) || (root >= size))              return MPI_ERR_ROOT;

  return MPI_SUCCESS;
}


/*
 *  check_count():
 *   Check if the count field in a message is correct
 */
int check_count (int count) {

  if (count < 0)                                 return MPI_ERR_COUNT;

  return MPI_SUCCESS;
}


/*
 *  check_datatype():
 *   Check if the datatype field in a message is correct
 */
int check_datatype (Mpi_P_Datatype *datatype) {

  if (((int)datatype == MPI_UB) || ((int)datatype == MPI_LB))
	return MPI_SUCCESS;

  if (datatype == MPI_DATATYPE_NULL)             return MPI_ERR_TYPE;

  return MPI_SUCCESS;
}


/*
 *  check_dtype_alloc():
 *   Check if the datatype field in a message is allocated
 */
int check_dtype_alloc (Mpi_P_Datatype *datatype) {

  if (!dtypeIsAllocated(datatype))               return MPI_ERR_TYPE;

  return MPI_SUCCESS;
}


/*
 *  check_dtype_commit():
 *   Check if the datatype field in a message is commited
 */
int check_dtype_commit (Mpi_P_Datatype *datatype) {

  if (!dtypeIsCommited(datatype))                return MPI_ERR_TYPE;

  return MPI_SUCCESS;
}


/*
 *  check_ops():
 *   Check if the datatype field in a message is correct
 */
int check_ops (Mpi_P_Op *op) {

  if (op == MPI_OP_NULL)                         return MPI_ERR_OP;

  return MPI_SUCCESS;
}


/*
 *  check_valid_ranks():
 *   Check if ranks in group are all different from each other and if all
 *   ranks belong to the group
 */
int check_valid_ranks (Mpi_P_Group *group, int size, int *ranks) {

  int i, j;
  int grpsize;

  grpsize = groupGetSize(group);

  for (i = 0; i < size; i++) {
    if (ranks[i] >= grpsize)                     return MPI_ERR_RANK;
    for (j = i + 1; j < size; j++)
      if (ranks[i] == ranks[j])                  return MPI_ERR_RANK;
  }

  return MPI_SUCCESS;
}


/*
 *  check_valid_ranges():
 *   Check if ranks in group are all different from each other and if all
 *   ranks belong to the group
 */
int check_valid_ranges (Mpi_P_Group *group, int n, int ranges[][3]) {

  int size;
  int flags[MAX_MPI_PROCS];
  int act_last;
  int first, last, stride;
  int i, j;


  if (n < 0)                                     return MPI_ERR_ARG;

  size = groupGetSize(group);

  /* First, clear the flag */
  for (i = 0; i < size; i++) {
    flags[i] = 0;
  }

  for (i = 0; i < n; i++) {

    first  = ranges[i][0];
    last   = ranges[i][1];
    stride = ranges[i][2];

    if (first < 0 || first >= size)              return MPI_ERR_ARG;
    if (stride == 0)                             return MPI_ERR_ARG;

    act_last = first + stride * ((last - first) / stride);
    if (last < 0 || act_last >= size)            return MPI_ERR_ARG;
    if ((stride > 0 && first > last) ||
        (stride < 0 && first < last))            return MPI_ERR_ARG;

    if (stride > 0) {

      for (j = first; j <= last; j += stride) {
        if (flags[j])                            return MPI_ERR_ARG;
        else flags[j] = 1;
      }

    } else {

      for (j = first; j >= last; j += stride) {
        if (flags[j])                            return MPI_ERR_ARG;
        else flags[j] = i + 1;
      }

    }

  }

  return MPI_SUCCESS;
}


/*
 *  check_request():
 *   Check if an array of requests is valid
 */
int check_request (int count, Mpi_P_Request_t *request) {

  int i;

  for (i = 0; i < count; i++) {
    if (request[i] == MPI_REQUEST_NULL)          return MPI_ERR_REQUEST;
  }

  return MPI_SUCCESS;
}


/*
 *  check_keyval():
 *   Check if a keyval has a correct value
 */
int check_keyval(int keyval) {

  if (keyval == MPI_KEYVAL_INVALID)              return MPI_ERR_ARG;

  return MPI_SUCCESS;
}


/*
 *  check_ndims():
 *   Check if a dimension number is in range
 */
int check_ndims (int ndims) {

  if ((ndims < 0) || (ndims > MAX_CART_DIMS))    return MPI_ERR_DIMS;

  return MPI_SUCCESS;
}


/*
 *  check_dims():
 *   Check for negative dimensions
 */
int check_dims (int ndims, int *dims) {

  int i;

  for (i = 0; i < ndims; i++) {
    if (dims[i] < 0)                             return MPI_ERR_DIMS;
  }
  return MPI_SUCCESS;
}


/*
 *  check_dims_graph():
 *   Check index and edges in a graph topology. Maximum index is the number of
 *   processes in an application and maximum edges is number of processes in an
 *   application multiplied by the average degree of a node
 */
int check_dims_graph (int nnodes, int nedges) {

  if ((nnodes < 0) || (nnodes > MAX_MPI_PROCS))  return MPI_ERR_DIMS;
  if ((nedges < 0) || (nedges > (MAX_MPI_PROCS * DEGREE)))
                                                 return MPI_ERR_DIMS;
  return MPI_SUCCESS;
}


/*
 *  check_request_nr():
 *   Check the number of request
 */
int check_request_nr (int count) {

  if (count < 0)                                 return MPI_ERR_ARG;

  return MPI_SUCCESS;
}


/*
 *  check_comm_type():
 *   Check the type of a communicator
 */
int check_comm_type (Mpi_P_Comm *comm, int type) {

  if (commGetType(comm) != type)                 return MPI_ERR_COMM;

  return MPI_SUCCESS;
}


/*
 *  check_disjoint():
 *   Check if two array of global ranks members of groups are disjoint
 */
int check_disjoint (int size1, int *ranks1, int size2, int *ranks2) {

  int i, j;

  for (i = 0; i < size1; i++) {
    for (j = 0; j < size2; j++) {
      if (ranks1[i] == ranks2[j])                return MPI_ERR_GROUP;
    }
  }

  return MPI_SUCCESS;
}


/*
 *  check_status():
 *   Check if null status
 */
int check_status (Mpi_P_Status *status) {

  if (status == (Mpi_P_Status *)NULL)            return MPI_ERR_IN_STATUS;

  return MPI_SUCCESS;
}


/*
 *  check_errhandler():
 *   Check if null error handler
 */
int check_errhandler (Mpi_P_Errhandler *errhandler) {

  if (errhandler == (Mpi_P_Errhandler *)NULL)    return MPI_ERR_OTHER;

  return MPI_SUCCESS;
}


/*
 *  check_topo_type():
 *   Check type of a topology
 */
int check_topo_type (Mpi_P_Comm *comm, int type) {

  if (commTopoType (comm) != type)               return MPI_ERR_TOPOLOGY;

  return MPI_SUCCESS;
}



#endif

