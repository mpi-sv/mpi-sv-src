#ifndef MPIKLEE_COMMLOG_H
#define MPIKLEE_COMMLOG_H

#include <iostream>
//#include <map>
#include <vector>
#include <set>
#include <list>
#include "llvm/Instructions.h"
#include "klee/Internal/Module/InstructionInfoTable.h"
#include <klee/Expr.h>
//#include <klee/ExecutionState.h>
using namespace std;
using namespace klee;
using namespace llvm;

namespace llvm {
  class Instruction;
}
namespace klee {
  class CommLogManager;
}

//AZQ SPECIFIC
#define ANY_SOURCE  0x7FFFFFFF
#define ANY_TAG	   0x00008000

typedef enum {
	NOP, COMM_SSEND = 1, COMM_SEND, COMM_RECV, COMM_BARR, COMM_ISEND, COMM_IRECV, COMM_WAIT, // do not change COMM_WAIT=7, coz azqmpi use  klee_mpi_nonblock(.., 7, to indicate wait.
	MPI_FINDATATRANS = 8, // memcpy finished by MPI lib in ISend or IRecv, DO NOT CHANGE!!! FIX IT TO BE 8.
	COMM_TEST,
	COMM_FIN,
	COMM_INIT,
	COMM_LEAK
} CommType; // COMM_FIN is the finalize operation.
static std::string array[13] = { "NOP", "PMPI_Ssend", "PMPI_Send", "PMPI_Recv", "PMPI_Barrier",
		"PMPI_Isend", "PMPI_Irecv", "PMPI_Wait", "NOP_MARKER", "PMPI_Test", "PMPI_Finalize",
		"PMPI_Init", "NOP_LEAK", };
enum BufferState {
	INIT, READY, FIN, OTHER
};

/*YU
 * used to represent the matching between send and recv
 * matched: precisely matching(no wildcard)
 * partially_matched: wildcard receive matches send
 * */
enum MatchType {
	UNMATCHED = 0, MATCHED = 1, PATIALLY_MATCHED = 2
};
enum IssueType {
	UNISSUED = 0, ISSUED = 1, UNKNOWN = 2
};

typedef void * buf_type;

/*YU: used to check the buffer property violation
 * ownerinfo is the info of the MPI call instruction */
typedef struct bufferInfo {
	void * req;
	long size;
	BufferState state;
	CommType commtype; // Note that only COMM_ISEND,COMM_IRECV are expected.
	const klee::InstructionInfo * ownerinfo; // Keeps the infomation(i.e. file, line) of the SEND/RECV call
} buf_info_t;

typedef long mpi_rank_t;
typedef unsigned int func_id_t;
typedef std::pair<int, int> task_ty; //used in MSPattern to record the matched send of the recv in slave, the key is the rank, value is the index

typedef struct str_arg {
	CommType func;

	// serve as function args, just a copy of the MPI operation args.
	void *buffer;
	int count;
	int source;
	int dest;
	int tag;
	int root;
	void * comm;
	void * request;
	void * status;
	void* type;
} arg_t;

/*
 * rank: process id
 * index: position of the event sequence
 * mpi_op: indicator of mpi operation
 * set_match_candidate: candidates set of wildcard receives
 * isSend/isRecv: whether op is send/recv
 * isBlock: whether op is blocking operation
 * iswildcard: whether recv any source or any tag.
 * happensBefore: temporal relationships between 2 operations of a given process
 * arguments: args of the MPI operation,  0: initial address, 1: count, ..., 3: src/dest,  4:tag,
 * 5: root, 6: comm, 7:req, 8:status, 9:type
 * */
class CommLogItem {
	//friend class SpecialFunctionHandler;
	//friend class CommLogManager;
public:
	mpi_rank_t rank;
	int index;
	bool LabelOfIrecv;
	/*NB,NA : since we my add Isends from a process to candidates of a Irecv(*)/Recv(*), we want to ensure the order of reading*/
	int NB; //added by yhb,   the index of the Isend that must read before it. default -1.
	bool NA; //added by yhb, true if some Isend behind need to add to candidates set, false default.

	/*the following fileds are used in Master-Slave Pattern, suppose we have tasks, and n slaves*/
	int recvOfMaster; //pattern: recv(*)-send,from 1 to m-n; for last n, the label is 0; otherwise is -1.
	bool recvFM; // pattern: recv(*)recv(*.source)send(*.source). a label for the second recv.
	int sendOfMaster; //denotes the ith task from 1, otherwise -1.
	bool sendOfSlave; // initially false. label for the special send in slave
	bool sendFS; // pattern: while(1){recv(M)send(M)send(M)}. a label for the second send.
	bool recvOfSlave; // initially false. label for the special recv in slave
	bool filtered; // initially false. only used for send and recv in slave.
	bool notifySend; // initially false. only used for send in master that notify a slave to terminate.
	int slaves; // the number of slaves for the Master-Slave pattern, otherwise -1.
	int schedules; // the number of repetitive pattern : recv(*) send(*.source)  in Master-Slave. otherwise -1;
	string file; //initially empty, the file location of an MPI operation.
	int lineNumber; //initially -1, the line number of an MPI operation.
	std::set<task_ty> tasks; //only used for recv in slave to store the matched send in master! include sends in master, so needs to filter!
/*----------------end ---------------------*/

	typedef std::set<CommLogItem *> matchcandi_ty;
	typedef std::set<CommLogItem *> ancestor_ty;
	CommType mpi_op;
	//MatchType matched;
	//IssueType issued;
	matchcandi_ty set_match_candidate;
	ancestor_ty ancestors;
	std::vector<ref<klee::Expr> > arguments;
	std::string funcname;
	bool wildcard_rewrite;
protected:
	//const llvm::Instruction * callins;

public:
	CommLogItem();
	CommLogItem(CommType op, MatchType matchedtype, std::string name);
//CommLogItem();
	~CommLogItem();
	void print(std::ostream & os);
	bool filter(CommLogItem* sendSlave); // whether the current send should be filtered according the static location of sendSlave.
	MatchType opMatched(CommLogItem *peerItem);
	std::string getFuncname() {
		assert(mpi_op > 0 && mpi_op <= 11);
		return array[mpi_op];
	}
	;
	bool isBlock();
	bool isSend();
	bool isRecv();
	bool isWildcard(bool tagOrSrc);
	bool happensBefore(CommLogItem &s);
	bool allAncestorMatched(std::vector<CommLogItem *> & proclog, int pos);
};

inline std::ostream &operator<<(std::ostream &os, CommLogItem &item) {
	if (item.isSend() || item.isRecv()) {
		int i = dyn_cast < ConstantExpr > (item.arguments[3])->getSExtValue() == ANY_SOURCE ?
				-1 : dyn_cast < ConstantExpr > (item.arguments[3])->getZExtValue();//getSExtValue();
		os << "[" << item.rank << "," << item.index << "] :" << item.getFuncname() << "("
				<< i<<")";
	} else
		os << "[" << item.rank << "," << item.index << "] :" << item.getFuncname();
	return os;
}

inline bool operator<(CommLogItem left, CommLogItem right) {
	return (left.rank== right.rank && left.index < right.index);
}
#endif
