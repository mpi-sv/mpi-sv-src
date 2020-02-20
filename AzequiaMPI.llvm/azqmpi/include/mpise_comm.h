#ifndef __MPISE_COMM_H__
#define __MPISE_COMM_H__
#include "mpi.h"


#define MAX_MPI_CALLS 4096 // Not sure it's enough

typedef enum {
	NOP,
	COMM_SSEND = 1,
	COMM_SEND,
	COMM_RECV,
	COMM_BARR,
	COMM_ISEND,
	COMM_IRECV,
	COMM_WAIT=7,    // do not change COMM_WAIT=7, coz azqmpi use  klee_mpi_nonblock(.., 7, to indicate wait.
	MPI_FINDATATRANS=8,// memcpy finished by MPI lib in ISend or IRecv, DO NOT CHANGE!!! FIX IT TO BE 8.
	COMM_TEST,
	COMM_FIN,
	COMM_INIT,
	COMM_WAITANY,
	COMM_LEAK
} COMM_TYPE;

typedef struct str_arg {
	COMM_TYPE  func;

	// serve as function args, just a copy of the MPI operation args.
	void *buffer;
	int count;
	int source;
	int dest;
	int tag;
	int root;
	MPI_Comm comm;
	MPI_Request * request;
	MPI_Status * status;
	MPI_Datatype type;
} arg_t;


#endif
