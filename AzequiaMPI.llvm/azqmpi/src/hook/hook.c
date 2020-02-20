#include <assert.h>
#include <stdbool.h>
#include "hook.h"
#include "simple-hash.h"
/*#undef MPI_Irecv
 #undef MPI_Recv
 #undef MPI_Isend
 #undef MPI_Barrier
 #undef MPI_Wait
 #undef MPI_Test
 #undef MPI_Ssend
 #undef MPI_Send
 #undef MPI_Init
 #undef MPI_Finalize
 #undef MPI_COMM_size
 #undef MPI_Comm_rank*/

#define LIB_HOOK_DBG 1
#define DR(code) { if(LIB_HOOK_DBG) {code}}

#define LOG_COMM_AND_PROMOTE(src, dest, type) klee_mpi_comm(src, dest, type)
#define GET_SOURCE_CANDIDATE()                  klee_get_anysource_candi()
#define NOTIFY_NONBLOCKING(buf,size,type,req)		klee_mpi_nonblock(buf, size, type, req)

#define FIRE_MPI_Slave() klee_fire_slave()
#define FIRE_MPI_7OP(func,rank, para1, para2, para3,para4, para5,para6, para7)  klee_fire_mpi7para(func,rank,para1, para2, para3,para4, para5,para6, para7)
#define FIRE_MPI_1OP(func,rank, para1)       klee_fire_mpi1para(func, rank,para1)
#define FIRE_MPI_2OP(func, rank,para1,para2) klee_fire_mpi2para(func, rank,para1,para2)
#define FIRE_MPI_0OP(func,rank)              klee_fire_mpi0para(func,rank)
#define FIRE_MPI_4OP(func,rank, ancestor,para1, para2, para3,para4) klee_fire_mpi4para(func, rank,ancestor, para1, para2, para3,para4)
#define KEEP_BLOCK()							while(klee_keep_block())
//#define KEEP_BLOCK()							klee_keep_block()
#define ANY_SOURCE_DBG
extern pthread_key_t key;
#define self()        ((Thr_t)pthread_getspecific(key))

#define BLOCKING_SEND   0// Treat MPI_Send as MPI_Ssend
extern int * func_cnt_array;
extern arg_t ** arg_array;
extern hash_table *table;
int getCpuId(void);
bool klee_keep_block(void);

static void CopyBuffer(void* inbuffer, void** outbuffer, MPI_Datatype datatype,
		int count) {
	if (inbuffer == NULL ) {
		*outbuffer = NULL;
		return;
	}
	MPI_Aint extent;
	MPI_Type_extent(datatype, &extent);
	*outbuffer = malloc(count * extent);
	memcpy(*outbuffer, inbuffer, count * extent);
	return;
}

static void CopyDataType(MPI_Datatype origtype, MPI_Datatype * dunptype) {
	assert(dunptype!=NULL);
	MPI_Type_contiguous(1, origtype, dunptype);
	MPI_Type_commit(dunptype);
	return;
}

static int CopyRequest(MPI_Request *req, MPI_Request**outreq) {
	*outreq = malloc(sizeof(MPI_Request));
	//Seems that this memcpy is non-sense.
	memcpy(*outreq, req, sizeof(MPI_Request));
	//NOTE the fact that MPI_Request is still a pointer. And this
	// pointer would not be initialized till Isend or Irecv.
	**outreq = NULL;

	MPI_Request * persistreq;
	if (find_entry(table, req, 1, (void **) &persistreq) < 0) {
		if (insert_entry(table, (void *) req, 1, (void *) (*outreq)) == 0)
			return 0;
	}
	else {
		// we just overwrite the key
		//printf("copyrequest, insert failed.\n");
		printf("copyrequest, using a same req variable.\n");
		set_entry(table, (void *)req, 1, (void *)(*outreq));
	}

	return -1;
}

/*static int CopyStatus(MPI_Status *stat, MPI_Status**outstat) {
 *outstat = malloc(sizeof(MPI_Status));
 memcpy(*outstat, stat, sizeof(MPI_Status));
 #if 1
 printf("CopyStatus: 0x%x -> 0x%x\n", req, *outreq);
 #endif
 if (insert_entry(table, (void *) stat, 1, (void *) (*outstat))
 == 0)
 return 0;
 else
 printf("CopyStatus, insert failed.\n");

 return -1;
 }*/

//fixme::return res.
int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag,
		MPI_Comm comm, MPI_Status *status) {
	int rank = commGetRank(comm);

#ifdef CHECK_MODE
	check_comm(comm);
	check_datatype (datatype, comm);
#endif
	//LOG_COMM_AND_PROMOTE(commGetRank(comm), source, COMM_RECV);
	int funcnt = func_cnt_array[rank];
	assert(funcnt< MAX_MPI_CALLS && "MPI call number exceeds MAX_MPI_CALLS");
	arg_array[rank][funcnt].status = malloc(sizeof(MPI_Status));
	FIRE_MPI_7OP(COMM_RECV, rank, buf, count, datatype, source, tag, comm,
			arg_array[rank][funcnt].status);

	func_cnt_array[rank]++;
	KEEP_BLOCK()
		;
	//Not sure that this is enough. The life span is a big problem.
	if (status)
		*status = *arg_array[rank][funcnt].status;
	return MPI_SUCCESS;
}

int MPI_Ssend(void *buf, int count, MPI_Datatype datatype, int dest, int tag,
		MPI_Comm comm) {
	//if (buf == NULL )
	//return MPI_ERR_BUFFER;
	int rank = commGetRank(comm);
#ifdef CHECK_MODE
	check_comm(comm);
	check_datatype (datatype, comm);
#endif

	FIRE_MPI_7OP(COMM_SSEND, rank, buf, count, datatype, dest, tag, comm, NULL);
	//LOG_COMM_AND_PROMOTE(commGetRank(comm), dest, COMM_SSEND);
	func_cnt_array[rank]++;
	KEEP_BLOCK()
		;
	return MPI_SUCCESS;
	//return PMPI_Ssend(buf, count, datatype, dest, tag, comm);
}

/*added by yhb to recongize the slave pattern*/
int MPI_LBSend(void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm){
	FIRE_MPI_Slave();
	return MPI_Send(buf, count, datatype, dest, tag, comm);
}

int MPI_Send(void *buf, int count, MPI_Datatype datatype, int dest, int tag,
		MPI_Comm comm) {
	//MPI_Request rqst;
	//if (buf == NULL )
	//return MPI_ERR_BUFFER;
	int rank = commGetRank(comm);

	//PCS_rqstAlloc(&rqst, comm, EMPTY_RQST_TYPE);

#ifdef CHECK_MODE
	check_comm(comm);
	check_datatype (datatype, comm);
#endif
	if (BLOCKING_SEND) {
		FIRE_MPI_7OP(COMM_SSEND, rank, buf, count, datatype, dest, tag, comm,
				NULL);
		KEEP_BLOCK()
			;
	} else {
		int funcnt = func_cnt_array[rank];
		assert(
				funcnt< MAX_MPI_CALLS && "MPI call number exceeds MAX_MPI_CALLS");
		arg_array[rank][funcnt].request = malloc(sizeof(MPI_Request));
		CopyBuffer(buf, &(arg_array[rank][funcnt].buffer), datatype, count);
		CopyDataType(datatype, &(arg_array[rank][funcnt].type));

		FIRE_MPI_7OP(COMM_ISEND, rank, arg_array[rank][funcnt].buffer, count,
				arg_array[rank][funcnt].type, dest, tag, comm,
				arg_array[rank][funcnt].request);
	}
	//if (!BLOCKING_SEND)
	//	rqstFree(PCS_self() ->RqstTable, &rqst);
	func_cnt_array[rank]++;
	return MPI_SUCCESS;

}

int MPI_Test(MPI_Request *request, int *flag, MPI_Status *status) {

	NOTIFY_NONBLOCKING(-1, -1, COMM_TEST, (void * )request);
	return MPI_SUCCESS;
	//return MPI_Test(request, flag, status);
}

int MPI_Isend(void *buf, int count, MPI_Datatype datatype, int dest, int tag,
		MPI_Comm comm, MPI_Request *request) {
	//if (buf == NULL )
	//return MPI_ERR_BUFFER;
	int rank = commGetRank(comm);
#ifdef CHECK_MODE
	check_comm(comm);
	check_datatype(datatype, comm);
#endif

	int funcnt = func_cnt_array[rank];
	assert(funcnt< MAX_MPI_CALLS && "MPI call number exceeds MAX_MPI_CALLS");
	CopyBuffer(buf, &(arg_array[rank][funcnt].buffer), datatype, count);
	CopyDataType(datatype, &(arg_array[rank][funcnt].type));
	CopyRequest(request, &(arg_array[rank][funcnt].request));
#if 0
	printf("MPI_Isend (%d,%d) copyrequest: 0x%x -> 0x%x\n", rank, func_cnt_array[rank],request, arg_array[rank][funcnt].request);
#endif
	/*arg_array[rank][funcnt].comm = comm;
	 arg_array[rank][funcnt].count = count;
	 arg_array[rank][funcnt].func = COMM_ISEND;
	 arg_array[rank][funcnt].dest = dest;
	 arg_array[rank][funcnt].request = request;
	 arg_array[rank][funcnt].tag = tag;
	 arg_array[rank][funcnt].type = &datatype;*/

	FIRE_MPI_7OP(COMM_ISEND, rank, arg_array[rank][funcnt].buffer, count,
			arg_array[rank][funcnt].type, dest, tag, comm,
			arg_array[rank][funcnt].request);
	func_cnt_array[rank]++;
	return MPI_SUCCESS;
	//return PMPI_Isend(buf, count, datatype, dest, tag, comm, request);
}

int MPI_Irecv(void *buf, int count, MPI_Datatype datatype, int source, int tag,
		MPI_Comm comm, MPI_Request *req) {

#ifdef CHECK_MODE
	check_comm(comm);
	check_datatype(datatype, comm);
#endif
	int rank = commGetRank(comm);
	//int funcnt = func_cnt_array[rank];
	//assert(funcnt< MAX_MPI_CALLS && "MPI call number exceeds MAX_MPI_CALLS");
	/*arg_array[rank][funcnt].func = COMM_IRECV;
	 arg_array[rank][funcnt].buffer = buf;
	 arg_array[rank][funcnt].count = count;
	 arg_array[rank][funcnt].type = &datatype;
	 arg_array[rank][funcnt].source = source;
	 arg_array[rank][funcnt].tag = tag;
	 arg_array[rank][funcnt].comm = comm;
	 arg_array[rank][funcnt].request = req;

	 func_cnt_array[rank]++;

	 FIRE_MPI_7OP(arg_array[rank][funcnt].func, arg_array[rank][funcnt].buffer,arg_array[rank][funcnt].count,
	 arg_array[rank][funcnt].type,	arg_array[rank][funcnt].source,arg_array[rank][funcnt].tag,
	 arg_array[rank][funcnt].comm, arg_array[rank][funcnt].request);*/

	int funcnt = func_cnt_array[rank];
	assert(funcnt< MAX_MPI_CALLS && "MPI call number exceeds MAX_MPI_CALLS");
	CopyDataType(datatype, &(arg_array[rank][funcnt].type));
	CopyRequest(req, &(arg_array[rank][funcnt].request));
#if 0
	printf("MPI_Isend (%d,%d) copyrequest: 0x%x -> 0x%x\n", rank, func_cnt_array[rank],req, arg_array[rank][funcnt].request);
#endif

	FIRE_MPI_7OP(COMM_IRECV, rank, buf, count, arg_array[rank][funcnt].type,
			source, tag, comm, arg_array[rank][funcnt].request);
	func_cnt_array[rank]++;
	//FIRE_MPI_OP(&(arg_array[rank][funcnt]));
	return MPI_SUCCESS;
	//return PMPI_Irecv(buf, count, datatype, hookedsource, tag, comm, req);
}

int MPI_Wait(MPI_Request *request, MPI_Status *status) {
	int rank = commGetRank(MPI_COMM_WORLD);
	/*int funcnt = func_cnt_array[rank];
	 arg_array[rank][funcnt].request = request;
	 arg_array[rank][funcnt].status = status;

	 func_cnt_array[rank]++;*/
	//FIRE_MPI_OP(&(arg_array[rank][funcnt]));
	//NOTIFY_NONBLOCKING(-1, -1, COMM_WAIT, (void * )request);
	//MPI_Request * persistreq = (MPI_Request * )g_hash_table_lookup(table,request);
	MPI_Request * persistreq;
	if (find_entry(table, request, 1, (void **) &persistreq) < 0)
		printf("MPI_Wait, failed to fetch persist request.\n");
#if 0
	printf("wait get persist req: 0x%x -> 0x%x\n", request, persistreq);
#endif
	//FIRE_MPI_2OP(COMM_WAIT, rank, (void * )request, status);
	FIRE_MPI_2OP(COMM_WAIT, rank, (void * )persistreq, status);
	func_cnt_array[rank]++;
	KEEP_BLOCK()
		;
	return MPI_SUCCESS;
	//return PMPI_Wait(request, status);
}

int MPI_Waitall(int count, MPI_Request *reqs, MPI_Status *stats) {
	int rank = commGetRank(MPI_COMM_WORLD);
	if (reqs == NULL )
		return MPI_ERR_REQUEST;
	//if (stats == NULL )
	//	return MPI_ERR_IN_STATUS;
	if (count <= 0)
		return MPI_ERR_COUNT;

	for (int i = 0; i < count; i++) {
		MPI_Request *req = NULL;
		//printf("req:%x", req);
		//printf("MPI_Waitall reqs address: 0x%x\n",&reqs[i]);
		if (find_entry(table, &reqs[i], 1, (void **) &req) < 0)
			printf("MPI_Wait, failed to fetch persist request.\n");
		//else
		//printf("waitall get persist req: 0x%x -> 0x%x\n", &(reqs[i]), req);
		if(stats !=NULL)
		    FIRE_MPI_2OP(COMM_WAIT, rank, req, &stats[i]);
		else
			FIRE_MPI_2OP(COMM_WAIT, rank, req, NULL);
		func_cnt_array[rank]++;
		//FIRE_MPI_2OP(COMM_WAIT, rank, &reqs[i], &stats[i]);// g_hash_table_lookup(table,reqs[i]),stats[i]);//
	}
	KEEP_BLOCK()
		;
	return MPI_SUCCESS;
}
int MPI_Barrier(MPI_Comm comm) {
	int rank = commGetRank(comm);
#ifdef CHECK_MODE
	check_comm(comm);
#endif
//printf("Barrier comm :%x\n",&comm);

	/*int funcnt = func_cnt_array[rank];
	 arg_array[rank][funcnt].func =COMM_BARR;
	 arg_array[rank][funcnt].comm = comm;
	 func_cnt_array[rank]++;*/
	FIRE_MPI_1OP(COMM_BARR, rank, comm);
	func_cnt_array[rank]++;
//FIRE_MPI_OP(&(arg_array[rank][funcnt]));
	KEEP_BLOCK()
		;
	return MPI_SUCCESS;
//return PMPI_Barrier(comm);
}

int MPI_Init(int *pargc, char *** pargv) {
	int res = PMPI_Init(pargc, pargv);
	int rank = commGetRank(MPI_COMM_WORLD);
//printf("rank %d init\n",rank);
	FIRE_MPI_2OP(COMM_INIT, rank, pargc, pargv);
	return res;
}

int MPI_Finalize(void) {
	int rank = commGetRank(MPI_COMM_WORLD);
	FIRE_MPI_0OP(COMM_FIN, rank);
	func_cnt_array[rank]++;
	KEEP_BLOCK()
		;
	return PMPI_Finalize();
	/*int i=0;
	 bool res=true;
	 while(++i){
	 res= klee_keep_block();
	 if(!res)
	 break;
	 }*/
	//printf("rank %d come out from keep_block in finalize. loop times:%d, block returned:%d\n",rank,i,res);
}

int MPI_Comm_rank(MPI_Comm comm, int *rank) {
	int res = PMPI_Comm_rank(comm, rank);
	return res;
}

int req2index(int rank, int curindex, MPI_Request *rqst) {
	assert(rank >= 0);
	int funcnt = func_cnt_array[rank];
	int i = curindex - 1;
	for (; i >= 0; i--) {
		if (arg_array[rank][i].request == rqst)
			return i;
	}
	return -1;
}
//NOTE!!!!
// OTHER FIRE_MPI_XOP is supposed to use parameters as:
// (OP,rank, para1, para2...),
// FIRE_MPI_4OP is DIFFERENT: FIRE_MPI_4OP(WAITANY,rank,isends/ireceives' index, para1...)

// I.e., if rank0 get these ops: isend1(req0), isend(req1), waitany(2,req[0..1])
// we'll use: FIRE_MPI_4OP(WAITANY,0,[0..1], para1...), where [0..1] would be a array.
int MPI_Waitany(int count, MPI_Request *rqst, int *index, RQST_Status *status) {
	int rank = commGetRank(MPI_COMM_WORLD);
	int temp;

	MPI_Request ** persistreq = malloc(count * sizeof(MPI_Request*));
	int * ancestor = malloc(count * sizeof(int));
	for (int i = 0; i < count; i++) {
		MPI_Request *req = NULL;
		if (find_entry(table, &rqst[i], 1, (void **) &req) < 0)
			printf("MPI_Waitany, failed to fetch persist request.\n");
		//else
		//printf("waitany get persist req: 0x%x -> 0x%x\n", &(rqst[i]), req);

		temp = req2index(rank, func_cnt_array[rank], req);
		if (temp != -1)
			ancestor[i] = temp;
		else
			printf("Waitany get ancestor failed!\n");
#if 0
		printf("MPI_Waitany, %d/%d, catch ancestor %d.\n", rank,
				func_cnt_array[rank], ancestor[i]);
#endif
		persistreq[i] = req;
	}
	// test persistreq
	//printf("MPI_Waitany, persistreq[0]:%x,*persistreq[0]:%x.\n", persistreq[0],*(persistreq[0]));

	FIRE_MPI_4OP(COMM_WAITANY, rank, ancestor, count, persistreq[0], index,
			status);
	func_cnt_array[rank]++;
	KEEP_BLOCK()
		;
	for (int i = 0; i < count; i++) {
		if (*(persistreq[i]) != NULL ) {
			//printf("MPI_Waitany, index:%d\n", i);
			*index = i;
			break;
		}
	}
	free(persistreq);
	return MPI_SUCCESS;
}
//#define DEBUG_MODE
int MPI_Sendrecv(void *sendbuf, int sendcount, MPI_Datatype sendtype, int dest,
		int sendtag, void *recvbuf, int recvcount, MPI_Datatype recvtype,
		int source, int recvtag, MPI_Comm comm, MPI_Status *status) {
	int mpi_errno;
	MPI_Request request;
	int rank = commGetRank(comm);
#ifdef DEBUG_MODE
	fprintf(stdout,
			"MPI_Sendrecv(start)\tProcess: %d, d/s:%d,%d, \t scount:%d, rcount:%d\n",
			rank, dest, source, sendcount, recvcount);
#endif

#ifdef CHECK_MODE
	if (mpi_errno = check_comm(comm)) goto mpi_exception;
	if (mpi_errno = check_tag(sendtag)) goto mpi_exception;
	if (mpi_errno = check_tag(recvtag)) goto mpi_exception;
	if (mpi_errno = check_dest_comm(dest, comm)) goto mpi_exception;
	if (mpi_errno = check_source_comm(source, comm)) goto mpi_exception;
	if (mpi_errno = check_count(sendcount)) goto mpi_exception;
	if (mpi_errno = check_count(recvcount)) goto mpi_exception;
	if (mpi_errno = check_datatype(sendtype)) goto mpi_exception;
	if (mpi_errno = check_datatype(recvtype)) goto mpi_exception;
#endif

//func_cnt_array[rank]++;

// 1. For avoiding interblocking it uses a non-blocking receive
	MPI_Irecv(recvbuf, recvcount, recvtype, source, recvtag, comm, &request);
	MPI_Send(sendbuf, sendcount, sendtype, dest, sendtag, comm);
//CALL_MPI_NEST(MPI_Irecv(recvbuf, recvcount, recvtype, source, recvtag, comm,	&request));
//CALL_MPI_NEST(MPI_Send(sendbuf, sendcount, sendtype, dest, sendtag, comm));

	/* 2. Wait for receiving only if source is not MPI_PROC_NULL */
	if ((source == MPI_PROC_NULL)&& (status != MPI_STATUS_IGNORE)){
	STATUS_setNull(status);
} else {
	//FIRE_MPI_2OP(COMM_WAIT, rank, (void * )&request, status);
	MPI_Wait(&request, status);
}

#ifdef DEBUG_MODE
	fprintf(stdout, "MPI_Sendrecv(end)\tProcess: %d\n", rank);
#endif
	return MPI_SUCCESS;
	mpi_exception: return commHandleError(comm, mpi_errno, "MPI_Sendrecv");
}

int MPI_Sendrecv_replace(void *buf, int count, MPI_Datatype datatype, int dest,
		int sendtag, int source, int recvtag, MPI_Comm comm, MPI_Status *status) {

	int mpi_errno;
	MPI_Request request;
	char *tmpbuf;
	int size;
	int pos;
	int rank = commGetRank(comm);
#ifdef DEBUG_MODE
	fprintf(stdout,
			"MPI_Sendrecv_replace (start)\tProcess%d, d/s:%d,%d,  count:%d\n",
			rank, dest, source, count);
#endif

#ifdef CHECK_MODE
	if (mpi_errno = check_comm(comm)) goto mpi_exception;
	if (mpi_errno = check_tag(sendtag)) goto mpi_exception;
	if (mpi_errno = check_tag(recvtag)) goto mpi_exception;
	if (mpi_errno = check_dest_comm(dest, comm)) goto mpi_exception;
	if (mpi_errno = check_source_comm(source, comm)) goto mpi_exception;
	if (mpi_errno = check_count(count)) goto mpi_exception;
	if (mpi_errno = check_datatype(datatype)) goto mpi_exception;
	if (buf == NULL) {mpi_errno = MPI_ERR_BUFFER; goto mpi_exception;}
#endif

	NEST_FXN_INCR() ;

	if (dtypeIsContiguous(datatype)) {

		size = dtypeGetExtent(datatype) * count;

		CALL_FXN(MALLOC(tmpbuf, size), MPI_ERR_INTERN);
		pos = 0;
		pack(buf, count, datatype, tmpbuf, size, &pos);

		/* 1.1. For avoiding interblocking it uses a non-blocking receive */
		//FIRE_MPI_7OP(COMM_IRECV, rank, buf, count, datatype, source,    		recvtag, comm, &request);
		//FIRE_MPI_7OP(COMM_SSEND, rank, tmpbuf, count, datatype, dest, sendtag,    		comm, NULL);
		//func_cnt_array[rank] += 2;
		MPI_Irecv(buf, count, datatype, source, recvtag, comm, &request);
		MPI_Send(tmpbuf, count, datatype, dest, sendtag, comm);

		/* 1.2. Wait for receiving only if source is not MPI_PROC_NULL */
		if ((source == MPI_PROC_NULL)&& (status != MPI_STATUS_IGNORE)){
		STATUS_setNull(status);
	} else {
		//FIRE_MPI_2OP(COMM_WAIT, rank, (void * )&request, status);
		//func_cnt_array[rank]++;
		//CALL_MPI_NEST(MPI_Wait  (&request, status));
		MPI_Wait (&request, status);
	}

		FREE(tmpbuf);

	} else {

		size = packSize(count, datatype);

		CALL_FXN(MALLOC(tmpbuf, size), MPI_ERR_INTERN);
		pos = 0;
		pack(buf, count, datatype, tmpbuf, size, &pos);
		// FIRE_MPI_7OP(COMM_IRECV, rank, buf,    count, datatype,   source, recvtag, comm, &request);
		// FIRE_MPI_7OP(COMM_SSEND, rank, tmpbuf, size, MPI_PACKED, dest, sendtag,comm, NULL);
		//func_cnt_array[rank] += 2;
		MPI_Irecv(buf, count, datatype, source, recvtag, comm, &request);
		MPI_Send(tmpbuf, size, MPI_PACKED, dest, sendtag, comm);

		/* 2.1. Wait for receiving only if source is not MPI_PROC_NULL */
		if ((source == MPI_PROC_NULL)&& (status != MPI_STATUS_IGNORE)){
		STATUS_setNull(status);
	} else {
		//FIRE_MPI_2OP(COMM_WAIT, rank, (void * )&request, status);
		//func_cnt_array[rank]++;
		MPI_Wait (&request, status);
	}

		FREE(tmpbuf);

	}

	NEST_FXN_DECR() ;

#ifdef DEBUG_MODE
	fprintf(stdout, "MPI_Sendrecv_replace (end)  \tProcess: %d\n", rank);
#endif

	return MPI_SUCCESS;

	mpi_exception_unnest:
	NEST_FXN_DECR() ;

	mpi_exception:

	return commHandleError(comm, mpi_errno, "MPI_Sendrecv_replace");
}

int MPI_Alltoall(void *sendbuf, int sendcount, MPI_Datatype sendtype,
		void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm) {

	int mpi_errno;
	int i;
	int rank;
	int gsize;
	int dtsendsz, dtrecvsz;
	int src, dst;

#ifdef CHECK_MODE
	if (mpi_errno = check_comm(comm)) goto mpi_exception;
	if (mpi_errno = check_comm_type(comm, INTRACOMM)) goto mpi_exception;
	if (mpi_errno = check_group(commGetLocalGroup(comm))) goto mpi_exception;
	if (mpi_errno = check_count(sendcount)) goto mpi_exception;
	if (mpi_errno = check_count(recvcount)) goto mpi_exception;
	if (mpi_errno = check_datatype(sendtype)) goto mpi_exception;
	if (mpi_errno = check_datatype(recvtype)) goto mpi_exception;
	if (mpi_errno = check_dtype_commit(sendtype)) goto mpi_exception;
	if (mpi_errno = check_dtype_commit(recvtype)) goto mpi_exception;
#endif

	if (sendcount == 0)
		return MPI_SUCCESS;

	rank = commGetRank(comm);
	gsize = commGetSize(comm);
#ifdef DEBUG_MODE
	fprintf(stdout, "MPI_Alltoall (start)\tProcess: %d\n", rank);
#endif
	dtsendsz = dtypeGetExtent(sendtype) * sendcount;
	dtrecvsz = dtypeGetExtent(recvtype) * recvcount;

	NEST_FXN_INCR() ;

	for (i = 0; i < gsize; i++) {
		src = (rank - i + gsize) % gsize;
		dst = (rank + i) % gsize;

		MPI_Sendrecv(((char *) sendbuf + dst * dtsendsz), sendcount, sendtype,
				dst, ALLTOALL_TAG, ((char *) recvbuf + src * dtrecvsz),
				recvcount, recvtype, src, ALLTOALL_TAG, comm, NULL );
	}

	NEST_FXN_DECR() ;

#ifdef DEBUG_MODE
	fprintf(stdout, "MPI_Alltoall (end)  \tProcess: %d\n", rank);
#endif

	return MPI_SUCCESS;

	mpi_exception_unnest:
	NEST_FXN_DECR() ;

	mpi_exception: return commHandleError(comm, mpi_errno, "MPI_Alltoall");
}

int MPI_Alltoallv(void *sendbuf, int *sendcounts, int *sdispls,
		MPI_Datatype sendtype, void *recvbuf, int *recvcounts, int *rdispls,
		MPI_Datatype recvtype, MPI_Comm comm) {

	int mpi_errno;
	int i;
	int rank;
	int gsize;
	int dtsendsz, dtrecvsz;
	int src, dst;

#ifdef CHECK_MODE
	if (mpi_errno = check_comm(comm)) goto mpi_exception;
	if (mpi_errno = check_comm_type(comm, INTRACOMM)) goto mpi_exception;
	if (mpi_errno = check_group(commGetLocalGroup(comm))) goto mpi_exception;
	if (mpi_errno = check_datatype(sendtype)) goto mpi_exception;
	if (mpi_errno = check_datatype(recvtype)) goto mpi_exception;
	if (mpi_errno = check_dtype_commit(sendtype)) goto mpi_exception;
	if (mpi_errno = check_dtype_commit(recvtype)) goto mpi_exception;
#endif

	rank = commGetRank(comm);
	gsize = commGetSize(comm);
#ifdef DEBUG_MODE
	fprintf(stdout, "MPI_Alltoallv (start)\tProcess: %d\n", rank);
#endif
#ifdef CHECK_MODE
	for (i = 0; i < gsize; i++) {
		if (mpi_errno = check_count(sendcounts[i])) goto mpi_exception;
		if (mpi_errno = check_count(recvcounts[i])) goto mpi_exception;
	}
#endif

	NEST_FXN_INCR() ;

	dtsendsz = dtypeGetExtent(sendtype);
	dtrecvsz = dtypeGetExtent(recvtype);

	for (i = 0; i < gsize; i++) {
		src = (rank - i + gsize) % gsize;
		dst = (rank + i) % gsize;

		MPI_Sendrecv(((char *) sendbuf + sdispls[dst] * dtsendsz),
				sendcounts[dst], sendtype, dst, ALLTOALL_TAG,
				((char *) recvbuf + rdispls[src] * dtrecvsz), recvcounts[src],
				recvtype, src, ALLTOALL_TAG, comm, NULL );
	}

	NEST_FXN_DECR() ;

#ifdef DEBUG_MODE
	fprintf(stdout, "MPI_Alltoallv (end)  \tProcess: %d\n", rank);
#endif

	return MPI_SUCCESS;

	mpi_exception_unnest:
	NEST_FXN_DECR() ;

	mpi_exception: return commHandleError(comm, mpi_errno, "MPI_Alltoallv");
}

int __smp_reduce_reference(const void * const sendbuf, void *recvbuf,
		const int count, const MPI_Datatype datatype, const MPI_Op op,
		const int root, const MPI_Comm comm) {
	int *locK, i, amount, idx;
	void *rootRecvBuff;
	void *sendAddr, *recvAddr;

	long long difference;
	int me = commGetRank(comm);
	int size = commGetSize(comm);
	int nBytes = dtypeGetExtent(datatype);

	if (me == root) {
		rootRecvBuff = recvbuf;
		memset(recvbuf, 0, count * nBytes);
	}
#ifdef DEBUG_MODE
	fprintf(stdout, "__smp_reduce_reference (start)\tProcess: %d\n", me);
#endif
	MPI_Bcast(&rootRecvBuff, sizeof(void *), MPI_BYTE, root, comm);
	difference = sendbuf - rootRecvBuff;

	int chunkSize = count / (size);
	int chunkRest = count % (size);
	int period = chunkSize * nBytes;

	for (i = 0; i < size; i++) {
		idx = i + me;
		if (idx >= size)
			idx -= size;

		amount = (idx == size - 1 ? chunkSize + chunkRest : chunkSize);
		recvAddr = &((char *) rootRecvBuff)[idx * period];
		sendAddr = recvAddr + difference;
		//locK = &comm->comm_root->ReduceLock[idx].Cerrojo;

		//lock(locK);
		(copsGetFunction(op))(sendAddr, recvAddr, &amount, datatype);
		//unlock(locK);
	}
	//MPI_Barrier(comm);

	if (me == root) {
		for (i = 0; i < size; i++) {
			if (i == me)
				continue;
#ifdef DEBUG_MODE
			fprintf(stdout,
					"__smp_reduce_reference ,recv from %d,  \tProcess: %d\n", i,
					me);
#endif
			MPI_Recv(NULL, 0, MPI_INT, i, REDUCE_TAG, comm, MPI_STATUS_IGNORE );
		}
	} else {
#ifdef DEBUG_MODE
		fprintf(stdout, "__smp_reduce_reference ,send to %d,  \tProcess: %d\n",
				root, me);
#endif
		MPI_Send(NULL, 0, MPI_INT, root, REDUCE_TAG, comm);
	}
#ifdef DEBUG_MODE
	fprintf(stdout, "__smp_reduce_reference (end)  \tProcess: %d\n", me);
#endif
	return MPI_SUCCESS;
}

int __lin_reduce (void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm) {

	int          mpi_errno;
	int          rank;
	int          size;
	char        *tmpbuf  = NULL;
	int          dtsizebytes;
	int          i;
	MPI_Request  req;


	NEST_FXN_INCR();

	rank = commGetRank(comm);

	/* 1. Root: Receive messages and operate on them */
	if (rank == root) {
		size        = commGetSize(comm);
		dtsizebytes = count * dtypeGetExtent(datatype);

		/* 1.1. Only malloc at root for temporary buffer */
		CALL_FXN (MALLOC(tmpbuf, dtsizebytes), MPI_ERR_INTERN);

		/* 1.2. No use memcpy or pack. Receive+Send avoids errors with derived non-contiguous datatypes */
		if (root == 0) {

			if (0 > copyData (sendbuf, recvbuf, count, datatype, root, comm))         goto mpi_exception_unnest;

		} else {

			MPI_Recv (recvbuf, count, datatype, 0, REDUCE_TAG, comm, NULL);

		}

		for (i = 1; i < size; i++) {

			/* 1.3. No use memcpy or pack. Receive+Send avoids errors with derived non-contiguous datatypes */
			if (root == i) {

				if (0 > copyData (sendbuf, tmpbuf, count, datatype, root, comm))        goto mpi_exception_unnest;

			} else {

				MPI_Recv (tmpbuf, count, datatype, i, REDUCE_TAG, comm, NULL);

			}

			(copsGetFunction(op)) (tmpbuf, recvbuf, &count, datatype);
		}

		FREE (tmpbuf);

		/* 2. I am not the root, send the buffer. */
	} else {

		MPI_Send (sendbuf, count, datatype, root, REDUCE_TAG, comm);

	}

	NEST_FXN_DECR();

	return MPI_SUCCESS;

	mpi_exception_unnest:
	NEST_FXN_DECR();

	mpi_exception:
	if (tmpbuf) free(tmpbuf);
	return commHandleError (comm, mpi_errno, "MPI_Reduce");
}

int __net_reduce (void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm) {
	return __lin_reduce(sendbuf, recvbuf, count, datatype, op, root, comm);
}



int MPI_Reduce(void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
		MPI_Op op, int root, MPI_Comm comm) {
	int mpi_errno;
	int rank;
	int size;
	char *tmpbuf = NULL;
	int dtsizebytes;
	int i, me;
	MPI_Request req;

	me = commGetRank(comm);
#ifdef DEBUG_MODE
	fprintf(stdout, "MPI_Reduce (start)\tProcess: %d\n", me);
#endif

#ifdef CHECK_MODE
	if (mpi_errno = check_comm(comm)) goto mpi_exception;
	if (mpi_errno = check_comm_type(comm, INTRACOMM)) goto mpi_exception;
	if (mpi_errno = check_dest_comm(root, comm)) goto mpi_exception;
	if (mpi_errno = check_count(count)) goto mpi_exception;
	if (mpi_errno = check_datatype(datatype)) goto mpi_exception;
	if (mpi_errno = check_dtype_commit(datatype)) goto mpi_exception;
	if (mpi_errno = check_ops(op)) goto mpi_exception;
#endif
	if (count == 0)
		return MPI_SUCCESS;

	//mpi_errno = __smp_reduce_reference(sendbuf, recvbuf, count, datatype, op, root, comm);
	// modifed by zhenbang to use net mode
	mpi_errno = __net_reduce(sendbuf, recvbuf, count, datatype, op, root, comm);

	if (0 > mpi_errno)
		goto mpi_exception;

#ifdef DEBUG_MODE
	fprintf(stdout, "MPI_Reduce (end)  \tProcess: %d\n", me);
#endif
	return MPI_SUCCESS;

	mpi_exception_unnest:
	NEST_FXN_DECR() ;

	mpi_exception: if (tmpbuf)
		free(tmpbuf);
	return commHandleError(comm, mpi_errno, "MPI_Reduce");
}

int __smp_broadCast_value(void *buffer, int count, Mpi_P_Datatype_t dtype,
		int root, Mpi_P_Comm_t comm) {
	int mpi_errno;
	int i;
	int me;
	int gsize;
	int dtsz;

	NEST_FXN_INCR() ;

	me = commGetRank(comm);

#ifdef DEBUG_MODE
	fprintf(stdout, "__smp_broadCast_value(%d),root:%d, : BEGIN\n", me,root);
	fflush(stdout);
#endif
	if (me == root) {
		gsize = commGetSize(comm);
		dtsz = dtypeGetExtent(dtype);

		for (i = 0; i < gsize; i++) {
			if (i == me)
				continue;
			MPI_Send(buffer, count, dtype, i, BCAST_TAG, comm);
		}
	} else {
		/* Receive the data from the root, itself included */
		MPI_Recv(buffer, count, dtype, root, BCAST_TAG, comm, NULL );
	}
	NEST_FXN_DECR() ;

#ifdef DEBUG_MODE
	fprintf(stdout, "__smp_broadCast_value(%d): END\n", me);
	fflush(stdout);
#endif

	return MPI_SUCCESS;

	mpi_exception_unnest:
	NEST_FXN_DECR() ;

	mpi_exception: return commHandleError(comm, mpi_errno,
			"__smp_broadCast_value");
}

int __smp_broadCast_reference(void *buffer, int count, Mpi_P_Datatype_t dtype,
		int root, Mpi_P_Comm_t comm) {
	void *rootBuff;
	void *srcAddr, *dstAddr;
	const int me = commGetRank(comm);
	const int size = commGetSize(comm);
	const int dtsz = dtypeGetExtent(dtype);
	const int nBytes = count * dtsz;
	int i, dstRank;

	if (me == root) {
		rootBuff = buffer;
	}
	__smp_broadCast_value(&rootBuff, sizeof(void *), MPI_BYTE, root, comm);

	if (me == root) {
		for (i = 0; i < size; i++) {
			if (i == me)
				continue;
			MPI_Recv(NULL, 0, MPI_INT, i, REDUCE_TAG, comm, MPI_STATUS_IGNORE );
		}
	} else {
		dstAddr = buffer;
		srcAddr = rootBuff;
		memcpy(dstAddr, srcAddr, nBytes);
		MPI_Send(NULL, 0, MPI_INT, root, REDUCE_TAG, comm);

	}
	return MPI_SUCCESS;
}

int __smp_broadCast(void *buffer, int count, Mpi_P_Datatype_t dtype, int root,
		Mpi_P_Comm_t comm) {
//	int nBytes = count * dtypeGetExtent(dtype);
//	if (nBytes <= 192) {
//		return (__smp_broadCast_value(buffer, count, dtype, root, comm));
//	} else
//		return (__smp_broadCast_reference(buffer, count, dtype, root, comm));
	// modified by zhenbang to enforce communication operations
	return (__smp_broadCast_value(buffer, count, dtype, root, comm));
}

int MPI_Bcast(void *buffer, int count, MPI_Datatype datatype, int root,
		MPI_Comm comm) {

	int mpi_errno;
	int mask;
	int relative_rank;
	int rank;
	int size;
	int src, dst;
	int me = commGetRank(comm);
#ifdef DEBUG_MODE
	fprintf(stdout, "MPI_Bcast (start)\tProcess: %d\n", me);
#endif

#ifdef CHECK_MODE
	if (mpi_errno = check_comm(comm)) goto mpi_exception;
	if (mpi_errno = check_comm_type(comm, INTRACOMM)) goto mpi_exception;
	if (mpi_errno = check_datatype(datatype)) goto mpi_exception;
	if (mpi_errno = check_dtype_commit(datatype)) goto mpi_exception;
	if (mpi_errno = check_root(root, comm)) goto mpi_exception;
	if (mpi_errno = check_count(count)) goto mpi_exception;
#endif
	if (count == 0)
		return MPI_SUCCESS;

	mpi_errno = __smp_broadCast(buffer, count, datatype, root, comm);

	if (0 > mpi_errno)
		goto mpi_exception;

#ifdef DEBUG_MODE
	fprintf(stdout, "MPI_Bcast (end)  \tProcess: %d\n", me);
#endif

	return MPI_SUCCESS;

	mpi_exception: return commHandleError(comm, mpi_errno, "MPI_Bcast");
}

int MPI_Allreduce (void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm) {
    int   mpi_errno;
	int me = commGetRank(comm);
#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Allreduce (start)\tProcess: %d\n", me);
#endif

  NEST_FXN_INCR();

  /* 1. Call reduce with root = 0 */
  MPI_Reduce(sendbuf, recvbuf, count, datatype, op, 0, comm);

  /* 2. Broadcast the result to other processes */
  MPI_Bcast(recvbuf, count, datatype, 0, comm);

  NEST_FXN_DECR();

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Allreduce (end)  \tProcess: %d\n", me);
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
  NEST_FXN_DECR();

mpi_exception:

  return commHandleError (comm, mpi_errno, "MPI_Allreduce");
}

int __commCreateMCC (Mpi_P_Comm *comm) {

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

int __comm_create (MPI_Comm intracomm, MPI_Group local_group, MPI_Group remote_group,
						int context, int type, void *errhnd,
						MPI_Comm *newcomm)   {

  int  mpi_errno;


  CALL_FXN (PCS_commCreate (intracomm, local_group, remote_group, DEFAULT_ATTRIBUTES_COUNT,
							context, type, errhnd, newcomm), MPI_ERR_COMM);

  __commCreateMCC(*newcomm);


  return MPI_SUCCESS;

mpi_exception_unnest:
  return mpi_errno;
}

int MPI_Comm_create (MPI_Comm comm, MPI_Group group, MPI_Comm *newcomm) {

  int       mpi_errno;
  MPI_Group newgroup;
  int       p_commNr;
  int       r_commNr;
  int       rank;

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Comm_create (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
  if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_comm_type(comm, INTRACOMM))        goto mpi_exception;
  if (mpi_errno = check_group(group))                      goto mpi_exception;
#endif

  NEST_FXN_INCR();

  *newcomm = (MPI_Comm) MPI_COMM_NULL;

  /* 1. AllReduce operation for get a new communicator number. Tasks propose a number
        and the max is elected. All ranks take part in the election.
   */
  p_commNr = PCS_commGetNrMax() + 1;
  CALL_MPI_NEST(MPI_Allreduce (&p_commNr, &r_commNr, 1, MPI_INT, MPI_MAX, comm));

  /* 2. Caller task must be in the group in newcomm, else return ok */
  if (0 > (rank = groupGetLocalRank(group)))               goto retorno;

  /* 3. Only tasks in new group create new communicator entries */
  groupGetRef(group, &newgroup);
  CALL_FXN (__comm_create (comm, newgroup, NULL, r_commNr, INTRACOMM, NULL, newcomm), MPI_ERR_COMM);

  /* 4. Copy the DEFAULT attributes to the new communicator */
  CALL_FXN (PCS_keyCopyDfltAttr(comm, *newcomm), MPI_ERR_OTHER);

retorno:
  NEST_FXN_DECR();

#ifdef DEBUG_MODE
  fprintf(stdout, "MPI_Comm_create (end)  \tProcess: 0x%x\n", PCS_self());
#endif

  return MPI_SUCCESS;

mpi_exception_unnest:
  NEST_FXN_DECR();

mpi_exception:
  return commHandleError(comm, mpi_errno, "MPI_Comm_create");
}

int __smp_gather_value(void *sendbuf, int sendcount, MPI_Datatype sendtype,
					 void *recvbuf, int recvcount, MPI_Datatype recvtype,
					 int   root, MPI_Comm comm)
{
	int     me   = commGetRank(comm);
	int     size = commGetSize(comm);
	int     i;
	char    tmpBuff[1024*8] __attribute__(( aligned(CACHE_LINE_SIZE) ));
	int     nBytes = sendcount * dtypeGetExtent(sendtype);

	if (me == root) {
		copyData (sendbuf, recvbuf + me * nBytes, sendcount, sendtype, root, comm);
		for (i = 0; i < size; i++) {
			if (me == i)  continue;
			MPI_Recv (recvbuf + i * nBytes, recvcount, recvtype, i, GATHER_TAG, comm, MPI_STATUS_IGNORE);
		}
	}
	else
		MPI_Send (sendbuf, sendcount, sendtype, root, GATHER_TAG, comm);
	return MPI_SUCCESS;
}




int __smp_gather_reference (void *sendbuf, int sendcount, MPI_Datatype sendtype,
						  void *recvbuf, int recvcount, MPI_Datatype recvtype,
						  int   root,    MPI_Comm comm)
{
	int          i;
	void        *rootRecvBuff;
	void        *srcAddr, *dstAddr;

	long long    difference;
	const int    me     = commGetRank(comm);
	const int    size   = commGetSize(comm);
	const int    nBytes = sendcount * dtypeGetExtent(sendtype);

	if (me == root) {
		rootRecvBuff = recvbuf;
	}

	treeContig_bcast(&rootRecvBuff, sizeof(void *), MPI_BYTE, root, comm);

	dstAddr = rootRecvBuff + (me * nBytes);
	srcAddr = sendbuf;
	//if(nBytes)
	memcpy(dstAddr, srcAddr, nBytes );


	if (me == root) {
		for (i = 0; i < size; i++) {
			if(i == me) continue;
			MPI_Recv (NULL, 0, MPI_INT, i, GATHER_TAG, comm, MPI_STATUS_IGNORE);
		}
	}
	else
		MPI_Send (NULL, 0, MPI_INT, root, GATHER_TAG, comm);

	return MPI_SUCCESS;
}


int __smp_gather(void *sendbuf, int sendcount, MPI_Datatype sendtype,
			   void *recvbuf, int recvcount, MPI_Datatype recvtype,
			   int   root, MPI_Comm comm)
{
	int nBytes = sendcount * dtypeGetExtent(sendtype);
	if(nBytes <= (128))
		return(__smp_gather_value(sendbuf, sendcount, sendtype,
								recvbuf, recvcount, recvtype,
								root, comm));
	else
		return(__smp_gather_reference(sendbuf, sendcount, sendtype,
									recvbuf, recvcount, recvtype,
									root, comm));
}

int __net_gather (void *sendbuf, int sendcount, MPI_Datatype sendtype,
				void *recvbuf, int recvcount, MPI_Datatype recvtype,
				int root, MPI_Comm comm )  {

	int          mpi_errno;
	int          i;
	int          rank;
	int          gsize;
	int          dtrecvsz;
	MPI_Request  req;

#ifdef DEBUG_MODE
	fprintf(stdout, "MPI_Gather (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
	if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_comm_type(comm, INTRACOMM))        goto mpi_exception;
  if (mpi_errno = check_group(commGetLocalGroup(comm)))    goto mpi_exception;
  if (mpi_errno = check_count(sendcount))                  goto mpi_exception;
  if (mpi_errno = check_datatype(sendtype))                goto mpi_exception;
  if (mpi_errno = check_dtype_commit(sendtype))            goto mpi_exception;
#endif

	NEST_FXN_INCR();

	rank  = commGetRank(comm);
	if (rank == root) {
		gsize = commGetSize(comm);

#ifdef CHECK_MODE
		if (mpi_errno = check_count(recvcount))                goto mpi_exception_unnest;
    if (mpi_errno = check_datatype(recvtype))              goto mpi_exception_unnest;
    if (mpi_errno = check_dtype_commit(recvtype))          goto mpi_exception_unnest;
    if (mpi_errno = check_root(root, comm))                goto mpi_exception_unnest;
#endif

		dtrecvsz = dtypeGetExtent(recvtype) * recvcount;

		for (i = 0; i < gsize; i++) {
			if (i == root) { /* root copy the buffer without sending it */
				MPI_Irecv((char *)recvbuf + i * dtrecvsz,
										recvcount, recvtype, i, GATHER_TAG, comm, &req);
			}
			else { /* root receive from the other processes */
				MPI_Recv( (char *)recvbuf + i * dtrecvsz,
										recvcount, recvtype, i, GATHER_TAG, comm, NULL);
			}
		}
	}

	/* Send the data to root, itself included */
	MPI_Send(sendbuf, sendcount, sendtype, root, GATHER_TAG, comm);

	if (rank == root) { /* Deallocate request by root */
		MPI_Wait(&req, NULL);
	}

	NEST_FXN_DECR();

#ifdef DEBUG_MODE
	fprintf(stdout, "net_Gather (end)  \tProcess: 0x%x\n", PCS_self());
#endif

	return MPI_SUCCESS;

	mpi_exception_unnest:
	NEST_FXN_DECR();

	mpi_exception:
	return commHandleError (comm, mpi_errno, "net_Gather");
}


int MPI_Gather (void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype,
				int root, MPI_Comm comm ){
	int          mpi_errno;
	int          rank;
	int          size;
	char        *tmpbuf  = NULL;
	int          dtsizebytes;
	int          i;
//	MPI_Request  req;

#ifdef DEBUG_MODE
	fprintf(stdout, "MPI_Gather(start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
    if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_comm_type(comm, INTRACOMM))        goto mpi_exception;
  if (mpi_errno = check_dest_comm(root, comm))             goto mpi_exception;
  if (mpi_errno = check_count(sendcount))                      goto mpi_exception;
  if (mpi_errno = check_count(recvcount))                      goto mpi_exception;
  if (mpi_errno = check_datatype(sendtype))                goto mpi_exception;
  if (mpi_errno = check_datatype(recvtype))                goto mpi_exception;
  //if (mpi_errno = check_datatype(datatype))                goto mpi_exception;
  //if (mpi_errno = check_dtype_commit(datatype))            goto mpi_exception;
#endif
	if(sendcount == 0)
		return MPI_SUCCESS;

//	mpi_errno = __smp_gather(sendbuf, sendcount, sendtype,
//							   recvbuf, recvcount, recvtype,
//							   root, comm);
	mpi_errno = __net_gather(sendbuf, sendcount, sendtype,
						   recvbuf, recvcount, recvtype,
						   root, comm);


	if (0 > mpi_errno)                                       goto mpi_exception;

#ifdef DEBUG_MODE
	fprintf(stdout, "MPI_Gather (end)  \tProcess: 0x%x\n", PCS_self());
#endif

	return MPI_SUCCESS;

//	mpi_exception_unnest:
//	NEST_FXN_DECR();

	mpi_exception:
	if (tmpbuf) free(tmpbuf);
	return commHandleError (comm, mpi_errno, "MPI_Gather");
}

int MPI_Allgather ( void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype,
					MPI_Comm comm ){
	int  mpi_errno;
	int  root;
	int  gsize;

#ifdef DEBUG_MODE
	fprintf(stdout, "MPI_Allgather (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
    if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_comm_type(comm, INTRACOMM))        goto mpi_exception;
  if (mpi_errno = check_group(commGetLocalGroup(comm)))    goto mpi_exception;
  if (mpi_errno = check_count(sendcount))                  goto mpi_exception;
  if (mpi_errno = check_count(recvcount))                  goto mpi_exception;
  if (mpi_errno = check_datatype(sendtype))                goto mpi_exception;
  if (mpi_errno = check_datatype(recvtype))                goto mpi_exception;
  if (mpi_errno = check_dtype_commit(sendtype))            goto mpi_exception;
  if (mpi_errno = check_dtype_commit(recvtype))            goto mpi_exception;
#endif

	NEST_FXN_INCR();

	gsize = commGetSize(comm);

	for (root = 0; root < gsize; root++) {

		MPI_Gather(sendbuf, sendcount, sendtype,
								 recvbuf, recvcount, recvtype, root, comm);

	}

	NEST_FXN_DECR();

#ifdef DEBUG_MODE
	fprintf(stdout, "MPI_Allgather (end)  \tProcess: 0x%x\n", PCS_self());
#endif

	return MPI_SUCCESS;

	mpi_exception_unnest:
	NEST_FXN_DECR();

	mpi_exception:
	return commHandleError (comm, mpi_errno, "MPI_Allgather");
}

int MPI_Gatherv (void *sendbuf, int sendcount, MPI_Datatype sendtype,
				 void *recvbuf, int *recvcounts, int *displs, MPI_Datatype recvtype,
				 int root, MPI_Comm comm )  {

	int          mpi_errno;
	int          i;
	int          rank;
	int          gsize;
	int          dtrecvsz;
	MPI_Request  req;

#ifdef DEBUG_MODE
	fprintf(stdout, "MPI_Gatherv (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
	if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_comm_type(comm, INTRACOMM))        goto mpi_exception;
  if (mpi_errno = check_group(commGetLocalGroup(comm)))    goto mpi_exception;
  if (mpi_errno = check_count(sendcount))                  goto mpi_exception;
  if (mpi_errno = check_datatype(sendtype))                goto mpi_exception;
  if (mpi_errno = check_dtype_commit(sendtype))            goto mpi_exception;
#endif

	NEST_FXN_INCR();

	rank  = commGetRank(comm);

	if (rank == root) {
		gsize = commGetSize(comm);

#ifdef CHECK_MODE
		if (mpi_errno = check_root(root, comm))                goto mpi_exception_unnest;
    for (i = 0; i < gsize; i++) {
      if (mpi_errno = check_count(recvcounts[i]))          goto mpi_exception_unnest;
    }
    if (mpi_errno = check_datatype(recvtype))              goto mpi_exception_unnest;
    if (mpi_errno = check_dtype_commit(recvtype))          goto mpi_exception_unnest;
#endif

		dtrecvsz = dtypeGetExtent(recvtype);
		for (i = 0; i < gsize; i++) {
			if (i == root) { /* root copy the buffer without sending it */

				MPI_Irecv((char *) recvbuf + displs[i] * dtrecvsz,
										recvcounts[i], recvtype, i, GATHER_TAG, comm, &req);

			} else { /* root receive from the other processes */

				MPI_Recv((char *) recvbuf + displs[i] * dtrecvsz,
									   recvcounts[i], recvtype, i, GATHER_TAG, comm, NULL);

			}
		}
	}

	/* Send the data to root, itself included */
	MPI_Send(sendbuf, sendcount, sendtype, root, GATHER_TAG, comm);

	if (rank == root) { /* Root must deallocate the request */
		MPI_Wait(&req, NULL);
	}

	NEST_FXN_DECR();

#ifdef DEBUG_MODE
	fprintf(stdout, "MPI_Gatherv (end)  \tProcess: 0x%x\n", PCS_self());
#endif

	return MPI_SUCCESS;

	mpi_exception_unnest:
	NEST_FXN_DECR();

	mpi_exception:
	return commHandleError (comm, mpi_errno, "MPI_Gatherv");
}


int MPI_Allgatherv (void *sendbuf, int sendcount, MPI_Datatype sendtype,
					void *recvbuf, int *recvcounts, int *displs, MPI_Datatype recvtype,
					MPI_Comm comm)   {
	int   mpi_errno;
	int   root;
	int   gsize;

#ifdef DEBUG_MODE
	fprintf(stdout, "MPI_Allgatherv (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
	if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_comm_type(comm, INTRACOMM))        goto mpi_exception;
  if (mpi_errno = check_group(commGetLocalGroup(comm)))    goto mpi_exception;
  if (mpi_errno = check_count(sendcount))                  goto mpi_exception;
  if (mpi_errno = check_datatype(sendtype))                goto mpi_exception;
  if (mpi_errno = check_datatype(recvtype))                goto mpi_exception;
  if (mpi_errno = check_dtype_commit(sendtype))            goto mpi_exception;
  if (mpi_errno = check_dtype_commit(recvtype))            goto mpi_exception;
#endif

	NEST_FXN_INCR();

	int rank  = commGetRank(comm);

	gsize = commGetSize(comm);

	for (root = 0; root < gsize; root++) {

		MPI_Gatherv(sendbuf, sendcount, sendtype,
								  recvbuf, recvcounts, displs, recvtype, root, comm);

	}

	NEST_FXN_DECR();

#ifdef DEBUG_MODE
	fprintf(stdout, "MPI_Allgatherv (end)  \tProcess: 0x%x\n", PCS_self());
#endif

	return MPI_SUCCESS;

	mpi_exception_unnest:
	NEST_FXN_DECR();

	mpi_exception:
	return commHandleError (comm, mpi_errno, "MPI_Allgatherv");
}

int MPI_Scatterv (void *sendbuf, int *sendcounts, int *displs, MPI_Datatype sendtype,
				  void *recvbuf, int recvcount, MPI_Datatype recvtype,
				  int root, MPI_Comm comm )  {
	int          mpi_errno;
	int          i;
	int          rank;
	int          gsize;
	int          dtsendsz;
	MPI_Request  req;

#ifdef DEBUG_MODE
	fprintf(stdout, "MPI_Scatterv (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
	if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_comm_type(comm, INTRACOMM))        goto mpi_exception;
  if (mpi_errno = check_group(commGetLocalGroup(comm)))    goto mpi_exception;
  if (mpi_errno = check_count(recvcount))                  goto mpi_exception;
  if (mpi_errno = check_datatype(recvtype))                goto mpi_exception;
  if (mpi_errno = check_dtype_commit(recvtype))            goto mpi_exception;
#endif

	NEST_FXN_INCR();

	rank  = commGetRank(comm);

	if (rank == root) {
		gsize = commGetSize(comm);

#ifdef CHECK_MODE
		if (mpi_errno = check_datatype(sendtype))              goto mpi_exception_unnest;
    if (mpi_errno = check_dtype_commit(sendtype))          goto mpi_exception_unnest;
    if (mpi_errno = check_root(root, comm))                goto mpi_exception_unnest;
    for (i = 0; i < gsize; i++) {
      if (mpi_errno = check_count(sendcounts[i]))          goto mpi_exception_unnest;
    }
#endif

		dtsendsz = dtypeGetExtent(sendtype);

		for (i = 0; i < gsize; i++) {

			if (i == root) { /* root copy the buffer without sending it */

				MPI_Isend((char *)sendbuf + (displs[i] * dtsendsz), sendcounts[i], sendtype,
										i, SCATTER_TAG, comm, &req);

			} else { /* root send to the other processes */

				MPI_Send( (char *)sendbuf + (displs[i] * dtsendsz), sendcounts[i], sendtype,
										i, SCATTER_TAG, comm);

			}
		}
	}

	/* Receive the data from the root, itself included */
	MPI_Recv(recvbuf, recvcount, recvtype, root, SCATTER_TAG, comm, NULL);

	if (rank == root) { /* Root need to deallocate the request */
		MPI_Wait(&req, NULL);
	}

	NEST_FXN_DECR();

#ifdef DEBUG_MODE
	fprintf(stdout, "MPI_Scatterv (end)  \tProcess: 0x%x\n", PCS_self());
#endif

	return MPI_SUCCESS;

	mpi_exception_unnest:
	NEST_FXN_DECR();

	mpi_exception:
	return commHandleError (comm, mpi_errno, "MPI_Scatterv");
}

int __smp_scatter_reference (void *sendbuf, int sendcount, MPI_Datatype sendtype,
								  void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm )
{
	void        *rootSendBuff;
	void        *sendAddr, *recvAddr;
	const int    me     = commGetRank(comm);
	const int    size   = commGetSize(comm);
	const int    dtsendsz = dtypeGetExtent(sendtype);
	const int    nBytes   = sendcount * dtsendsz;
	int          i;

	if (me == root) {
		rootSendBuff = sendbuf;
	}

	MPI_Bcast(&rootSendBuff , sizeof(void *), MPI_BYTE, root, comm);

	recvAddr = recvbuf;
	sendAddr = rootSendBuff + (me * nBytes);
	memcpy(recvAddr, sendAddr, nBytes );

	if (me == root) {
		for (i = 0; i < size; i++) {
			if(i == me) continue;
			MPI_Recv (NULL, 0, MPI_INT, i, REDUCE_TAG, comm, MPI_STATUS_IGNORE);
		}
	}
	else
		MPI_Send (NULL, 0, MPI_INT, root, REDUCE_TAG, comm);

	return MPI_SUCCESS;
}

int __smp_scatter(void *sendbuf, int sendcount, MPI_Datatype sendtype,
					   void *recvbuf, int recvcount, MPI_Datatype recvtype,  int root, MPI_Comm comm )
{
#ifdef FBOX_BUF_MAX
	int nBytes = sendcount * dtypeGetExtent(sendtype);
  if(nBytes <= FBOX_BUF_MAX/2)
    return(smp_scatter_value      (sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm));
  else
#endif
	return(__smp_scatter_reference  (sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm));
}

/*
 *  net_Scatter
 */
int __net_scatter (void *sendbuf, int sendcount, MPI_Datatype sendtype,
				 void *recvbuf, int recvcount, MPI_Datatype recvtype,
				 int root, MPI_Comm comm )
{
	int          mpi_errno;
	int          i;
	int          rank;
	int          gsize;
	int          dtsendsz;
	MPI_Request  req;

#ifdef DEBUG_MODE
	fprintf(stdout, "MPI_Scatter (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
	if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_comm_type(comm, INTRACOMM))        goto mpi_exception;
  if (mpi_errno = check_group(commGetLocalGroup(comm)))    goto mpi_exception;
  if (mpi_errno = check_count(recvcount))                  goto mpi_exception;
  if (mpi_errno = check_datatype(recvtype))                goto mpi_exception;
  if (mpi_errno = check_dtype_commit(recvtype))            goto mpi_exception;
#endif

	NEST_FXN_INCR();

	rank  = commGetRank(comm);

	if (rank == root) {
		gsize = commGetSize(comm);

#ifdef CHECK_MODE
		if (mpi_errno = check_count(sendcount))                goto mpi_exception_unnest;
    if (mpi_errno = check_datatype(sendtype))              goto mpi_exception_unnest;
    if (mpi_errno = check_dtype_commit(sendtype))          goto mpi_exception_unnest;
    if (mpi_errno = check_root(root, comm))                goto mpi_exception_unnest;
#endif

		dtsendsz = dtypeGetExtent(sendtype);

		for (i = 0; i < gsize; i++) {
			if (i == root) { /* root copy the buffer without sending it */

				MPI_Isend((char *)sendbuf + (i * sendcount * dtsendsz), sendcount, sendtype,
										i, SCATTER_TAG, comm, &req);

			} else { /* root send to the other processes */

				MPI_Send((char *)sendbuf + (i * sendcount * dtsendsz), sendcount, sendtype, i, SCATTER_TAG, comm);

			}
		}
	}
	/* Receive the data from the root, itself included */
	MPI_Recv(recvbuf, recvcount, recvtype, root, SCATTER_TAG, comm, NULL);

	if (rank == root) { /* Root must deallocate the request */
		MPI_Wait(&req, NULL);
	}

	NEST_FXN_DECR();

#ifdef DEBUG_MODE
	fprintf(stdout, "MPI_Scatter (end)  \tProcess: 0x%x\n", PCS_self());
#endif

	return MPI_SUCCESS;

	mpi_exception_unnest:
	NEST_FXN_DECR();

	mpi_exception:
	return commHandleError (comm, mpi_errno, "MPI_Scatter");
}


int MPI_Scatter(void *sendbuf, int sendcount, MPI_Datatype sendtype,
				void *recvbuf, int recvcount, MPI_Datatype recvtype,
				int root, MPI_Comm comm )
{

	int          mpi_errno;
	int          rank;
	int          size;
	char        *tmpbuf  = NULL;
	int          dtsizebytes;
	int          i;
	MPI_Request  req;


#ifdef DEBUG_MODE
	fprintf(stdout, "MPI_Scatter(start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
	if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_comm_type(comm, INTRACOMM))        goto mpi_exception;
  if (mpi_errno = check_dest_comm(root, comm))             goto mpi_exception;
  if (mpi_errno = check_count(sendcount))                      goto mpi_exception;
  if (mpi_errno = check_count(recvcount))                      goto mpi_exception;
  if (mpi_errno = check_datatype(sendtype))                goto mpi_exception;
  if (mpi_errno = check_datatype(recvtype))                goto mpi_exception;
  //if (mpi_errno = check_count(count))                      goto mpi_exception;
  //if (mpi_errno = check_datatype(datatype))                goto mpi_exception;
  //if (mpi_errno = check_dtype_commit(datatype))            goto mpi_exception;
  //if (mpi_errno = check_ops(op))                           goto mpi_exception;
#endif
	if(sendcount == 0)
		return MPI_SUCCESS;


	//mpi_errno = __smp_scatter(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm);

	mpi_errno = __net_scatter(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm);

	if (0 > mpi_errno)                                       goto mpi_exception;

#ifdef DEBUG_MODE
	fprintf(stdout, "MPI_Scatter(end)  \tProcess: 0x%x\n", PCS_self());
#endif

	return MPI_SUCCESS;

	mpi_exception_unnest:
	NEST_FXN_DECR();

	mpi_exception:
	if (tmpbuf) free(tmpbuf);
	return commHandleError (comm, mpi_errno, "MPI_Reduce");
}

/*
 *  MPI_Unpack
 *    Unpacking data of a received message
 */
int MPI_Unpack (void *inbuf,  int insize,   int *position,
								void *outbuf, int outcount, MPI_Datatype datatype, MPI_Comm comm) {

	int  mpi_errno;
	int  realsize;

#ifdef DEBUG_MODE
	fprintf(stdout, "MPI_Unpack (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
	if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_datatype(datatype))                goto mpi_exception;
  if (mpi_errno = check_dtype_commit(datatype))            goto mpi_exception;
#endif

	//realsize = packSize(outcount, datatype);
	//printf("----------------%d---------------###%d----\n", insize, realsize);
	//if (insize > realsize)                                   {mpi_errno = MPI_ERR_BUFFER; goto mpi_exception;}

	CALL_FXN (unpack(inbuf, insize, position, outbuf, outcount, datatype), MPI_ERR_INTERN);

#ifdef DEBUG_MODE
	fprintf(stdout, "MPI_Unpack (end)  \tProcess: 0x%x\n", PCS_self());
#endif

	return MPI_SUCCESS;

	mpi_exception_unnest:
	mpi_exception:
	return commHandleError (comm, mpi_errno, "MPI_Unpack");
}

int MPI_Pack (void *inbuf,  int incount, MPI_Datatype datatype,
							void *outbuf, int outsize, int *position, MPI_Comm comm) {

	int  mpi_errno;
	int  realsize;

#ifdef DEBUG_MODE
	fprintf(stdout, "MPI_Pack (start)\tProcess: 0x%x\n", PCS_self());
#endif

#ifdef CHECK_MODE
	if (mpi_errno = check_comm(comm))                        goto mpi_exception;
  if (mpi_errno = check_datatype(datatype))                goto mpi_exception;
  if (mpi_errno = check_dtype_commit(datatype))            goto mpi_exception;
#endif

	//realsize = packSize(incount, datatype);
	//if (outsize < realsize)                                  {mpi_errno = MPI_ERR_BUFFER; goto mpi_exception;}

	CALL_FXN (pack(inbuf, incount, datatype, outbuf, outsize, position), MPI_ERR_INTERN);

#ifdef DEBUG_MODE
	fprintf(stdout, "MPI_Pack (end)  \tProcess: 0x%x\n", PCS_self());
#endif

	return MPI_SUCCESS;

	mpi_exception_unnest:
	mpi_exception:
	return commHandleError (comm, mpi_errno, "MPI_Pack");
}

int MPI_Type_hvector (int count, int blocklength, Mpi_P_Aint stride, MPI_Datatype oldtype, MPI_Datatype *newtype) {

	int  mpi_errno;

#ifdef CHECK_MODE
	if (mpi_errno = check_datatype(oldtype))                 goto mpi_exception;
#endif

	CALL_FXN(PCS_dtypeCreate( count,                               /* Size of arrays    */
														&blocklength,                        /* Block lengths     */
														&stride,                             /* Displacements     */
														(Mpi_P_Datatype_t *)&oldtype,        /* Types             */
														DTYPE_HVECTOR,                       /* Kind of MPI type  */
														newtype),                            /* New type created  */
					 MPI_ERR_TYPE);

	return MPI_SUCCESS;

	mpi_exception_unnest:
	mpi_exception:
	return commHandleError (MPI_COMM_WORLD, mpi_errno, "MPI_Type_hvector");
}

//FIXME:As we hook MPI_Waitany, we do not get the function address of
// PMPI_Waitany. So we use this ugly method to keep PMPI_Waitany in the
// symbol table. Any better method?
int nonuse(void *buf, int count, MPI_Datatype datatype, int dest, int tag,
		MPI_Comm comm) {
	int rank = commGetRank(comm);
	MPI_Request *rqst;
	int done;
	RQST_Status *status;
//int count, MPI_Request *rqst, int *index, RQST_Status *status
	int res =PMPI_Waitany(count, rqst, &done, &status);
	res +=PMPI_Ssend(buf, count, datatype, dest, tag, comm);
	return res;
}

