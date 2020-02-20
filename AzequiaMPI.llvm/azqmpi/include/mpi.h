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

#ifndef	MPI_H
#define MPI_H

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------/
 /   Declaration of types and functions used by this module        /
 /----------------------------------------------------------------*/
#include <config.h>

#include <com.h>
#include <addr.h>

#include <env.h>
#include <p_dtype.h>
#include <p_collops.h>

/*-------------------------------------------------------/
 /                   Public constants                     /
 /-------------------------------------------------------*/
/* Any source */
#define MPI_ANY_SOURCE	        ADDR_RNK_ANY

#define MPI_ANY_TAG             TAG_ANY

#define MPI_UNDEFINED           AZQ_UNDEFINED

#define MPI_PROC_NULL           (PROC_NULL)

/* Max length of the processor name */
#define MPI_MAX_PROCESSOR_NAME	 16

/* Communicators */
typedef struct Mpi_Comm *MPI_Comm;
#define MPI_COMM_WORLD          ((MPI_Comm)COMM_WORLD_PTR)
#define MPI_COMM_SELF           ((MPI_Comm)COMM_SELF_PTR)
#define MPI_COMM_NULL           ((MPI_Comm)NULL)

/* Errors */
#define MPI_MAX_ERROR_STRING     32

#define	MPI_SUCCESS               0

#define MPI_ERR_BUFFER            2
#define MPI_ERR_COUNT             3
#define MPI_ERR_TYPE              4
#define MPI_ERR_TAG               5
#define MPI_ERR_COMM              6
#define MPI_ERR_RANK              7
#define MPI_ERR_REQUEST           8
#define MPI_ERR_ROOT              9
#define MPI_ERR_GROUP            10
#define MPI_ERR_OP               11
#define MPI_ERR_TOPOLOGY         12
#define MPI_ERR_DIMS             13
#define MPI_ERR_ARG              14
#define MPI_ERR_UNKNOWN          15
#define MPI_ERR_TRUNCATE         16
#define MPI_ERR_OTHER            17
#define MPI_ERR_INTERN           18
#define MPI_ERR_IN_STATUS        19
#define MPI_ERR_PENDING          20
#define MPI_ERR_LASTCODE         21

/* Error handlers */
#define MPI_ERRORS_ARE_FATAL    ((MPI_Errhandler) FATAL_ERROR_PTR)
#define MPI_ERRORS_RETURN       ((MPI_Errhandler) RETURN_ERROR_PTR)
#define MPI_ERRHANDLER_NULL     ((MPI_Errhandler) NULL)

typedef void (MPI_Handler_function)(MPI_Comm *comm, int *errcode, ...);
typedef struct Mpi_Errhandler *MPI_Errhandler;

/* Groups */
#define	MPI_GROUP_NULL	        ((MPI_Group) NULL)
#define MPI_GROUP_EMPTY         ((MPI_Group) GROUP_EMPTY_PTR)

typedef Mpi_P_Group *MPI_Group;

/* Comparation for groups and communicators */
#define MPI_IDENT                1
#define MPI_SIMILAR              2
#define MPI_UNEQUAL              3
#define MPI_CONGRUENT            4

/* Basic C datatypes */
#define	MPI_CHAR                 ((MPI_Datatype)PCS_dtypeGet(MPI_P_CHAR))
#define	MPI_SHORT                ((MPI_Datatype)PCS_dtypeGet(MPI_P_SHORT))
#define   MPI_INT                  ((MPI_Datatype)PCS_dtypeGet(MPI_P_INT))
#define	MPI_LONG                 ((MPI_Datatype)PCS_dtypeGet(MPI_P_LONG))
#define	MPI_LONG_LONG_INT        ((MPI_Datatype)PCS_dtypeGet(MPI_P_LONG_LONG_INT))
#define	MPI_LONG_LONG            ((MPI_Datatype)PCS_dtypeGet(MPI_P_LONG_LONG))
#define	MPI_UNSIGNED_CHAR        ((MPI_Datatype)PCS_dtypeGet(MPI_P_UNSIGNED_CHAR))
#define	MPI_UNSIGNED_SHORT       ((MPI_Datatype)PCS_dtypeGet(MPI_P_UNSIGNED_SHORT))
#define	MPI_UNSIGNED             ((MPI_Datatype)PCS_dtypeGet(MPI_P_UNSIGNED))
#define	MPI_UNSIGNED_LONG        ((MPI_Datatype)PCS_dtypeGet(MPI_P_UNSIGNED_LONG))
#define MPI_UNSIGNED_LONG_LONG   ((MPI_Datatype)PCS_dtypeGet(MPI_P_UNSIGNED_LONG_LONG))
#define	MPI_FLOAT                ((MPI_Datatype)PCS_dtypeGet(MPI_P_FLOAT))
#define	MPI_DOUBLE               ((MPI_Datatype)PCS_dtypeGet(MPI_P_DOUBLE))
#define	MPI_LONG_DOUBLE          ((MPI_Datatype)PCS_dtypeGet(MPI_P_LONG_DOUBLE))

#define MPI_2INT                 ((MPI_Datatype)PCS_dtypeGet(MPI_P_2INT))
#define MPI_SHORT_INT            ((MPI_Datatype)PCS_dtypeGet(MPI_P_SHORT_INT))
#define MPI_LONG_INT             ((MPI_Datatype)PCS_dtypeGet(MPI_P_LONG_INT))
#define MPI_FLOAT_INT            ((MPI_Datatype)PCS_dtypeGet(MPI_P_FLOAT_INT))
#define MPI_DOUBLE_INT           ((MPI_Datatype)PCS_dtypeGet(MPI_P_DOUBLE_INT))
#define MPI_LONG_DOUBLE_INT      ((MPI_Datatype)PCS_dtypeGet(MPI_P_LONG_DOUBLE_INT))

/* Fortran datatypes */
#define MPI_COMPLEX              ((MPI_Datatype)PCS_dtypeGet(MPI_P_COMPLEX))
#define MPI_DOUBLE_COMPLEX       ((MPI_Datatype)PCS_dtypeGet(MPI_P_DOUBLE_COMPLEX))

#define	MPI_BYTE                 ((MPI_Datatype)PCS_dtypeGet(MPI_P_BYTE))
#define	MPI_PACKED               ((MPI_Datatype)PCS_dtypeGet(MPI_P_PACKED))
#define MPI_LB                   MPI_P_LB
#define MPI_UB                   MPI_P_UB

/* Basic Fortran datatypes */
#define MPI_INTEGER              ((MPI_Datatype)PCS_dtypeGet(MPI_P_INT))
#define MPI_REAL                 ((MPI_Datatype)PCS_dtypeGet(MPI_P_FLOAT))
#define MPI_DOUBLE_PRECISION     ((MPI_Datatype)PCS_dtypeGet(MPI_P_DOUBLE))
#define MPI_LOGICAL              ((MPI_Datatype)PCS_dtypeGet(MPI_P_INT))

#define MPI_BOTTOM               (void *)0
#define MPI_DATATYPE_NULL        (Mpi_P_Datatype *)NULL

typedef Mpi_P_Datatype *MPI_Datatype;
typedef Mpi_P_Aint MPI_Aint;

/* Requests */
typedef Mpi_P_Request *MPI_Request;
#define MPI_REQUEST_NULL   ((MPI_Request) AZQ_RQST_NULL)

/* User buffer overload due to info struct */
#define MPI_BSEND_OVERHEAD  INFO_SEGMENT_SIZE

/* Status. The first fields are like Azequia status, so waitxxx() in
 Azequia can be called with this structure. In MPI_Waitall() and
 similar functions, it is not neccessary create temporal arrays of
 statuses.
 */
struct MPI_Status {
	int MPI_ERROR;
	int MPI_SOURCE;
	int MPI_TAG;
	int Count;
	int Cancelled;
};
typedef struct MPI_Status MPI_Status;

/* Collective operations */
#define MPI_OP_NULL	            ((MPI_Op) NULL)

typedef void (MPI_User_function)(void *invec, void *inoutvec, int *len,
		MPI_Datatype *datatype);
typedef struct Mpi_Op *MPI_Op;

#define MPI_SUM	                ((MPI_Op) PCS_copGet(MPI_SUM_INDEX))
#define MPI_MAX                 ((MPI_Op) PCS_copGet(MPI_MAX_INDEX))
#define MPI_MIN                 ((MPI_Op) PCS_copGet(MPI_MIN_INDEX))
#define MPI_PROD                ((MPI_Op) PCS_copGet(MPI_PROD_INDEX))
#define MPI_LAND                ((MPI_Op) PCS_copGet(MPI_LAND_INDEX))
#define MPI_BAND                ((MPI_Op) PCS_copGet(MPI_BAND_INDEX))
#define MPI_LOR                 ((MPI_Op) PCS_copGet(MPI_LOR_INDEX))
#define MPI_BOR                 ((MPI_Op) PCS_copGet(MPI_BOR_INDEX))
#define MPI_LXOR                ((MPI_Op) PCS_copGet(MPI_LXOR_INDEX))
#define MPI_BXOR                ((MPI_Op) PCS_copGet(MPI_BXOR_INDEX))
#define MPI_MAXLOC              ((MPI_Op) PCS_copGet(MPI_MAXLOC_INDEX))
#define MPI_MINLOC              ((MPI_Op) PCS_copGet(MPI_MINLOC_INDEX))

/* Keyval copy and delete functions for cached attributes */
typedef int MPI_Copy_function(MPI_Comm oldcomm, int keyval, void *extra_state,
		void *attribute_val_in, void *attribute_val_out, int *flag);
typedef int MPI_Delete_function(MPI_Comm comm, int keyval, void *attribute_val,
		void *extra_state);

extern int MPI_NULL_COPY_FN(MPI_Comm oldcomm, int keyval, void *extra_state,
		void *attribute_val_in, void *attribute_val_out, int *flag);
extern int MPI_DUP_FN(MPI_Comm oldcomm, int keyval, void *extra_state,
		void *attribute_val_in, void *attribute_val_out, int *flag);
extern int MPI_NULL_DELETE_FN(MPI_Comm comm, int keyval, void *attribute_val,
		void *extra_state);

#define  MPI_KEYVAL_INVALID      (-1)

/* Default attributes for MPI_COMM_WORLD, and the implementation specific attribute for maximum
 number of nodes supported */
#define MPI_TAG_UB                 0
#define MPI_HOST                   1
#define MPI_IO                     2
#define MPI_WTIME_IS_GLOBAL        3
#define MPI_MAX_NODES              4

/* Topologies */
#define  MPI_CART   (TOPO_CART)
#define  MPI_GRAPH  (TOPO_GRAPH)

/* + MPI 1.3 and 2.1 adds *****************************************************************/

/* Version */
#define  MPI_VERSION             1
#define  MPI_SUBVERSION          3

#define  MPI_STATUS_IGNORE       NULL
#define  MPI_STATUSES_IGNORE    (MPI_Status *)NULL

/* Thread-level support constants */
#define  MPI_THREAD_SINGLE       0
#define  MPI_THREAD_FUNNELED     1
#define  MPI_THREAD_SERIALIZED   2
#define  MPI_THREAD_MULTIPLE     3

/* - MPI 1.3 and 2.1 adds *****************************************************************/

/*-------------------------------------------------------/
 /           Declaration of public functions              /
 /-------------------------------------------------------*/

/*-------------------------------------------------------/
 /           Declaration of exported functions            /
 /-------------------------------------------------------*/

/* Profiling interface */
#ifdef AZQMPI_PROFILING
#define FUNCTION(name)  P##name
#else
#define FUNCTION(name)  name
#endif

/* + MPI 1.3 and 2.1 adds *****************************************************************/

extern int FUNCTION(MPI_Init_thread)(int *argc, char **argv[], int required,
		int *supported);
extern int FUNCTION(MPI_Query_thread)(int *provided);
extern int FUNCTION(MPI_Is_thread_main)(int *flag);

/* - MPI 1.3 and 2.1 adds *****************************************************************/

/* Init/finalize functions */
extern int FUNCTION(MPI_Init)(int *argc, char ***argv);
extern int FUNCTION(MPI_Finalize)(void);
extern int FUNCTION(MPI_Initialized)(int *flag);

/* Attribute functions */
extern int FUNCTION(MPI_Keyval_create)(MPI_Copy_function *copy_fn,
		MPI_Delete_function *delete_fn, int *keyval, void *extra_state);
extern int FUNCTION(MPI_Keyval_free)(int *keyval);
extern int FUNCTION(MPI_Attr_put)(MPI_Comm comm, int keyval,
		void *attribute_val);
extern int FUNCTION(MPI_Attr_get)(MPI_Comm comm, int keyval,
		void *attribute_val, int *flag);
extern int FUNCTION(MPI_Attr_delete)(MPI_Comm comm, int keyval);

/* Collective functions */
extern int FUNCTION(MPI_Op_create)(MPI_User_function *function, int commute,
		MPI_Op *op);
extern int FUNCTION(MPI_Op_free)(MPI_Op *op);
extern int FUNCTION(MPI_Reduce)(void *sendbuf, void *recvbuf, int count,
		MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm);
extern int FUNCTION(MPI_Allreduce)(void *sendbuf, void *recvbuf, int count,
		MPI_Datatype datatype, MPI_Op op, MPI_Comm comm);
extern int FUNCTION(MPI_Reduce_scatter)(void *sendbuf, void *recvbuf,
		int *recvcounts, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm);
extern int FUNCTION(MPI_Scan)(void *sendbuf, void *recvbuf, int count,
		MPI_Datatype datatype, MPI_Op op, MPI_Comm comm);
extern int FUNCTION(MPI_Bcast)(void *buffer, int count, MPI_Datatype datatype,
		int root, MPI_Comm comm);
extern int FUNCTION(MPI_Barrier)(MPI_Comm comm);
extern int FUNCTION(MPI_Gather)(void *sendbuf, int sendcnt,
		MPI_Datatype sendtype, void *recvbuf, int recvcount,
		MPI_Datatype recvtype, int root, MPI_Comm comm);
extern int FUNCTION(MPI_Allgather)(void *sendbuf, int sendcnt,
		MPI_Datatype sendtype, void *recvbuf, int recvcount,
		MPI_Datatype recvtype, MPI_Comm comm);
extern int FUNCTION(MPI_Gatherv)(void *sendbuf, int sendcount,
		MPI_Datatype sendtype, void *recvbuf, int *recvcounts, int *displs,
		MPI_Datatype recvtype, int root, MPI_Comm comm);
extern int FUNCTION(MPI_Allgatherv)(void *sendbuf, int sendcount,
		MPI_Datatype sendtype, void *recvbuf, int *recvcounts, int *displs,
		MPI_Datatype recvtype, MPI_Comm comm);
extern int FUNCTION(MPI_Scatter)(void *sendbuf, int sendcount,
		MPI_Datatype sendtype, void *recvbuf, int recvcount,
		MPI_Datatype recvtype, int root, MPI_Comm comm);
extern int FUNCTION(MPI_Scatterv)(void *sendbuf, int *sendcounts, int *displs,
		MPI_Datatype sendtype, void *recvbuf, int recvcount,
		MPI_Datatype recvtype, int root, MPI_Comm comm);
extern int FUNCTION(MPI_Alltoall)(void *sendbuf, int sendcount,
		MPI_Datatype sendtype, void *recvbuf, int recvcount,
		MPI_Datatype recvtype, MPI_Comm comm);
extern int FUNCTION(MPI_Alltoallv)(void *sendbuf, int *sendcounts, int *sdispls,
		MPI_Datatype sendtype, void *recvbuf, int *recvcounts, int *rdispls,
		MPI_Datatype recvtype, MPI_Comm comm);

/* Communicator functions */
extern int FUNCTION(MPI_Comm_size)(MPI_Comm comm, int *size);
extern int FUNCTION(MPI_Comm_rank)(MPI_Comm comm, int *rank);
extern int FUNCTION(MPI_Comm_group)(MPI_Comm comm, MPI_Group *group);
extern int FUNCTION(MPI_Comm_create)(MPI_Comm comm, MPI_Group group,
		MPI_Comm *newcomm);
extern int FUNCTION(MPI_Comm_free)(MPI_Comm *comm);
extern int FUNCTION(MPI_Comm_dup)(MPI_Comm comm, MPI_Comm *newcomm);
extern int FUNCTION(MPI_Comm_compare)(MPI_Comm comm1, MPI_Comm comm2,
		int *result);
extern int FUNCTION(MPI_Comm_split)(MPI_Comm comm, int colour, int key,
		MPI_Comm *newcomm);
extern int FUNCTION(MPI_Comm_test_inter)(MPI_Comm comm, int *flag);
extern int FUNCTION(MPI_Intercomm_create)(MPI_Comm local_comm, int local_leader,
		MPI_Comm peer_comm, int remote_leader, int tag, MPI_Comm *newintercomm);
extern int FUNCTION(MPI_Intercomm_merge)(MPI_Comm intercomm, int high,
		MPI_Comm *newintracomm);
extern int FUNCTION(MPI_Comm_remote_size)(MPI_Comm comm, int *size);
extern int FUNCTION(MPI_Comm_remote_group)(MPI_Comm comm, MPI_Group *group);

/* Datatype functions */
extern int FUNCTION(MPI_Get_count)(MPI_Status *status, MPI_Datatype datatype,
		int *count);
extern int FUNCTION(MPI_Get_elements)(MPI_Status *status, MPI_Datatype datatype,
		int *count);

extern int FUNCTION(MPI_Type_size)(MPI_Datatype datatype, int *size);

extern int FUNCTION(MPI_Type_contiguous)(int count, MPI_Datatype oldtype,
		MPI_Datatype *newtype);
extern int FUNCTION(MPI_Type_vector)(int count, int blocklength, int stride,
		MPI_Datatype oldtype, MPI_Datatype *newtype);
extern int FUNCTION(MPI_Type_hvector)(int count, int blocklength, int stride,
		MPI_Datatype oldtype, MPI_Datatype *newtype);
extern int FUNCTION(MPI_Type_indexed)(int count, int *array_of_blocklengths,
		MPI_Aint *array_of_displacements, MPI_Datatype oldtype,
		MPI_Datatype *newtype);
extern int FUNCTION(MPI_Type_hindexed)(int count, int *array_of_blocklengths,
		MPI_Aint *array_of_displacements, MPI_Datatype oldtype,
		MPI_Datatype *newtype);
extern int FUNCTION(MPI_Type_struct)(int count, int *array_of_blocklengths,
		MPI_Aint *array_of_displacements, MPI_Datatype *array_of_types,
		MPI_Datatype *newtype);
extern int FUNCTION(MPI_Type_commit)(MPI_Datatype *datatype);
extern int FUNCTION(MPI_Type_free)(MPI_Datatype *datatype);
extern int FUNCTION(MPI_Address)(void *location, MPI_Aint *address);
extern int FUNCTION(MPI_Type_extent)(MPI_Datatype datatype, MPI_Aint *extent);
extern int FUNCTION(MPI_Type_lb)(MPI_Datatype datatype, MPI_Aint *displacement);
extern int FUNCTION(MPI_Type_ub)(MPI_Datatype datatype, MPI_Aint *displacement);
extern int FUNCTION(MPI_Pack)(void *inbuf, int incount, MPI_Datatype datatype,
		void *outbuf, int outsize, int *position, MPI_Comm comm);
extern int FUNCTION(MPI_Unpack)(void *inbuf, int insize, int *position,
		void *outbuf, int outcount, MPI_Datatype datatype, MPI_Comm comm);
extern int FUNCTION(MPI_Pack_size)(int incount, MPI_Datatype datatype,
		MPI_Comm comm, int *size);

/* Error handling functions */
extern int FUNCTION(MPI_Error_class)(int errorcode, int *errorclass);
extern int FUNCTION(MPI_Error_string)(int errorcode, char *string,
		int *resultlen);
extern int FUNCTION(MPI_Errhandler_create)(MPI_Handler_function *function,
		MPI_Errhandler *errhandler);
extern int FUNCTION(MPI_Errhandler_set)(MPI_Comm comm,
		MPI_Errhandler errhandler);
extern int FUNCTION(MPI_Errhandler_get)(MPI_Comm, MPI_Errhandler *errhandler);
extern int FUNCTION(MPI_Errhandler_free)(MPI_Errhandler *errhandler);

/* Group functions */
extern int FUNCTION(MPI_Group_size)(MPI_Group group, int *size);
extern int FUNCTION(MPI_Group_rank)(MPI_Group group, int *rank);
extern int FUNCTION(MPI_Group_translate_ranks)(MPI_Group group1, int n,
		int *ranks1, MPI_Group group2, int *ranks2);
extern int FUNCTION(MPI_Group_compare)(MPI_Group group1, MPI_Group group2,
		int *result);
extern int FUNCTION(MPI_Group_union)(MPI_Group group1, MPI_Group group2,
		MPI_Group *newgroup);
extern int FUNCTION(MPI_Group_intersection)(MPI_Group group1, MPI_Group group2,
		MPI_Group *newgroup);
extern int FUNCTION(MPI_Group_difference)(MPI_Group group1, MPI_Group group2,
		MPI_Group *newgroup);
extern int FUNCTION(MPI_Group_incl)(MPI_Group group, int n, int *ranks,
		MPI_Group *newgroup);
extern int FUNCTION(MPI_Group_excl)(MPI_Group group, int n, int *ranks,
		MPI_Group *newgroup);
extern int FUNCTION(MPI_Group_range_incl)(MPI_Group group, int n,
		int ranges[][3], MPI_Group *newgroup);
extern int FUNCTION(MPI_Group_range_excl)(MPI_Group group, int n,
		int ranges[][3], MPI_Group *newgroup);
extern int FUNCTION(MPI_Group_free)(MPI_Group *group);

/* Point to point functions */
extern int FUNCTION(MPI_Send)(void *buf, int count, MPI_Datatype datatype,
		int dest, int tag, MPI_Comm comm);
extern int FUNCTION(MPI_Recv)(void *buf, int count, MPI_Datatype datatype,
		int source, int tag, MPI_Comm comm, MPI_Status *status);
extern int FUNCTION(MPI_Buffer_attach)(void *buffer, int size);
extern int FUNCTION(MPI_Buffer_detach)(void *buffer_addr, int *size);
extern int FUNCTION(MPI_Bsend)(void *buf, int count, MPI_Datatype datatype,
		int dest, int tag, MPI_Comm comm);
extern int FUNCTION(MPI_Rsend)(void *buf, int count, MPI_Datatype datatype,
		int dest, int tag, MPI_Comm comm);
extern int FUNCTION(MPI_Irsend)(void *buf, int count, MPI_Datatype datatype,
		int dest, int tag, MPI_Comm comm, MPI_Request *request);
extern int FUNCTION(MPI_Ibsend)(void *buf, int count, MPI_Datatype datatype,
		int dest, int tag, MPI_Comm comm, MPI_Request *request);
extern int FUNCTION(MPI_Ssend)(void *buf, int count, MPI_Datatype datatype,
		int dest, int tag, MPI_Comm comm);
extern int FUNCTION(MPI_Issend)(void *buf, int count, MPI_Datatype datatype,
		int dest, int tag, MPI_Comm comm, MPI_Request *request);
extern int FUNCTION(MPI_Isend)(void *buf, int count, MPI_Datatype datatype,
		int dest, int tag, MPI_Comm comm, MPI_Request *request);
extern int FUNCTION(MPI_Irecv)(void *buf, int count, MPI_Datatype datatype,
		int source, int tag, MPI_Comm comm, MPI_Request *request);
extern int FUNCTION(MPI_Wait)(MPI_Request *request, MPI_Status *status);
extern int FUNCTION(MPI_Waitany)(int count, MPI_Request *array_of_request,
		int *index, MPI_Status *status);
extern int FUNCTION(MPI_Waitall)(int count, MPI_Request *array_of_request,
		MPI_Status *array_of_statuses);
extern int FUNCTION(MPI_Waitsome)(int incount, MPI_Request *array_of_request,
		int *outcount, int *array_of_indices, MPI_Status *array_of_statuses);
extern int FUNCTION(MPI_Test)(MPI_Request *request, int *flag,
		MPI_Status *status);
extern int FUNCTION(MPI_Testany)(int count, MPI_Request *array_of_request,
		int *index, int *flag, MPI_Status *status);
extern int FUNCTION(MPI_Testall)(int count, MPI_Request *array_of_request,
		int *flag, MPI_Status *array_of_status);
extern int FUNCTION(MPI_Testsome)(int incount, MPI_Request *array_of_request,
		int *outcount, int *array_of_indices, MPI_Status *array_of_status);
extern int FUNCTION(MPI_Request_free)(MPI_Request *request);
extern int FUNCTION(MPI_Sendrecv)(void *sendbuf, int sendcount,
		MPI_Datatype sendtype, int dest, int sendtag, void *recvbuf,
		int recvcount, MPI_Datatype recvtype, int source, int recvtag,
		MPI_Comm comm, MPI_Status *status);
extern int FUNCTION(MPI_Sendrecv_replace)(void *buf, int count,
		MPI_Datatype datatype, int dest, int sendtag, int source, int recvtag,
		MPI_Comm comm, MPI_Status *status);
extern int FUNCTION(MPI_Iprobe)(int src, int tag, MPI_Comm comm, int *flag,
		MPI_Status *status);
extern int FUNCTION(MPI_Probe)(int src, int tag, MPI_Comm comm,
		MPI_Status *status);
extern int FUNCTION(MPI_Send_init)(void *buf, int count, MPI_Datatype datatype,
		int dest, int tag, MPI_Comm comm, MPI_Request *request);
extern int FUNCTION(MPI_Bsend_init)(void *buf, int count, MPI_Datatype datatype,
		int dest, int tag, MPI_Comm comm, MPI_Request *request);
extern int FUNCTION(MPI_Ssend_init)(void *buf, int count, MPI_Datatype datatype,
		int dest, int tag, MPI_Comm comm, MPI_Request *request);
extern int FUNCTION(MPI_Rsend_init)(void *buf, int count, MPI_Datatype datatype,
		int dest, int tag, MPI_Comm comm, MPI_Request *request);
extern int FUNCTION(MPI_Recv_init)(void *buf, int count, MPI_Datatype datatype,
		int source, int tag, MPI_Comm comm, MPI_Request *request);
extern int FUNCTION(MPI_Start)(MPI_Request *request);
extern int FUNCTION(MPI_Startall)(int count, MPI_Request *array_of_requests);
extern int FUNCTION(MPI_Cancel)(MPI_Request *request);
extern int FUNCTION(MPI_Test_cancelled)(MPI_Status *status, int *flag);

/* Topologies functions */
extern int FUNCTION(MPI_Cart_create)(MPI_Comm comm_old, int ndims, int *dims,
		int *periods, int reorder, MPI_Comm *comm_cart);
extern int FUNCTION(MPI_Cart_coords)(MPI_Comm comm, int rank, int maxdims,
		int *coords);
extern int FUNCTION(MPI_Cart_rank)(MPI_Comm comm, int *coords, int *rank);
extern int FUNCTION(MPI_Topo_test)(MPI_Comm comm, int *status);
extern int FUNCTION(MPI_Cartdim_get)(MPI_Comm comm, int *ndims);
extern int FUNCTION(MPI_Cart_get)(MPI_Comm comm, int maxdims, int *dims,
		int *periods, int *coords);
extern int FUNCTION(MPI_Cart_shift)(MPI_Comm comm, int direction, int disp,
		int *rank_source, int *rank_dest);
extern int FUNCTION(MPI_Cart_map)(MPI_Comm comm, int ndims, int *dims,
		int *periods, int *newrank);
extern int FUNCTION(MPI_Cart_sub)(MPI_Comm comm, int *remain_dims,
		MPI_Comm *newcomm);

extern int FUNCTION(MPI_Graph_create)(MPI_Comm comm_old, int nnodes, int *index,
		int *edges, int reorder, MPI_Comm *comm_graph);
extern int FUNCTION(MPI_Graphdims_get)(MPI_Comm comm, int *nnodes, int *nedges);
extern int FUNCTION(MPI_Graph_get)(MPI_Comm comm, int maxindex, int maxedges,
		int *index, int *edges);
extern int FUNCTION(MPI_Graph_neighbors_count)(MPI_Comm comm, int rank,
		int *nneighbors);
extern int FUNCTION(MPI_Graph_neighbors)(MPI_Comm comm, int rank,
		int maxneighbors, int *neighbors);
extern int FUNCTION(MPI_Graph_map)(MPI_Comm comm, int nnodes, int *index,
		int *edges, int *newrank);

/* Time functions */
extern double FUNCTION(MPI_Wtime)(void);
extern double FUNCTION(MPI_Wtick)(void);

/* Other functions */
extern int FUNCTION(MPI_Get_processor_name)(char *name, int *resultlen);
extern int FUNCTION(MPI_Abort)(MPI_Comm comm, int errorcode);
extern int FUNCTION(MPI_Pcontrol)(const int level, ...);
extern int FUNCTION(MPI_Get_version)(int *version, int *subversion);


extern int FUNCTION(MPI_Dims_create) (int nnodes, int ndims, int *dims);
#ifdef __cplusplus
}
#endif

/* Main function for operators. Put underscores at finish for FORTRAN support:
 FORTRAN main program must be a "subroutine node_main" by now */
#define main usermain

//#define CHECK_MODE
//To help hooking...for mpise. It's argly but works..

#define MPI_Init_thread 			 PMPI_Init_thread
#define MPI_Query_thread 		 PMPI_Query_thread
#define MPI_Is_thread_main		 PMPI_Is_thread_main

//- MPI 1.3 and 2.1 adds *****************************************************************

// Init/finalize functions
#define MPI_Init 					PMPI_Init
#define MPI_Finalize 			PMPI_Finalize
#define MPI_Initialized			PMPI_Initialized

//Attribute functions
#define MPI_Keyval_create 		PMPI_Keyval_create
#define MPI_Keyval_free 			PMPI_Keyval_free
#define MPI_Attr_put 			PMPI_Attr_put
#define MPI_Attr_get 			PMPI_Attr_get
#define MPI_Attr_delete 			PMPI_Attr_delete

//Collective functions
#define MPI_Op_create 			PMPI_Op_create
#define MPI_Op_free 				PMPI_Op_free
#define MPI_Reduce   			PMPI_Reduce
#define MPI_Allreduce			PMPI_Allreduce
#define MPI_Reduce_scatter		PMPI_Reduce_scatter
#define MPI_Scan					PMPI_Scan
#define MPI_Bcast					PMPI_Bcast
#define MPI_Barrier				PMPI_Barrier
#define MPI_Gather				PMPI_Gather
#define MPI_Allgather			PMPI_Allgather
#define MPI_Gatherv				PMPI_Gatherv
#define MPI_Allgatherv			PMPI_Allgatherv
#define MPI_Scatter				PMPI_Scatter
#define MPI_Scatterv			PMPI_Scatterv
#define MPI_Alltoall			PMPI_Alltoall
#define MPI_Alltoallv			PMPI_Alltoallv

// Communicator functions
#define MPI_Comm_size			PMPI_Comm_size
#define MPI_Comm_rank			PMPI_Comm_rank
#define MPI_Comm_group			PMPI_Comm_group
#define MPI_Comm_create			PMPI_Comm_create
#define MPI_Comm_free			PMPI_Comm_free
#define MPI_Comm_dup				PMPI_Comm_dup
#define MPI_Comm_compare			PMPI_Comm_compare
#define MPI_Comm_split			PMPI_Comm_split
#define MPI_Comm_test_inter		PMPI_Comm_test_inter
#define MPI_Intercomm_create	PMPI_Intercomm_create
#define MPI_Intercomm_merge		PMPI_Intercomm_merge
#define MPI_Comm_remote_size	PMPI_Comm_remote_size
#define MPI_Comm_remote_group	PMPI_Comm_remote_group

// Datatype functions
#define MPI_Get_count			PMPI_Get_count
#define MPI_Get_elements			PMPI_Get_elements

#define MPI_Type_size			PMPI_Type_size

#define MPI_Type_contiguous		PMPI_Type_contiguous
#define MPI_Type_vector			PMPI_Type_vector
#define MPI_Type_hvector			PMPI_Type_hvector
#define MPI_Type_indexed			PMPI_Type_indexed
#define MPI_Type_hindexed		PMPI_Type_hindexed
#define MPI_Type_struct			PMPI_Type_struct
#define MPI_Type_commit			PMPI_Type_commit
#define MPI_Type_free			PMPI_Type_free
#define MPI_Address				PMPI_Address
#define MPI_Type_extent			PMPI_Type_extent
#define MPI_Type_lb				PMPI_Type_lb
#define MPI_Type_ub				PMPI_Type_ub
#define MPI_Pack					PMPI_Pack
#define MPI_Unpack				PMPI_Unpack
#define MPI_Pack_size			PMPI_Pack_size

// Error handling functions
#define MPI_Error_class			PMPI_Error_class
#define MPI_Error_string			PMPI_Error_string
#define MPI_Errhandler_create	PMPI_Errhandler_create
#define MPI_Errhandler_set		PMPI_Errhandler_set
#define MPI_Errhandler_get		PMPI_Errhandler_get
#define MPI_Errhandler_free		PMPI_Errhandler_free

// Group functions
#define MPI_Group_size				PMPI_Group_size
#define MPI_Group_rank				PMPI_Group_rank
#define MPI_Group_translate_ranks	PMPI_Group_translate_ranks
#define MPI_Group_compare			PMPI_Group_compare
#define MPI_Group_union				PMPI_Group_union
#define MPI_Group_intersection		PMPI_Group_intersection
#define MPI_Group_difference		PMPI_Group_difference
#define MPI_Group_incl				PMPI_Group_incl
#define MPI_Group_excl				PMPI_Group_excl
#define MPI_Group_range_incl		PMPI_Group_range_incl
#define MPI_Group_range_excl		PMPI_Group_range_excl
#define MPI_Group_free				PMPI_Group_free

// Point to point functions
#define MPI_Send						PMPI_Send
#define MPI_Recv					PMPI_Recv
#define MPI_Buffer_attach		PMPI_Buffer_attach
#define MPI_Buffer_detach		PMPI_Buffer_detach
#define MPI_Bsend					PMPI_Bsend
#define MPI_Rsend					PMPI_Rsend
#define MPI_Irsend				PMPI_Irsend
#define MPI_Ibsend				PMPI_Ibsend
#define MPI_Ssend					PMPI_Ssend
#define MPI_Issend				PMPI_Issend
#define MPI_Isend					PMPI_Isend
#define MPI_Irecv					PMPI_Irecv
#define MPI_Wait					PMPI_Wait
#define MPI_Waitany				PMPI_Waitany
#define MPI_Waitall				PMPI_Waitall
#define MPI_Waitsome				PMPI_Waitsome
#define MPI_Test					PMPI_Test
#define MPI_Testany				PMPI_Testany
#define MPI_Testall				PMPI_Testall
#define MPI_Testsome				PMPI_Testsome
#define MPI_Request_free			PMPI_Request_free
#define MPI_Sendrecv				PMPI_Sendrecv
#define MPI_Sendrecv_replace			PMPI_Sendrecv_replace
#define MPI_Iprobe				PMPI_Iprobe
#define MPI_Probe					PMPI_Probe
#define MPI_Send_init			PMPI_Send_init
#define MPI_Bsend_init			PMPI_Bsend_init
#define MPI_Ssend_init			PMPI_Ssend_init
#define MPI_Rsend_init			PMPI_Rsend_init
#define MPI_Recv_init			PMPI_Recv_init
#define MPI_Start					PMPI_Start
#define MPI_Startall				PMPI_Startall
#define MPI_Cancel				PMPI_Cancel
#define MPI_Test_cancelled			PMPI_Test_cancelled

// Topologies functions
#define MPI_Cart_create			PMPI_Cart_create
#define MPI_Cart_coords			PMPI_Cart_coords
#define MPI_Cart_rank			PMPI_Cart_rank
#define MPI_Topo_test			PMPI_Topo_test
#define MPI_Cartdim_get			PMPI_Cartdim_get
#define MPI_Cart_get				PMPI_Cart_get
#define MPI_Cart_shift			PMPI_Cart_shift
#define MPI_Cart_map				PMPI_Cart_map
#define MPI_Cart_sub				PMPI_Cart_sub

#define MPI_Graph_create			PMPI_Graph_create
#define MPI_Graphdims_get			PMPI_Graphdims_get
#define MPI_Graph_get			PMPI_Graph_get
#define MPI_Graph_neighbors_count			PMPI_Graph_neighbors_count
#define MPI_Graph_neighbors			PMPI_Graph_neighbors
#define MPI_Graph_map			PMPI_Graph_map

//Time functions
#define MPI_Wtime			PMPI_Wtime
#define MPI_Wtick			PMPI_Wtick

// Other functions
#define MPI_Get_processor_name			PMPI_Get_processor_name
#define MPI_Abort			PMPI_Abort
#define MPI_Pcontrol			PMPI_Pcontrol
#define MPI_Get_version			PMPI_Get_version

#define MPI_Dims_create PMPI_Dims_create
#endif
