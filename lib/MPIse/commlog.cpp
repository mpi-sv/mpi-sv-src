/*
 * commlog.cpp
 *
 *  Created on: 2013-4-25
 *      Author: Herman
 */

#include <iostream>
#include <assert.h>
#include <glog/logging.h>
#include <boost/foreach.hpp>
#include "klee/klee.h"
#include "mpise/commlog.h"

CommLogItem::CommLogItem(CommType op, MatchType matchedtype, std::string name) :
		rank(ANY_SOURCE), index(ANY_SOURCE), wildcard_rewrite(false),NA(false),NB(-1),lineNumber(-1),file(""),
		sendOfMaster(-1),recvOfMaster(-1),sendOfSlave(false),recvOfSlave(false),slaves(-1),schedules(-1),filtered(false)
,recvFM(false),sendFS(false){
	mpi_op = op;
	funcname = name;
}

CommLogItem::CommLogItem() :
		rank(ANY_SOURCE), index(ANY_SOURCE), mpi_op(NOP), funcname("NOP"), wildcard_rewrite(false),NA(false),NB(-1),lineNumber(-1),file(""),
		sendOfMaster(-1),recvOfMaster(-1),sendOfSlave(false),recvOfSlave(false),slaves(-1),schedules(-1),filtered(false),notifySend(false)
,recvFM(false),sendFS(false){
	ancestors = std::set<CommLogItem *>();
}

CommLogItem::~CommLogItem() {
	if (set_match_candidate.size() == 0)
		return;
	for (std::set<CommLogItem*>::iterator it = set_match_candidate.begin();
			it != set_match_candidate.end(); it++) {
		set_match_candidate.erase(it);
	}
}

void CommLogItem::print(std::ostream & os) {
	assert(os!=NULL && "CommLogItem::print(): NULL os!");
	string op;
	switch (mpi_op) {
	case (COMM_SSEND): {
		op = "ssend";
		break;
	}
	case (COMM_RECV): {
		op = "receive";
		break;
	}
	case (COMM_BARR): {
		op = "barrier";
		break;
	}
	case (COMM_WAIT): {
		op = "wait";
		break;
	}
	case (COMM_ISEND): {
		op = "isend";
		break;
	}
	case (COMM_IRECV): {
		op = "irecv";
		break;
	}
	case (COMM_TEST): {
		op = "test";
		break;
	}
	case (COMM_FIN): {
		op = "finanize";
		break;
	}
	case (COMM_INIT): {
		op = "init";
		break;
	}
	case (COMM_SEND): {
		op = "send";
		break;
	}
	default:
		op = "unknown";
	}
	os << "[" << rank << "," << index << "]" << op;
	/*os << op << "	"<< matched << "	 {";
	 for (std::set<mpi_rank_t>::iterator it = set_match_candidate.begin();
	 it != set_match_candidate.end();) {
	 os << *it;
	 if (++it != set_match_candidate.end())
	 os << ",";
	 }
	 os << "}" << std::endl;*/
}
/*
 MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag,	MPI_Comm comm, MPI_Status *status);
 MPI_Send (void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);
 MPI_Ssend(void *buf, int count, MPI_Datatype datatype, int dest, int tag,MPI_Comm comm);
 MPI_Isend(void *buf, int count, MPI_Datatype datatype, int dest, int tag,MPI_Comm comm, MPI_Request *request);
 MPI_Irecv(void *buf, int count, MPI_Datatype datatype, int source, int tag,	MPI_Comm comm, MPI_Request *request);
 */

//FIXME: we have not consider communicators, and wait operator.
MatchType CommLogItem::opMatched(CommLogItem *peerItem) {
	if (peerItem == NULL)
		return UNMATCHED;
	int dest, src, tag1, tag2;
	int comm1,comm2; // added yhb, because in athena exist: Isend()Isend,they only differ in the communicator!
	//this : rank  send  dest   && peerItem:  rank   recv  src
	if (this->isSend() && peerItem->isRecv()) {
		dest = dyn_cast<ConstantExpr>(arguments[3])->getSExtValue();
		tag1 = dyn_cast<ConstantExpr>(arguments[4])->getSExtValue();

		src = dyn_cast<ConstantExpr>(peerItem->arguments[3])->getSExtValue();
		tag2 = dyn_cast<ConstantExpr>(peerItem->arguments[4])->getSExtValue();

//		comm1= dyn_cast<ConstantExpr>(arguments[5])->getSExtValue(); // added yhb
//		comm2=dyn_cast<ConstantExpr>(peerItem->arguments[5])->getSExtValue(); //added yhb

		if (rank == src && dest == peerItem->rank
				&& (tag1 == tag2 || tag1 == ANY_TAG || tag2 == ANY_TAG)  /*&& comm1==comm2*/ )
			return MATCHED;
		else if (ANY_SOURCE == src && dest == peerItem->rank
				&& (tag1 == tag2 || tag1 == ANY_TAG || tag2 == ANY_TAG)  /*&& comm1==comm2*/)
			return PATIALLY_MATCHED;
	}
	//this : rank  recv   dest   && peerItem : rank   send   src
	if (this->isRecv() && peerItem->isSend()) {
		dest = dyn_cast<ConstantExpr>(arguments[3])->getSExtValue();
		tag1 = dyn_cast<ConstantExpr>(arguments[4])->getSExtValue();

		src = dyn_cast<ConstantExpr>(peerItem->arguments[3])->getSExtValue();
		tag2 = dyn_cast<ConstantExpr>(peerItem->arguments[4])->getSExtValue();

//		if(rank==1&&index==0){
//			LOG(INFO)<<"got it";
//			LOG(INFO)<<peerItem->index;
//			LOG(INFO)<<"comm: "<<arguments[5];
//			LOG(INFO)<<"comm: "<<peerItem->arguments[5];
//		}
//
//		comm1= dyn_cast<ConstantExpr>(arguments[5])->getSExtValue(); // added yhb
//		comm2=dyn_cast<ConstantExpr>(peerItem->arguments[5])->getSExtValue(); //added yhb

		if (rank == src && dest == peerItem->rank && (tag1 == tag2 || tag1 == ANY_TAG || tag2 == ANY_TAG)  /*&&comm1==comm2*/)
			return MATCHED;
		else if (ANY_SOURCE == dest && rank == src && (tag1 == tag2 || tag1 == ANY_TAG || tag2 == ANY_TAG) /*&& comm1==comm2*/)
			return PATIALLY_MATCHED;
	}

	if (this->mpi_op == COMM_BARR && peerItem->mpi_op == COMM_BARR)
		return PATIALLY_MATCHED;

//FIXME! Unhandled opration:wait.
	return UNMATCHED;
}

/*	NOP,
 COMM_SSEND = 1,
 COMM_SEND,
 COMM_RECV,
 COMM_BARR,
 COMM_ISEND,
 COMM_IRECV,
 COMM_WAIT,
 MPI_FINDATATRANS=8,
 COMM_TEST,
 COMM_FIN,
 COMM_INIT,
 COMM_LEAK
 */
bool CommLogItem::isBlock() {
	return (mpi_op == COMM_SSEND || mpi_op == COMM_RECV || mpi_op == COMM_BARR
			|| mpi_op == COMM_WAIT || mpi_op == COMM_FIN);
}

bool CommLogItem::isSend() {
	return (mpi_op == COMM_SEND || mpi_op == COMM_ISEND || mpi_op == COMM_SSEND);
}

bool CommLogItem::isRecv() {
	return (mpi_op == COMM_RECV || mpi_op == COMM_IRECV);
}
// tagOrSrc: 0, test if it's wildcard tag recv, 1 wildcard src recv.
// NOTE THAT  ANY_SOURCE is -1 ---as uint64

// IMPORTANT:: wildcard_rewrite is ture means that arguments[3] is ANY_SOURCE ,but we dynamicly rewrite it to be some
// special value in some state (set it as state A). Then when we are in state B, we ignore arguments[3]. We still think
// that it's ANY_SOURCE.

bool CommLogItem::isWildcard(bool tagOrSrc) {
	if (mpi_op != COMM_RECV && mpi_op != COMM_IRECV)
		return false;
	//LOG(INFO)<<dyn_cast < ConstantExpr > (arguments[3])->getSExtValue();
	if (tagOrSrc) {
		return dyn_cast<ConstantExpr>(arguments[3])->getSExtValue() == ANY_SOURCE || wildcard_rewrite;

	} else {
		return dyn_cast<ConstantExpr>(arguments[4])->getSExtValue() == ANY_TAG;
	}
	return false;
}
//NOTE:This method should only be applied with two commlogitem which lie in the same process.
bool CommLogItem::happensBefore(CommLogItem &s) {

	if (rank != s.rank || index >= s.index)
		return false;

//1. Block:  l -->   s if l is block operation.
	if (isBlock())
		return true;
	if (this == &s)
		return false;

//2. Send:  l -->  s if s,l both sends and targets at the same proc.
// argments[3]:dest , argments[4]:tag ,argments[5]:comm
	if (isSend() && s.isSend()){
		//int src1 = dyn_cast<ConstantExpr>(arguments[3])->getZExtValue();
		//int src2 = dyn_cast<ConstantExpr>(s.arguments[3])->getZExtValue();
		//int tag1 = dyn_cast<ConstantExpr>(arguments[4])->getZExtValue();
		//int tag2 = dyn_cast<ConstantExpr>(s.arguments[4])->getZExtValue();
		//int comm=dyn_cast<ConstantExpr>(arguments[5])->getZExtValue();
		//int comm2=dyn_cast<ConstantExpr>(s.arguments[5])->getZExtValue();
		//LOG(INFO)<<"src1: "<<src1 << "src2: "<<src2<<"tag1: "<<tag1<<"tag2: "<<tag2 <<"comm1: "<<comm<<"comm2: "<<comm2;

//		if(s.index==233 && index==232 && rank==0 ){
//			LOG(INFO)<<"src1: "<<src1 << "src2: "<<src2<<"tag1: "<<tag1<<"tag2: "<<tag2 <<"comm1: "<<comm<<"comm2: "<<comm2;
//		}

			if(dyn_cast<ConstantExpr>(arguments[3])->getZExtValue()
					== dyn_cast<ConstantExpr>(s.arguments[3])->getZExtValue()
			&& dyn_cast<ConstantExpr>(arguments[4])->getSExtValue()
					== dyn_cast<ConstantExpr>(s.arguments[4])->getSExtValue()
			&& dyn_cast<ConstantExpr>(arguments[5])->getZExtValue()
					== dyn_cast<ConstantExpr>(s.arguments[5])->getZExtValue())
		return true;
	}
//3. Recv: l --> s if s,l both recvs and expects msg from the same proc.
// argments[3]:src , argments[4]:tag ,argments[5]:comm

	if (isRecv() && s.isRecv()
			&& (dyn_cast<ConstantExpr>(arguments[3])->getSExtValue()
					== dyn_cast<ConstantExpr>(s.arguments[3])->getSExtValue()
					|| dyn_cast<ConstantExpr>(arguments[3])->getSExtValue() == ANY_SOURCE
                    ||wildcard_rewrite) //added by yhb to fix the bug of wildcard rewriting which can violate the happens-before relationship
			&& (dyn_cast<ConstantExpr>(arguments[4])->getSExtValue()
					== dyn_cast<ConstantExpr>(s.arguments[4])->getSExtValue()
					|| dyn_cast<ConstantExpr>(arguments[4])->getZExtValue() == ANY_TAG)
			&& dyn_cast<ConstantExpr>(arguments[5])->getZExtValue()
					== dyn_cast<ConstantExpr>(s.arguments[5])->getZExtValue())
		return true;

//4. Wait,Test: l--> s if l is a non-block send/recv and s is it's corresponding wait/test.
	if (mpi_op == COMM_IRECV && (s.mpi_op == COMM_WAIT || s.mpi_op == COMM_TEST)) {
		assert(arguments.size() == 7 && "Wrong argment sizes of ");

		bool res = dyn_cast<ConstantExpr>(arguments[6])->getZExtValue()
				== dyn_cast<ConstantExpr>(s.arguments[0])->getZExtValue();
		return res;
	}

	if (mpi_op == COMM_ISEND && (s.mpi_op == COMM_WAIT || s.mpi_op == COMM_TEST)) {
		assert(arguments.size() == 7 && "Wrong argment sizes of ");
		//LOG(INFO)<<"send req: "<<arguments[6] << " wait req:"<<s.arguments[0];
		bool res = dyn_cast<ConstantExpr>(arguments[6])->getZExtValue()
				== dyn_cast<ConstantExpr>(s.arguments[0])->getZExtValue();
		return res;
	}
	return false;
}

/*
 * we need to filter the following send that have same static location with the sendSlave.
 * */
bool CommLogItem::filter(CommLogItem* sendSlave){
  if (lineNumber == sendSlave->lineNumber && file == sendSlave->file)
		return true;
	else
		return false;
}

