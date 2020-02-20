#ifndef COMMLOG_MANAGER_H
#define COMMLOG_MANAGER_H

#include "mpise/commlog.h"
#include "klee/Threading.h"
#include "mpise/analysis.h"
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/CallingConv.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Assembly/PrintModulePass.h"
#include "llvm/Support/IRBuilder.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Assembly/Writer.h"

using namespace llvm;

#include <vector>
#include <set>
#include <list>
#include <algorithm>
using namespace llvm;
namespace klee {
typedef uint64_t wlist_id_t;
typedef std::map<wlist_id_t, std::set<thread_uid_t> > wlists_ty;
typedef std::map<buf_type, bufferInfo> mpibuffermap_ty;
typedef std::map<thread_uid_t, long> checkcount_ty;
typedef std::map<mpi_rank_t, std::vector<CommLogItem *> > mpicommlog_ty;
typedef std::map<CommLogItem *, MatchType> mpicommlog_mcflag_ty;
typedef std::map<mpi_rank_t, thread_uid_t> rank2tuid_ty;

class CommLogManager {
	friend class SpecialFunctionHandler;
	friend class Executor;
public:

	mpibuffermap_ty mpibuffermap;
	mpicommlog_ty mpicommlog;
	mpicommlog_mcflag_ty logmatchflag;
	wlist_id_t wlstids[128];
	unsigned long mpinodes;
	thread_id_t next_thread_candidate;
	bool sync_chk_enable[128]; //Bad manner. MAX_THREADS in config.h
	bool rank_blocked[128];
	bool rank_finalized[128];
	checkcount_ty chkmap;
	rank2tuid_ty rank2thread;
	std::vector<std::list<CommLogItem *> > matchpairs;
	std::map<std::pair<unsigned long , KFunction *>, unsigned int> dirtycalls;

public:
	CommLogManager();
	bool allNodesBlocked();
	bool allMatched(ExecutionState &state,std::vector<CommLogItem *> &v);
	bool allNodesNotEnabled(ExecutionState &state);
	bool allNodesFinished();
	bool coenabledHB(CommLogItem & , CommLogItem &);
	unsigned getFinishedCnt();
	bool check(ExecutionState &state,int subdepth);
	bool getAncestor(std::vector<CommLogItem *> & proclog, int pos);
	void getAllAncestors(ExecutionState &state,std::map<mpi_rank_t, std::vector<CommLogItem *> > &m);
	/*
	 * YU:
	 * an operation enable means all its ancestors which have happens-before relationship  that have matched.
	 * */
	bool getAllEnabled(ExecutionState &state,std::map<mpi_rank_t, std::vector<CommLogItem *> > &m);
	bool getMatchedPairs(ExecutionState &state,std::map<mpi_rank_t, std::vector<CommLogItem *> > &enabled,
			bool &wildcardpairs,bool &hasnormalpair);
	bool getCollectivePairs(std::map<mpi_rank_t, std::vector<CommLogItem *> > &enabled,
			CommType collectiveComm);
	bool getWaitPairs(std::map<mpi_rank_t, std::vector<CommLogItem *> > &enabled);
	bool getSRPairs(std::map<mpi_rank_t, std::vector<CommLogItem *> > &enabled, bool haswildcard);
	void preparFireSRPair(ExecutionState & state, std::list<CommLogItem *> &srset);
	int  preparFireMatchedPairs(ExecutionState &state, bool wild,int subdepth,bool &wildreached);
	/*
	 * YU:
	 * whether the ancestors of op in index pos, process proclog all matched!
	 * ancestor means the happens-before relation
	 * */
	bool allAncestorMatched(ExecutionState &state,std::vector<CommLogItem *> & proclog, int pos);
	//int prepareMatchedPairs(ExecutionState &state);
	void dump(ExecutionState &state,std::ostream &os);
	bool unblockWaitofIsend(ExecutionState &state,std::vector<CommLogItem *> & proclog, int rank);
    std::string  getWildcardMatches();
private:
	unsigned long depth;
	std::map<mpi_rank_t, std::list<std::pair<Function *,CommLogItem *> > > funcstofire;
	std::map<mpi_rank_t, std::vector<std::vector<ref<klee::Expr> > > > argments4funcs;
	std::list<std::pair<Location, Location> > wildcardmatches;

private:
#if 0
	llvm::Function * getFatherFunction(ExecutionState &state,std::string & funcname);
	Function * packFunctions(ExecutionState &state, mpi_rank_t rank, int depth,
			std::list<std::pair<Function *,CommLogItem *> > &functions);
	bool fixArgments(ExecutionState &state, Thread & thread,
			std::list<std::pair<Function *, CommLogItem *> > &functions);
#endif

};
}
#endif /* COMMLOG_MANAGER_H */
