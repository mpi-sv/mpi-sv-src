//===-- ExecutionState.h ----------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef KLEE_EXECUTIONSTATE_H
#define KLEE_EXECUTIONSTATE_H

#include "klee/Constraints.h"
#include "klee/Expr.h"
#include "klee/Internal/ADT/TreeStream.h"

// FIXME: We do not want to be exposing these? :(
#include "../../lib/Core/AddressSpace.h"
#include "klee/Internal/Module/KInstIterator.h"

#include "llvm/Support/TimeValue.h"

#include "klee/Threading.h"
#include "klee/MultiProcess.h"
#include "klee/AddressPool.h"
#include "klee/StackTrace.h"

#include "mpise/commlogManager.h"//Added by herman.

#include <map>
#include <set>
#include <vector>

#include <boost/shared_ptr.hpp>
using boost::shared_ptr;

#include <glog/logging.h>

using namespace llvm;

namespace cloud9 {
namespace worker {
class SymbolicState;
}
}

namespace llvm {
class Instruction;
}

namespace klee {
  class Array;
  class CallPathNode;
  struct Cell;
  struct KFunction;
  struct KInstruction;
  class MemoryObject;
  class PTreeNode;
  struct InstructionInfo;
  class Executor;

  class ExecutionState;

namespace c9 {
std::ostream &printStateStack(std::ostream &os, const ExecutionState &state);
std::ostream &printStateConstraints(std::ostream &os, const ExecutionState &state);
std::ostream &printStateMemorySummary(std::ostream &os, const ExecutionState &state);
}

std::ostream &operator<<(std::ostream &os, const ExecutionState &state); // XXX Cloud9 hack
std::ostream &operator<<(std::ostream &os, const MemoryMap &mm);

typedef uint64_t wlist_id_t;


class ExecutionState {
  friend class Executor;
  friend class ObjectState;

public:
  typedef std::vector<StackFrame> stack_ty;

  typedef std::map<thread_uid_t, Thread> threads_ty;
  typedef std::map<process_id_t, Process> processes_ty;
  typedef std::map<int,int> MSInfo_ty;
  bool delayed; //added by yhb to indicate the pruning states, initialized to false.
  bool wildContained; // added by yhb to indicate whether need to do CSP Model Checking.
	bool detectedMSPattern; // used to indicate whether we detect Master-Slave pattern, initially false
	int counterForMS; // used in MS, otherwise -1. it is used to record the order of the recv(*);send(*.source)
  string fileForLastRecvMaster;// added by yhb, to make sure the location of the first repetitive recv in M-S is recorded!
	int lineNumberForLastRecvMaster;
 // typedef std::map<proc_funcid_t,>


private:
  // unsupported, use copy constructor
  ExecutionState &operator=(const ExecutionState&); 
  std::map< std::string, std::string > fnAliases;

  cloud9::worker::SymbolicState *c9State;

  void setupMain(KFunction *kf);
  void setupTime();
  void setupAddressPool();
public:
  /* System-level parameters */
  Executor *executor;

  unsigned depth;

  /// Disables forking, set by user code.
  bool forkDisabled;

  mutable double queryCost;
  double weight;

  TreeOStream pathOS, symPathOS;
  unsigned instsSinceCovNew;

  bool coveredNew;
  sys::TimeValue lastCoveredTime;

  void setCoveredNew() {
    coveredNew = true;
    lastCoveredTime = sys::TimeValue::now();
  }

  std::map<const std::string*, std::set<unsigned> > coveredLines;

  PTreeNode *ptreeNode;

  int crtForkReason;
  Instruction *crtSpecialFork;


  set<unsigned int> currentLTLStates;
  /// ordered list of symbolics: used to generate test cases. 
  //
  // FIXME: Move to a shared list structure (not critical).
  std::vector< std::pair<const MemoryObject*, const Array*> > symbolics;

  ConstraintManager globalConstraints;
  CommLogManager  globalCommLog;


  // For a multi threaded ExecutionState
  threads_ty threads;
  processes_ty processes;

  wlists_ty waitingLists;
  wlist_id_t wlistCounter;

  uint64_t stateTime;

  AddressPool addressPool;
  AddressSpace::cow_domain_t cowDomain;


  //*************************************************************end adding

  Thread& createThread(thread_id_t tid, KFunction *kf);
  Process& forkProcess(process_id_t pid);
  void terminateThread(threads_ty::iterator it);
  void terminateProcess(processes_ty::iterator it);

  threads_ty::iterator nextThread(threads_ty::iterator it) {
    if (it == threads.end())
      it = threads.begin();
    else {
      it++;
      if (it == threads.end())
        it = threads.begin();
    }

    crtProcessIt = processes.find(crtThreadIt->second.getPid());

    return it;
  }

  void scheduleNext(threads_ty::iterator it) {
    assert(it != threads.end());

    crtThreadIt = it;
    crtProcessIt = processes.find(crtThreadIt->second.getPid());
  }

  wlist_id_t getWaitingList() { return wlistCounter++; }
  void sleepThread(wlist_id_t wlist);
  void notifyOne(wlist_id_t wlist, thread_uid_t tid);
  void notifyAll(wlist_id_t wlist);

  threads_ty::iterator crtThreadIt;
  processes_ty::iterator crtProcessIt;

  unsigned int preemptions;


  /* Shortcut methods */

  Thread &crtThread() { return crtThreadIt->second; }
  const Thread &crtThread() const { return crtThreadIt->second; }

  Process &crtProcess() { return crtProcessIt->second; }
  const Process &crtProcess() const { return crtProcessIt->second; }

  ConstraintManager &constraints() { return globalConstraints; }
  const ConstraintManager &constraints() const { return globalConstraints; }

  AddressSpace &addressSpace() { return crtProcess().addressSpace; }
  const AddressSpace &addressSpace() const { return crtProcess().addressSpace; }

  KInstIterator& pc() { return crtThread().pc; }
  const KInstIterator& pc() const { return crtThread().pc; }

  KInstIterator& prevPC() { return crtThread().prevPC; }
  const KInstIterator& prevPC() const { return crtThread().prevPC; }

  stack_ty& stack() { return crtThread().stack; }
  const stack_ty& stack() const { return crtThread().stack; }

  std::string getFnAlias(std::string fn);
  void addFnAlias(std::string old_fn, std::string new_fn);
  void removeFnAlias(std::string fn);

  /* Logging support */
  unsigned char state_id[20];
  unsigned char parent_id[20];
  unsigned char fork_id[20];

  /* Various counters */
  uint64_t totalInstructions;
  uint64_t totalBranches;
  uint64_t totalForks;
  mutable uint64_t totalQueries; // TODO(bucur): Fix this atrocity
  mutable uint64_t totalTime;
public:
  ExecutionState(Executor *_executor, KFunction *kf);

  // XXX total hack, just used to make a state so solver can
  // use on structure
  ExecutionState(Executor *_executor, const std::vector<ref<Expr> > &assumptions);

  ~ExecutionState();
  
  ExecutionState *branch(bool copy = false);

  void pushFrame(Thread &t, KInstIterator caller, KFunction *kf) {
    t.stack.push_back(StackFrame(caller,kf));
  }
  void pushFrame(KInstIterator caller, KFunction *kf) {
    pushFrame(crtThread(), caller, kf);
  }

  void popFrame(Thread &t) {
    StackFrame &sf = t.stack.back();
    for (std::vector<const MemoryObject*>::iterator it = sf.allocas.begin(),
           ie = sf.allocas.end(); it != ie; ++it)
      processes.find(t.getPid())->second.addressSpace.unbindObject(*it);
    t.stack.pop_back();
  }
  void popFrame() {
    popFrame(crtThread());
  }

  void addSymbolic(const MemoryObject *mo, const Array *array) { 
    symbolics.push_back(std::make_pair(mo, array));
  }
  void addConstraint(ref<Expr> e) { 
    constraints().addConstraint(e);
  }

  bool merge(const ExecutionState &b);

  cloud9::worker::SymbolicState *getCloud9State() const { return c9State; }
  void setCloud9State(cloud9::worker::SymbolicState *state) { c9State = state; }

  StackTrace getStackTrace() const;

  bool isExternalCallSafe() const;
  const InstructionInfo * getCurrentStackCallerInfo();
  void dumpStack();
};

}

#endif
