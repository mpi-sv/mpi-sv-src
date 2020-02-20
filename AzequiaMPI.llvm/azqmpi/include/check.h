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

#ifndef CHECK_H
#define CHECK_H

#ifdef CHECK_MODE

#include <config.h>

#include <env.h>
#include <p_comn.h>
#include <p_group.h>
#include <p_collops.h>
#include <p_dtype.h>
#include <p_rqst.h>
#include <p_status.h>
#include <p_errhnd.h>


   /*-------------------------------------------------------/
  /         Declaration of exported constnants             /
 /-------------------------------------------------------*/

   /*-------------------------------------------------------/
  /           Declaration of exported functions            /
 /-------------------------------------------------------*/
extern int  check_tag            (int tag);
extern int  check_group          (Mpi_P_Group *group);
extern int  check_comm           (Mpi_P_Comm *comm);
extern int  check_source         (int source, Mpi_P_Group *group);
extern int  check_dest           (int dest, Mpi_P_Group *group);
extern int  check_rank_comm      (int rank, Mpi_P_Comm *comm);
extern int  check_dest_comm      (int dest, Mpi_P_Comm *comm);
extern int  check_root           (int root, Mpi_P_Comm *comm);
extern int  check_source_comm    (int dest, Mpi_P_Comm *comm);
extern int  check_count          (int count);
extern int  check_datatype       (Mpi_P_Datatype *datatype);
extern int  check_dtype_alloc    (Mpi_P_Datatype *datatype);
extern int  check_dtype_commit   (Mpi_P_Datatype *datatype);
extern int  check_ops            (Mpi_P_Op *op);
extern int  check_valid_ranks    (Mpi_P_Group *group, int size, int *ranks);
extern int  check_valid_ranges   (Mpi_P_Group *group, int n, int ranges[][3]);
extern int  check_request        (int count, Mpi_P_Request_t *request);
extern int  check_keyval         (int keyval);
extern int  check_ndims          (int ndims);
extern int  check_dims           (int ndims, int *dims);
extern int  check_dims_graph     (int nnodes, int nedges);
extern int  check_request_nr     (int count);
extern int  check_comm_type      (Mpi_P_Comm *comm, int type);
extern int  check_disjoint       (int size1, int *ranks1, int size2, int *ranks2);
extern int  check_status         (Mpi_P_Status *status);
extern int  check_errhandler     (Mpi_P_Errhandler *errhandler);
extern int  check_topo_type      (Mpi_P_Comm *comm, int type);


#endif

#endif
