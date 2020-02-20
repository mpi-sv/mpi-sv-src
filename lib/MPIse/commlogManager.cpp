
#include "klee/Executor.h"
#include "mpise/commlogManager.h"
#include "cloud9/worker/WorkerCommon.h"
#include "klee/Internal/Module/InstructionInfoTable.h"

#include "llvm/Module.h"
#include "llvm/Support/CommandLine.h"

#include <glog/logging.h>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <string>
#include "klee/Statistics.h"
#include "klee/Executor.h"

using namespace llvm;
using namespace klee;
#define COMMLOG_DEBUG
typedef const std::map<mpi_rank_t, std::vector<CommLogItem *> >::value_type pair_type;
extern cl::opt<bool> SyncOpt;
static bool rankopFired[128] = { false };
static unsigned int finCnt = 0;

CommLogManager::CommLogManager() :
		mpinodes(0), next_thread_candidate(0), depth(0){
	if (WithLibMPI) {
		bool chk = SyncOpt ? false : true;
		memset(sync_chk_enable, chk, sizeof(sync_chk_enable));
		memset(rank_blocked, 0, sizeof(rank_blocked));
		memset(rank_finalized, 0, sizeof(rank_finalized));
		memset(wlstids, 0, sizeof(wlist_id_t) * 128);

	}
}

bool CommLogManager::allNodesBlocked() {
	int i, counts;
	assert(mpinodes > 0 && mpinodes <= 128 && "Invalid mpinodes!");

	for (i = 0, counts = 0; i < mpinodes; i++) {
		if (rank_blocked[i])
			counts++;
		if (counts >= mpinodes)
			return true;
	}
	return false;
}

unsigned CommLogManager::getFinishedCnt() {
	unsigned counts;
	int i;
	assert(mpinodes > 0 && mpinodes <= 128 && "Invalid mpinodes!");

	for (i = 0, counts = 0; i < mpinodes; i++) {
		if (rank_finalized[i])
			counts++;
	}
	return counts;
}

bool CommLogManager::allNodesFinished() {
	return getFinishedCnt() == mpinodes;
}

// 1. check proclog[0] ~ proclog[i-1] if proclog[j] --> proclog[i].
// 2. if yes, we say that proclog[j] is proclog[i]'s ancestor, we push proclog[j] into i's ancestor list.
bool CommLogManager::getAncestor(std::vector<CommLogItem *> & proclog, int pos) {
	int i;
	if( (pos == 12) &&(*proclog[pos]).rank==0 ){
		LOG(INFO)<<"got it"<<pos;
	}
	if (pos <= 0)
		return 0;
	for (i = pos - 1; i >= 0; i--) {
		if (proclog[i]->happensBefore(*proclog[pos])) {
			proclog[pos]->ancestors.insert(proclog[i]);
			//LOG(INFO)<<*proclog[i]<<" -> "<<*proclog[pos];
			//proclog[pos]->ancestors.merge(proclog[i]->ancestors);
		}
	}

	return 0;
}

void CommLogManager::getAllAncestors(ExecutionState &state,
									 std::map<mpi_rank_t, std::vector<CommLogItem *> > &m) {
#ifdef COMMLOG_DEBUG
	BOOST_FOREACH(pair_type &p, m) {
					LOG(INFO) << "*****now blocked and dump all the issued operations**** of state: " << &state <<
					"  rank " << p.first << "commlog begin**********";
					BOOST_FOREACH(CommLogItem *it, p.second) {
									LOG(INFO) << *it;
								}
					LOG(INFO) << "********************dump end***************************";
				}
#endif
	unsigned long i, j;
	for (i = 0; i < m.size(); i++) {
		for (j = 0; j < m[i].size(); j++)
			getAncestor(m[i], j);
	}
	return;
}

bool CommLogManager::allAncestorMatched(ExecutionState &state, std::vector<CommLogItem *> &proclog,
										int pos) {
	if (pos <= 0 || proclog[pos]->ancestors.size() == 0)
		return true;
	BOOST_FOREACH(CommLogItem *item, proclog[pos]->ancestors) {
					if (state.globalCommLog.logmatchflag[item] != MATCHED)
						return false;
				}
	return true;
}

bool CommLogManager::getAllEnabled(ExecutionState &state,
		std::map<mpi_rank_t, std::vector<CommLogItem *> > &m) {
	if (state.globalCommLog.mpicommlog.empty())
		return false;
	long i, j;
	m.clear();
	for (i = 0; i < state.globalCommLog.mpicommlog.size(); i++) {
		for (j = 0; j < state.globalCommLog.mpicommlog[i].size(); j++) {
			if (state.globalCommLog.logmatchflag[state.globalCommLog.mpicommlog[i][j]] == UNMATCHED
					&& allAncestorMatched(state, state.globalCommLog.mpicommlog[i], j))
//				if(mpicommlog[i][j]->isRecv()){
//					if(mpicommlog[i][j]->set_match_candidate.size()>0)
//					mpicommlog[i][j]->set_match_candidate.clear();
//				}
				m[i].push_back(state.globalCommLog.mpicommlog[i][j]);
		}
	}
	if (!m.empty())
		return true;
	return false;
}

bool CommLogManager::getCollectivePairs(std::map<mpi_rank_t, std::vector<CommLogItem *> > &enabled,
										CommType collectiveComm) {
	unsigned long i, j;
	std::list<CommLogItem *> mpair;
	mpair.clear();
	for (i = 0; i < mpicommlog.size(); i++) {
		for (j = 0; j < mpicommlog[i].size(); j++) {
			if (logmatchflag[mpicommlog[i][j]] == UNMATCHED
				&& mpicommlog[i][j]->mpi_op == collectiveComm) {
				mpair.push_back(mpicommlog[i][j]);
			}
		}
	}
	/*since collective, mpair.size()==mpicommlog.size() */
	if (!mpair.empty() && mpair.size() == mpicommlog.size()) {
		BOOST_FOREACH(CommLogItem *item, mpair) {
						assert(item != NULL);
						logmatchflag[item] = MATCHED;
					}
		matchpairs.push_back(mpair);
		return true;
	}
	mpair.clear();
	return false;
}

// For this situation, we omitted the happens before relation:
//  p0               p1
// r(*or 1,tag=*)   s1(0,tag123)
//                  s2(0,tag234)
// i.e. s1.happensbefore(s2)  returns false, since they ignores the peer recv operation.
// In fact s1 indeed happens before s2 according to MPI standard.
bool CommLogManager::coenabledHB(CommLogItem & recv, CommLogItem & send) {
	if (!recv.isRecv() || recv.set_match_candidate.empty() || !recv.isWildcard(false)
			|| !send.isSend())
		return false;
	BOOST_FOREACH(CommLogItem *it,recv.set_match_candidate){
	if((*it)<send && dyn_cast<ConstantExpr>(it->arguments[4])->getSExtValue()
			!= dyn_cast<ConstantExpr>(send.arguments[4])->getSExtValue() ) //tag is not equal!
	{
		LOG(INFO)<<"send is:" <<send.rank<<"---"<<send.index;
		LOG(INFO)<<"it is:"<<it->rank<<"----"<<it->index;
		LOG(INFO)<<"it is:"<< dyn_cast<ConstantExpr>(it->arguments[3])->getSExtValue();
		LOG(INFO)<<"send is:"<<dyn_cast<ConstantExpr>(send.arguments[3])->getSExtValue();
		LOG(INFO)<<"size of recv"<<recv.set_match_candidate.size()<<"------------"<<recv.rank<<"----------------"<<recv.index;
	     return true;
	}
}
	return false;
}

bool CommLogManager::getWaitPairs(std::map<mpi_rank_t, std::vector<CommLogItem *> > &enabled) {
	std::list<CommLogItem *> waits;
	BOOST_FOREACH(pair_type&p,enabled){
	BOOST_FOREACH(CommLogItem *it,p.second) {
		if(it->mpi_op == COMM_WAIT) {
			waits.push_back(it);
			logmatchflag[it] = MATCHED;
		}
	}
}
	if (!waits.empty()) {
		matchpairs.push_back(waits);
		return true;
	}
	return false;
}

bool CommLogManager::getSRPairs(std::map<mpi_rank_t, std::vector<CommLogItem *> > &enabled,
		bool haswildcard) {
	unsigned long peerCnt = 0;
	if (!haswildcard) {
		BOOST_FOREACH(pair_type&p,enabled){
		BOOST_FOREACH(CommLogItem *it,p.second) {
			if(logmatchflag[it] == UNMATCHED && it->isRecv() &&!it->isWildcard(true) ) {
			    it->set_match_candidate.clear(); // added by yhb! fix the bug 4.16: for the issued but not matched Recv, they may be affected by other paths
			    //path1: Recv can match [0,5], while in path2,  the target of [0,5] is changed, but the candidates of recv can still contain [0,5] because of path1, because they share the same object for this Recv.
				int i = dyn_cast < ConstantExpr > (it->arguments[3])->getSExtValue();
				assert(i>=0 && i<128);
				/*YU:since want to receive a message from process i, then we explore the enabled event list of rank i */
				BOOST_FOREACH(CommLogItem *item,enabled[i]) {
					//if(it->opMatched(item) == MATCHED) {
					if(it->opMatched(item) != UNMATCHED) {
						std::list<CommLogItem *> cset;
						cset.push_back(item);
						cset.push_back(it);
						bool flag=true; //we want to avoid to add same one
						if(!coenabledHB(*it,*item)) {
							BOOST_FOREACH(CommLogItem *temp,it->set_match_candidate) {
								if( temp->rank==item->rank && temp->index==item->index ) {
									flag=false;
									break;
								}
							}
							if(flag) {
								it->set_match_candidate.insert(item);
							}
							logmatchflag[item]= logmatchflag[it] = MATCHED;
							matchpairs.push_back(cset);
						}
						else
						LOG(INFO)<<"find co-enabled, operation "<<*item<<" do not match temporarily.";

//#ifdef COMMLOG_DEBUG
#if 0
						LOG(INFO)<<*item;
						LOG(INFO)<<*it;
						LOG(INFO)<<"get SR pair:";
						BOOST_FOREACH(CommLogItem *comlog, cset) {
							LOG(INFO)<<*comlog;
						}
#endif

					}
				}
			}
		}
	}

}
else {
	BOOST_FOREACH(pair_type&p,enabled) {
		BOOST_FOREACH(CommLogItem *it,p.second) {
			if (logmatchflag[it]== UNMATCHED && it->isRecv() &&it->isWildcard(true)) {
				haswildcard = true;
				it->set_match_candidate.clear();  // added by yhb! fix the bug 4.16: for the issued but not matched Recv, they may be affected by other paths
				BOOST_FOREACH(pair_type&pprime,enabled) {
					if(pprime.first == p.first) continue;
					BOOST_FOREACH(CommLogItem *itprime,pprime.second) {
						if(logmatchflag[itprime] == UNMATCHED && itprime->isSend() &&
								(itprime->opMatched(it) == PATIALLY_MATCHED||
										(it->wildcard_rewrite && dyn_cast<ConstantExpr>(itprime->arguments[3])->getSExtValue()== it->rank
										&& (  dyn_cast<ConstantExpr>(it->arguments[4])->getSExtValue()==dyn_cast<ConstantExpr>(itprime->arguments[4])->getSExtValue() ||
												dyn_cast<ConstantExpr>(it->arguments[4])->getSExtValue()==ANY_TAG || dyn_cast<ConstantExpr>(itprime->arguments[4])->getSExtValue()==ANY_TAG)
										) ) ) {
							std::list<CommLogItem *> cset;
							cset.push_back(itprime);
							cset.push_back(it);
							peerCnt ++;
							if(!coenabledHB(*it,*itprime)) {
								it->set_match_candidate.insert(itprime);
								/*#ifdef COMMLOG_DEBUG
								 LOG(INFO)<<*it<<" get SR* pair:";
								 BOOST_FOREACH(CommLogItem *comlog, cset) {
								 LOG(INFO)<<*comlog;
								 }
								 #endif*/
								matchpairs.push_back(cset);
							}
							else
							LOG(INFO)<<"find co-enabled, operation "<<*itprime<<" do not match temporarily.";
						}
					}
				}
			}
		}
	}

}

//Here we find that not all candidate is found.
//FIXME:: How can we know that a wildcard has all possible sends found?
//	if (haswildcard && peerCnt < mpinodes - finCnt - 1 && peerCnt > 0
//			&& !matchpairs[0].back()->isBlock()) {
//		LOG(INFO)<<"Incomplete matching! only "<<peerCnt<<" matches, incomplete match pairs cleared.";
//		LOG(INFO)<<mpinodes <<"-----"<<finCnt<<"------"<<peerCnt<<"\n";
//		matchpairs.clear();
//	}

#ifdef COMMLOG_DEBUG
	BOOST_FOREACH(std::list<CommLogItem *> & mset, matchpairs){
	LOG(INFO)<<"match set begin:";
	BOOST_FOREACH(CommLogItem * tempItem, mset) {
		// Here we should not mark all candidates as matched-- just mark what we should fire.
		//tempItem->matched= MATCHED;
		LOG(INFO)<<*tempItem;
	}
	LOG(INFO)<<"*************";
}
#endif
	return !matchpairs.empty();
}

bool CommLogManager::getMatchedPairs(ExecutionState &state,
									 std::map<mpi_rank_t, std::vector<CommLogItem *> > &enabled,
									 bool &wildcardpairs, bool &hasnormalpair) {

	state.globalCommLog.matchpairs.clear();
	//Get global finalize.
	if (state.globalCommLog.getCollectivePairs(enabled, COMM_FIN)) {
		hasnormalpair = true;
		return true;
	}
	//Get global barrier.
	if (state.globalCommLog.getCollectivePairs(enabled, COMM_BARR)) {
		hasnormalpair = true;
		return true;
	}
	//Get global SR.
	if (state.globalCommLog.getSRPairs(enabled, false)) {
		hasnormalpair = true;
		return true;
	}
	//Get all wait that issued it's corresponding send/recv
	if (state.globalCommLog.getWaitPairs(enabled)) {
		hasnormalpair = true;
		return true;
	}
	//Get global SR*.
	if (state.globalCommLog.getSRPairs(enabled, true)) {
		wildcardpairs = true;
		return true;
	}
	state.globalCommLog.matchpairs.clear();
	//hasnormalpair=wildcardpairs=false;
	return false;
}
//TODO::We need to treat forks by wildcard receives.
//FIXME:: We need to deal with crooked barrier: what about the process leaving alone by the other 2?
void CommLogManager::preparFireSRPair(ExecutionState & state, std::list<CommLogItem *> &mset) {
	assert(mset.size() == 2);
	//CommLogItem * temp = state.globalCommLog.mpicommlog[0][0];

	CommLogItem * log_send = state.globalCommLog.mpicommlog[mset.front()->rank][mset.front()->index];
	CommLogItem * log_recv = state.globalCommLog.mpicommlog[mset.back()->rank][mset.back()->index];
	Function *func_send = state.executor->kmodule->module->getFunction(log_send->getFuncname());
	Function *func_recv = state.executor->kmodule->module->getFunction(log_recv->getFuncname());
	if (func_send == NULL)
		LOG(ERROR)<<"Could not get send func:"<< log_send->getFuncname();
	if (func_recv == NULL)
		LOG(ERROR)<<"Could not get recv func: "<<log_recv->getFuncname();

	assert(log_send->arguments.size() == 7 && log_recv->arguments.size() == 7 && func_send!=NULL);
	assert(
			log_send->rank >= 0 && log_send->rank <= 128 && log_recv->rank >= 0 && log_recv->rank <= 128 && func_recv!=NULL);
	// PMPI_Ssend may get null functions. In this case, we return PMPI_Ssend in hooked api.
	if (func_send != NULL) {
		std::pair<Function*, CommLogItem*> p;
		p.first = func_send;
		p.second = log_send;
		state.globalCommLog.funcstofire[log_send->rank].push_back(p);
		state.globalCommLog.argments4funcs[log_send->rank].push_back(log_send->arguments);
	}
	//log_send->issued = ISSUED;
	state.globalCommLog.logmatchflag[log_send] = MATCHED;
	if (log_send->isBlock())
		state.globalCommLog.rank_blocked[log_send->rank] = false;
	rankopFired[log_send->rank] = true;

	if (func_recv != NULL) {
		std::pair<Function*, CommLogItem*> p;
		p.first = func_recv;
		p.second = log_recv;
		state.globalCommLog.funcstofire[log_recv->rank].push_back(p);
		state.globalCommLog.argments4funcs[log_recv->rank].push_back(log_recv->arguments);
	}
	//log_recv->issued = ISSUED;
	state.globalCommLog.logmatchflag[log_recv] = MATCHED;
	if (log_recv->isBlock())
		state.globalCommLog.rank_blocked[log_recv->rank] = false;
	rankopFired[log_recv->rank] = true;

#ifdef COMMLOG_DEBUG
	LOG(INFO)<<*(log_send)<<" and "<<*log_recv<<" "<<" fired.";
#endif
	return;
}

int CommLogManager::preparFireMatchedPairs(ExecutionState &state, bool wildcardpair, int sub,
		bool &wildreached) {
	if (state.globalCommLog.matchpairs.empty()) {
		LOG(INFO)<<"fireMatchedPairs got empty matchpairs to fire";
		return -1;
	}
	int i;
	//state.globalCommLog.funcstofire.clear();
	//state.globalCommLog.argments4funcs.clear();
	std::list<CommLogItem *>::iterator it =state.globalCommLog.matchpairs[0].begin();
	if((*it)->mpi_op == COMM_FIN) {
		//assert(allNodesBlocked()&& state.globalCommLog.matchpairs.size()==1);
		for(;it!=state.globalCommLog.matchpairs[0].end();it++) {
			state.globalCommLog.rank_finalized[(*it)->rank]= true;
			//This ensures that all nodes should reach finalize && all operations should be matched.
			if(state.globalCommLog.matchpairs[0].size()== state.globalCommLog.mpinodes &&
					state.globalCommLog.allMatched(state,state.globalCommLog.mpicommlog[(*it)->rank])) {
				state.globalCommLog.rank_blocked[(*it)->rank]= false;
			}
		}
	}

	else if((*it)->mpi_op == COMM_BARR) {
		assert(allNodesBlocked()&& state.globalCommLog.matchpairs.size()==1);
		std::pair<Function*, CommLogItem*> p;
		for(;it!=state.globalCommLog.matchpairs[0].end();it++) {
			Function *f = state.executor->kmodule->module->getFunction("PMPI_Barrier");
			assert( (*it)->arguments.size()==1 );
			assert( (*it)->rank>=0 && (*it)->rank<128);
			rankopFired[(*it)->rank] = true;

			p.first = f;
			p.second = *it;
			state.globalCommLog.funcstofire[(*it)->rank].push_back(p);
			state.globalCommLog.argments4funcs[(*it)->rank].push_back((*it)->arguments);

#ifdef COMMLOG_DEBUG
			LOG(INFO)<<*(*it)<<" "<<(*it)->funcname<<" fired.";
#endif
			//(*it)->issued = ISSUED;
			state.globalCommLog.rank_blocked[(*it)->rank]= false;

		}
	}

	else if((*it)->mpi_op == COMM_WAIT) {
		std::pair<Function*, CommLogItem*> p;
		for(;it!=state.globalCommLog.matchpairs[0].end();it++) {
			Function *f = state.executor->kmodule->module->getFunction("PMPI_Wait");
			assert( (*it)->arguments.size()==2);
			assert( (*it)->rank>=0 && (*it)->rank<128);
			p.first = f;
			p.second = *it;
			state.globalCommLog.funcstofire[(*it)->rank].push_back(p);
			state.globalCommLog.argments4funcs[(*it)->rank].push_back((*it)->arguments);
			//(*it)->issued = ISSUED;
			state.globalCommLog.rank_blocked[(*it)->rank]= false;
			rankopFired[(*it)->rank] = true;
#ifdef COMMLOG_DEBUG
			LOG(INFO)<<*(*it)<<" "<<(*it)->funcname<<" fired.";
#endif

		}
	}
	else if((*it)->isSend() && !wildcardpair) {
		BOOST_FOREACH(std::list<CommLogItem *>& mset,matchpairs) {
			state.globalCommLog.preparFireSRPair(state,mset);
		}
	}

	//deal with mutiple matched pairs brought by wildcards.
	else if((*it)->isSend() && state.globalCommLog.matchpairs.size()>=1 && wildcardpair) {
		ExecutionState *lastState = &state;
		i=0;
		BOOST_FOREACH(std::list<CommLogItem *>& mset,state.globalCommLog.matchpairs) {
			assert(state.globalCommLog.matchpairs.size()>=1 && wildcardpair);
			BOOST_FOREACH(CommLogItem *item,mset) {
				item->print(LOG(INFO));
			}
			ref<klee::Expr> newsrc= ConstantExpr::create(mset.front()->rank,mset.back()->arguments[3].get()->getWidth());
			mset.back()->arguments[3]= newsrc;
			mset.back()->wildcard_rewrite = true;

			pair<Location, Location> p;
			p.first.PID=mset.back()->rank;
			p.first.index=mset.back()->index;
			p.second.PID=mset.front()->rank;
			p.second.index=mset.front()->index;
			LOG(INFO) << lastState ;
			if(state.globalCommLog.matchpairs.size() >0 && i< state.globalCommLog.matchpairs.size()-1) {
				Executor::StatePair sp = state.executor->fork(*lastState, KLEE_FORK_WILD);
				//for debug
				ExecutionState * left=sp.first;
				ExecutionState * right=sp.second;
				left->fileForLastRecvMaster=lastState->fileForLastRecvMaster; // added by yhb, to make sure the location of the first repetitive recv in M-S is recorded!
				right->lineNumberForLastRecvMaster=lastState->lineNumberForLastRecvMaster;
//  			LOG(INFO)<<left<<"-----------forking------"<<(left->globalCommLog.mpicommlog[0][0]);
//				LOG(INFO)<<right<<"----------forking-------"<<(right->globalCommLog.mpicommlog[0][0]);
				//testing!!!!!
				//left->globalCommLog.mpicommlog[0][1]->arguments[3]=1000;
//				LOG(INFO)<<"AAAYHB"<<dyn_cast < ConstantExpr > (left->globalCommLog.mpicommlog[0][0]->arguments[3])->getSExtValue();
//				left->globalCommLog.mpicommlog[0][0]->arguments[3]=left->globalCommLog.mpicommlog[1][0]->arguments[3];
//				LOG(INFO)<<"AAAYHB"<<dyn_cast < ConstantExpr > (left->globalCommLog.mpicommlog[0][0]->arguments[3])->getSExtValue();
//				LOG(INFO)<<"AAAYHB"<<dyn_cast < ConstantExpr > (right->globalCommLog.mpicommlog[0][0]->arguments[3])->getSExtValue();
				// end
				sp.first->globalCommLog.wildcardmatches.push_back(p);
		//		LOG(INFO)<<"state: "<<sp.first;
				//sp.first->executor->location2state.insert(Executor::location2state_ty::value_type(sp.first->globalCommLog.getWildcardMatches(),sp.first));
				sp.first->executor->insertLoctoState(Executor::location2state_ty::value_type(sp.first->globalCommLog.getWildcardMatches(),sp.first));
				LOG(WARNING)<<"Wildcard " <<*mset.back()<< " matches with "<<*mset.front()<<" args:"<<mset.back()->arguments[3];
				sp.first->globalCommLog.preparFireSRPair(*sp.first,mset);
				wildreached = true;
				sp.first->globalCommLog.check(*sp.first,sub+1);

				lastState = sp.second;
				i++;
			}
			else {
				lastState->globalCommLog.preparFireSRPair(*lastState,mset);
				lastState->globalCommLog.wildcardmatches.push_back(p);
			//	LOG(INFO)<<"state: "<<lastState;
				//lastState->executor->location2state.insert(Executor::location2state_ty::value_type(lastState->globalCommLog.getWildcardMatches(),lastState));
				lastState->executor->insertLoctoState(Executor::location2state_ty::value_type(lastState->globalCommLog.getWildcardMatches(),lastState));
				LOG(WARNING)<<"Wildcard " <<*mset.back()<< " matches with "<<*mset.front()<< " args:"<<mset.back()->arguments[3];
				wildreached = true;
				//FIXME::is it neccessary?
				//lastState->globalCommLog.check(*lastState,sub+1);
			}
		}
	}
	else
	{
		LOG(WARNING)<<"preparFireMatchedPairs: UNKNOWN match set";
		BOOST_FOREACH(std::list<CommLogItem *>& mset,state.globalCommLog.matchpairs) {
			BOOST_FOREACH(CommLogItem *item,mset) {
				item->print(LOG(INFO));
			}
		}
		LOG(ERROR)<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!exit!!!!!!!!!!!!!";
		exit(1);
	}

	return 0;
}

//TODO:: Two things to be done:
//

/*YU:  use check function to deadlock or find match set to move forward of SE*/
//1. deal with enabled not empty and getpairs returns 0. This may be deadlock or just ok
//        (consider a isend without any recv mathced.).
//2.  when some process get finalized.

bool CommLogManager::check(ExecutionState &state, int sub) {
	unsigned long i, j, subdepth = sub;
#if 0
	LOG(INFO)<<" state " << &state << " checking info of state:";
	//state.globalCommLog.dump(state, LOG(INFO));
	state.dumpStack();
#endif
	if(depth==7){
		LOG(INFO)<<"got it";
	}
	bool haswildcard, hasnormal, hasmc;
	bool wildreached = !state.globalCommLog.funcstofire.empty();//why?
	int counter = 0;
	int enabled_num = 0;
	std::list<CommLogItem *>::iterator it;
	std::map<mpi_rank_t, std::vector<CommLogItem *> > enabled;
	haswildcard = hasnormal = false;
	if (!sub) {
		getAllAncestors(state, state.globalCommLog.mpicommlog);
		memset(rankopFired, 0, sizeof(rankopFired));
		state.globalCommLog.dirtycalls.clear();
	}
	//1. get all enabled operations into $enabled$. CommLogItem $item$ is said to be enabled
	//  if all it's ancestors are matched.

	// Now all threads blocked. for all enabled, if
	// a. all matched pairs are not wildcard.     Explore all possible enabled, match all possible pairs and then form new enabled set and explore.
	// b. Some are normal ones and some are wild.  Let all normal pairs go, except those wildcard ones.
	// c. all are wild matches.                    Let one pair go and wait for the next CHECK.
	/*YU
	 * we use this while loop, beacause there may exist some matching which can only be enabled after some matching happened!
	 * beacause we find matchings only from the enabled list!
	 * */
	while (getAllEnabled(state, enabled)) { //YU: get all enabled from the global map of current state!
#ifdef COMMLOG_DEBUG
		LOG(INFO)<<"state "<<&state<<" CHECK begin,depth/subdepth: "<<depth<<"/"<<subdepth<<" with enabled:";
		for (i = 0, enabled_num = 0; i < enabled.size(); i++) {
			LOG(INFO)<<"rank "<<i<<" enabled:"<< enabled[i].size();
			if(!enabled[i].empty())
			enabled_num++;
			for (j = 0; j < enabled[i].size(); j++) {
				LOG(INFO)<<*enabled[i][j]; //<< enabled[i][j]->arguments[0];
			}
		}
		LOG(INFO)<<"enabled END*********************************";
#endif

		//2. pick matched set out of those enabled.
		hasmc=getMatchedPairs(state, enabled, haswildcard,hasnormal);
		// case a. reaches here, we exhausted all enabled normal pairs.
		/*the matching set did not find a matching when blocking, means deadlock!*/
		if (!hasmc) {
			// This means we do not find any matched pairs in this section.
			if (counter == 0 && sub == 0 ) {
				//state.globalCommLog.dump(state, LOG(INFO));
				//state.dumpStack();
				LOG(INFO)<<"state" <<&state <<"******** hang (deadlock happends,check the comm log.)";
				 std::map<std::string, klee::ExecutionState*>::iterator ite = state.executor->location2state.begin();
				for (; ite != state.executor->location2state.end();ite++) {
						if (ite->second == &state) {
							LOG(INFO)<<"matchings -->: "<<ite->first<<"\n";
							break;
						}
				}
				theStatisticManager->iterationForDeadlock=++theStatisticManager->iteration;
				LOG(INFO)<<"found deadlock in iteration: "<<theStatisticManager->iterationForDeadlock<<"\n";
				state.executor->deadlockHappened=true;
//				std::set<ExecutionState*>::iterator it = (state.executor->states).begin();
//				while (it != (state.executor->states).end()) {
//					(state.executor->removedStates).insert(*it);
//					it++;
//				}
				//exit(0);
				return -1;
			} else if (enabled_num == 1 && getFinishedCnt() == mpinodes - 1) {
				for (i = 0, enabled_num = 0; i < enabled.size(); i++) {
					if (!enabled[i].empty())
					break;
				}
				if (unblockWaitofIsend(state, state.globalCommLog.mpicommlog[i], i)) {
					LOG(INFO)<<"Unblocking Wait of Irecv, rank "<< i;
					state.globalCommLog.dump(state, LOG(INFO));
				}
			}
			break;
		}

#ifdef COMMLOG_DEBUG
		LOG(INFO)<<"MATCH pairs:";
		for (i = 0; i < matchpairs.size(); i++) {
			for (it = matchpairs[i].begin(); it != matchpairs[i].end(); it++) {
				LOG(INFO)<<"["<<(*it)->rank<<","<<(*it)->index<< "]" << (*it)->funcname;
			}
		}
		LOG(INFO)<<"MATCH END*********************************";
#endif
		//case b. normal pairs exhausted, now we let it go, leaving wild pairs alone.
		//case c. all wildcard matches. fire one pair, continue to se .
		/*
		 * YU: since we need to rewrite wildcard, we do this to avoid to miss some matchings of wildcard, so jump out of the while to handle it as later as possible.
		 * */
		if(hasnormal && haswildcard || !hasnormal && haswildcard && wildreached) {
			string reason = hasnormal && haswildcard?"normal pairs run out.(mixed normal & wildcard pairs)":"pure wildcard goes one(others should not be explored.)";
			LOG(INFO)<<"check break early for "<<reason;
			break;
		}
		//3. Prepair to fire matched set operations.
		preparFireMatchedPairs(state, haswildcard, subdepth, wildreached);
		subdepth++;
		counter++;
		// This is for a state that inserts itself a pair in state.globalCommLog.funcstofire.
		// take the sate A for example.
		// when current pair 1,2,3 are wildcard ones, A would produce 3 states(via fork.): a, b, c. and c would be itself.
		// Assume that a picks pair1, b picks pair2, c picks pair3; for state c. it can not break from the above break,
		// coz variable "wildreached" is false.
		// After fuction "preparFireMatchedPairs", c would pick pair3(i.e. push the functions into  state.globalCommLog.funcstofire),
		// so we should break it here. Or else it would fork states again and exhaust all pair1,2,3 for state c..which is too
		// aggressive.
		if(hasnormal && haswildcard || !hasnormal && haswildcard && wildreached) {
			string reason = hasnormal && haswildcard?"normal pairs run out.(mixed normal & wildcard pairs)":"pure wildcard goes one(others should not be explored.)";
			LOG(INFO)<<"check break early for "<<reason;
			break;
		}
	}
	// This means that we exhausted all the pairs, and don't worry, this is not the first subdepth.
	// if counter ==0 && subdepth==0 this should be a deadlock.
	if (counter == 0 && subdepth != 0 && !wildreached) {
		LOG(INFO)<<"state "<<&state<<" counter ==0 subdepth!=0";
		return 0;
	}
	//4. push all the funcs into stack of each thread with respect to rank. Wake up those threads.
	typedef std::map<mpi_rank_t, std::list<std::pair<Function *, CommLogItem*> > >::value_type funcpair;
	bool returntostackback;
	//LOG(INFO)<<"state "<< &state <<" state.globalCommLog.funcstofire size "<<state.globalCommLog.funcstofire.size();
	BOOST_FOREACH(funcpair &p, state.globalCommLog.funcstofire){
	Thread &thread=state.threads.find(rank2thread[p.first])->second;

	i = p.second.size()-1;
	returntostackback = true;
#ifdef COMMLOG_DEBUG
	LOG(INFO)<<"push functions into stack of rank "<<p.first<<" :";
#endif
	typedef std::pair<Function *,CommLogItem*> ty_pair;

	BOOST_REVERSE_FOREACH(ty_pair &fpair, p.second) {

#ifdef COMMLOG_DEBUG
		uint64_t temp=0xF;
		if(argments4funcs[p.first][i].size() >4)
		temp= dyn_cast < ConstantExpr > (argments4funcs[p.first][i][3])->getSExtValue();
		LOG(INFO)<<"function "<<fpair.first->getName().str()<<"("<< temp<<")";
#endif
		if(thread.getEnabled()) {
			LOG(ERROR)<<"unexpected enabled thread, rank: "<<p.first<< ": "<<thread.getPid()<<"/"<<thread.getTid();
			state.globalCommLog.dump(state,LOG(INFO));
			state.dumpStack();
		}
		assert(i>=0 && !thread.getEnabled());

		state.executor->threadexecutecall(state,thread, p.first, fpair.first, argments4funcs[p.first][i],returntostackback);

		if (state.executor->checkGlobalAllDelayed()) {
		    return false;
		}

		i--;
		returntostackback = false;
	}
#if 0
	Function *f = packFunctions(state,p.first,depth,p.second);
	std::vector<ref<Expr> > args;
	args.clear();
	state.executor->threadexecutecall(state,thread, f, args,returntostackback);
#endif
	//fixArgments(state,thread,p.second);
#ifdef COMMLOG_DEBUG
	LOG(INFO)<<"push END*********&& wake up thread "<<&thread<< " in "<< wlstids[p.first]<<"***********";
#endif

	LOG(INFO)<<"Wake up rank "<<p.first<<" for func push.";
	//if(!state.globalCommLog.rank_blocked[p.first])
	if(!thread.getEnabled())
	//state.notifyOne(wlstids[p.first], rank2thread[p.first]);
		/*YU: wake up the block thread*/
	state.notifyOne(thread.waitingList, rank2thread[p.first]);
}
#if 0
state.dumpStack();
#endif
	depth++;
	state.globalCommLog.funcstofire.clear();
	state.globalCommLog.argments4funcs.clear();

	//5. Deal with finalized.
	if (!getMatchedPairs(state, enabled, haswildcard, hasnormal)) {
		BOOST_FOREACH(mpicommlog_ty::value_type & p, state.globalCommLog.mpicommlog) {
						Thread &thread = state.threads.find(state.globalCommLog.rank2thread[p.first])->second;
						// Here we deal with MPI_Finalized, by just wake up these processes.
						//Note that we should ensure that all operations before finalize should be matched.
						if (state.globalCommLog.allNodesFinished() && state.globalCommLog.allMatched(state, p.second) &&
							!p.second.back()->funcname.compare("PMPI_Finalize") &&
							!thread.getEnabled()) {
							//!state.threads.find(state.globalCommLog.rank2thread[p.first])->second.getEnabled()) {
							if (state.waitingLists[state.globalCommLog.wlstids[p.first]].find(
									state.globalCommLog.rank2thread[p.first]) !=
								state.waitingLists[state.globalCommLog.wlstids[p.first]].end()) {
								LOG(INFO) << "Wake up rank " << p.first << " for finalize.";
								state.notifyOne(state.globalCommLog.wlstids[p.first],
												state.globalCommLog.rank2thread[p.first]);
							}
						}
					}
	}
	//Sometimes we fork state, and the forked one got all thread not enabled; but after we check, some threads may be marked as
	//enabled. So we should make sure that crtThread is enabled.
	if (!state.crtThread().getEnabled())
		state.executor->schedule(state, false);
	return 0;
}

void CommLogManager::dump(ExecutionState &state, std::ostream &os) {

	//1. dump commlog.
	os << "\n********YHB dumps the info of state " << &state << "********" << "\n";
	BOOST_FOREACH(pair_type &p, state.globalCommLog.mpicommlog){
					os << "  Rank " << p.first << " Commlog begin**********" << "\n";
					BOOST_FOREACH(CommLogItem *it, p.second) {
									os << *it << " matched?: " << state.globalCommLog.logmatchflag[it] <<
									"  0: unmatched; 1: matched for deterministic receive; 2: partially matching for wildcard\n";
								}
				}
	os << "*****************end of state info *****************" << "\n";
	//2. dump enabled.
	std::map<mpi_rank_t, std::vector<CommLogItem *> > enabled;
	getAllEnabled(state, enabled);
	os << "********************YHB dumps all the enabled operation of every rank in state above*************************** \n";
	BOOST_FOREACH(pair_type &p, enabled) {
					os << "  Rank " << p.first << " enabled operations:**********" << "\n";
					BOOST_FOREACH(CommLogItem *it, p.second) {
									os << *it << "\n";
								}
					os << "*******************************end of enable dump ***************************" << "\n";
				}

	//3. rank & process info.
	for (int i = 0; i < mpinodes; i++) {
		os << "rank " << i << " thread " << state.globalCommLog.rank2thread[i].second << ":" <<
		state.globalCommLog.rank2thread[i].first <<
		" blocked/enabled :" << state.globalCommLog.rank_blocked[i] << "/" <<
		state.threads.find(state.globalCommLog.rank2thread[i])->second.getEnabled() << "\n";
	}
	return;
}

bool CommLogManager::allNodesNotEnabled(ExecutionState &state) {
	for (int i = 0; i < state.globalCommLog.mpinodes; i++) {
		if (state.threads.find(state.globalCommLog.rank2thread[i])->second.getEnabled()) {
			return false;
		}
	}
	return true;
}

bool CommLogManager::allMatched(ExecutionState &state, std::vector<CommLogItem *> &v) {
	if (v.empty() || v.back()->mpi_op != COMM_FIN)
		return false;

	BOOST_FOREACH(CommLogItem *item, v) {
		if (state.globalCommLog.logmatchflag[item] == UNMATCHED
				&& item->isBlock()   /// added by zhenbang
		   )
						return false;
	}
	return true;
}

bool CommLogManager::unblockWaitofIsend(ExecutionState &state, std::vector<CommLogItem *> &proclog,
										int rank) {
	if (proclog.empty() || rank < 0 || rank > 128 || !state.globalCommLog.rank_blocked[rank]
		|| proclog.back()->getFuncname().compare("PMPI_Wait"))
		return false;

	// Now we know that this process is blocked by PMPI_Wait.
	BOOST_FOREACH(CommLogItem *item, proclog) {
					if (item != NULL && item->isSend() && item->happensBefore(*proclog.back()) &&
						state.globalCommLog.logmatchflag[item] == UNMATCHED) if (!state.threads.find(
							state.globalCommLog.rank2thread[rank])->second.getEnabled()) {
						//LOG(INFO)<<"Wake up rank "<<rank<<" for Wait of Isend.";
						state.globalCommLog.rank_blocked[rank] = false;
						state.notifyOne(state.globalCommLog.wlstids[rank], state.globalCommLog.rank2thread[rank]);
						return true;
					}
				}
	return false;
}
#if 0
bool CommLogManager::fixArgments(ExecutionState &state, Thread & thread,
		std::list<std::pair<Function *, CommLogItem *> > &functions) {
	typedef std::pair<Function *, CommLogItem *> ty_fpair;
	KFunction *tempkf;
	KInstruction * ki;
	KFunction *kf = thread.stack.back().kf;
	unsigned j = 0, e;
	BOOST_REVERSE_FOREACH( ty_fpair& p, functions) {
		tempkf=state.executor->kmodule->functionMap[p.first];
		ki = kf->instructions[j++];
		if(isa<CallInst>(ki->inst) && j < kf->numInstructions) {
			for (e = 0; e <= p.second->arguments.size()-1; ++e) {
				state.executor->bindArgument(tempkf,e,state,p.second->arguments[e]);
			}
		}
	}
	return true;
}

#define __DEBUG_PACK
Function * CommLogManager::packFunctions(ExecutionState &state, mpi_rank_t rank, int depth,
		std::list<std::pair<Function *, CommLogItem *> > &functions) {
	assert(functions.size() >= 2 && "less that 2 functions should not be packed.");

	int i = 0, j, e;

	std::string tempname = "__packed_";
	tempname.append(boost::lexical_cast<string>(rank));
	tempname.append(boost::lexical_cast<string>(depth));

	Function * packedfunc = cast<Function>(
			state.executor->kmodule->module->getOrInsertFunction(tempname,
					Type::getVoidTy(getGlobalContext()), NULL));
	// Add a basic block to the function.
	BasicBlock *BB = BasicBlock::Create(getGlobalContext(), "entry", packedfunc);
	//IRBuilder<> builder(BB);
	//builder.creatCall();
	typedef std::pair<Function *, CommLogItem *> ty_fpair;
	CallInst * ci;
	BOOST_REVERSE_FOREACH( ty_fpair& p, functions) {
#ifdef  __DEBUG_PACK
		LOG(INFO)<<"function "<<p.first->getName().str()<<" being packing...";
#endif

		std::vector<llvm::Value*> ArgsV;
		CallSite cs((Instruction *)p.second->callins);
		for (j = 0, e = p.first->arg_size(); j != e; ++j) {
			ArgsV.push_back(p.second->callins->getOperand(j));
			//ArgsV.push_back(cs.getArgument(j));

#if 0
			// we can not assert the same ins into two BB. So what can we do ?
			if (Instruction *inst = dyn_cast<Instruction>(ArgsV[j])) {
				BB->getInstList().insert(BB->getInstList().begin(),inst);
			}
			else if (Argument *a = dyn_cast<Argument>(v)) {
			} else if (isa<BasicBlock>(v) || isa<InlineAsm>(v) ||
					isa<MDNode>(v)) {
			} else {
				assert(isa<Constant>(v));
				Constant *c = cast<Constant>(v);
			}
#endif
		}
		ArgsV.reserve(p.first->arg_size());
		ci = CallInst::Create(p.first, ArgsV, "", BB);

#if 0//def  __DEBUG_PACK
		for (j = 0, e = p.first->arg_size(); j != e; ++j) {
			ci->getOperand(j)->dump();
		}
#endif

		state.executor->kmodule->infos->infos.insert(std::make_pair(ci,
						state.executor->kmodule->infos->dummyInfo));//InstructionInfo(0,"",-1, 0,0)));
	}
	ReturnInst *ri = ReturnInst::Create(getGlobalContext(), BB);
	state.executor->kmodule->infos->infos.insert(
			std::make_pair(ri, state.executor->kmodule->infos->dummyInfo));	//InstructionInfo(0, "", -1, 0, 0)));

#ifdef  __DEBUG_PACK
// TODO: print this function.

#endif
	KFunction *kf = new KFunction(packedfunc, state.executor->kmodule);
	state.executor->kmodule->functionMap.insert(std::make_pair(packedfunc, kf));

	KInstruction * ki;

	for (j = 0; j < kf->numInstructions; ++j) {
		ki = kf->instructions[j];
		ki->info = &(state.executor->kmodule->infos->getInfo(ki->inst));
	}

	return packedfunc;

}

#undef __DEBUG_PACK

llvm::Function * CommLogManager::getFatherFunction(ExecutionState &state, std::string & funcname) {
	int pos = funcname.find("PMPI_", 0);
	if (pos == string::npos)
	return NULL;

	Function *f = state.executor->kmodule->module->getFunction(
			funcname.substr(1, funcname.size() - 1));
	return f;
}
#endif

std::string CommLogManager::getWildcardMatches() {
	std::string res;
	typedef std::pair<Location, Location> ty_locpair;
	BOOST_FOREACH(ty_locpair&p, wildcardmatches){
	res.append(boost::lexical_cast<string>(p.first.PID));
	res.append(boost::lexical_cast<string>(","));
	res.append(boost::lexical_cast<string>(p.first.index));
	res.append(boost::lexical_cast<string>(","));
	res.append(boost::lexical_cast<string>(p.second.PID));
	res.append(boost::lexical_cast<string>(","));
	res.append(boost::lexical_cast<string>(p.second.index));
	res.append(boost::lexical_cast<string>(";"));
}
	LOG(INFO)<<"getWildcardMatches:"<<res;
	return res;
}
