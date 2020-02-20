//===-- Executor_Calls.cpp ------------------------------------------------===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "klee/Executor.h"
#include "klee/Statistics.h"
#include "klee/util/ExprPPrinter.h"

#include "Context.h"
#include "ExternalDispatcher.h"
#include "StatsTracker.h"
#include "Memory.h"
#include "MemoryManager.h"
#include "SpecialFunctionHandler.h"
#include "TimingSolver.h"
#include "klee/Internal/Module/InstructionInfoTable.h"

#include "llvm/Function.h"
#include "llvm/Module.h"
#include "llvm/LLVMContext.h"
#include "llvm/IntrinsicInst.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

#include <glog/logging.h>

#include <set>
#include <list>
#include <sstream>
#include <algorithm>

using namespace llvm;

namespace {

cl::opt<bool> AllowExternalSymCalls("allow-external-sym-calls", cl::init(false));

cl::opt<bool> DebugCallHistory("debug-call-history", cl::init(false));

cl::opt<bool> NoExternals("no-externals", cl::desc("Do not allow external functin calls"));

cl::opt<bool> SuppressExternalWarnings("suppress-external-warnings");

cl::list<std::string> AvoidModelledFunctions("avoid-modelled-functions", cl::desc(""));

}

extern std::map<std::string, std::string> sensitiveEventFunctionMap;

namespace klee {

// XXX shoot me
static const char *okExternalsList[] = { "printf", "fprintf", "puts", "getpid" };
static std::set<std::string> okExternals(okExternalsList,
		okExternalsList + (sizeof(okExternalsList) / sizeof(okExternalsList[0])));


/**
 * Check whether there is a violation
 * @param state
 * @param f
 * @param arguments
 * @return false represents there exists a violation
 */
bool checkLTLViolation(LTL_Property &property, ExecutionState &state, mpi_rank_t rank, Function *f, std::vector<ref<Expr> > &arguments) {

    /// construct the event

	string funcName = f->getName().str();

	if (sensitiveEventFunctionMap.find(funcName) == sensitiveEventFunctionMap.end()) {
	    return true;
	}

	/// get the function name first
	string event = sensitiveEventFunctionMap[funcName];

    /// get the destination/source process rank
    int tRank = -1;
    if (klee::ConstantExpr *c = dynamic_cast<klee::ConstantExpr *>(arguments[3].get())) {
        tRank = c->getSExtValue();
    };

    /// construct the event
    std::stringstream eventStream;
    eventStream << event << "(" << rank << "," << tRank << ")";
    event = eventStream.str();

    LOG(INFO) << "--------Event LTL ----------" << event << "--------------------";

    if (property.get_value(event) > 0)  {
        /// (1) generate the atoms with respect to the fucntion call
        int atom = property.get_value(event);
        //set <int> current_atomsproperty.get_Atoms(property.get_value(event));
        // the type of 'f' should be string like 'send' in hwj's implementation. I'm not sure what type it is here?

        /// (2) advance the currentLTLStates states of the current state
        set <unsigned int> current_states_temp;
        current_states_temp.clear();
        for (auto current_state : state.currentLTLStates) {
            set <unsigned int> result;
            result.clear();
            property.Automaton->transite(current_state, atom, result);
            current_states_temp.insert(result.begin(), result.end());
        }
        if (current_states_temp.size() > 0) {
            state.currentLTLStates.clear();
            state.currentLTLStates.insert(current_states_temp.begin(),current_states_temp.end());
        }

        for (auto s : state.currentLTLStates) {
            LOG(INFO) << s << "\n";
        }

        /// (3) Check whether there exists a final state
        for (auto current_state : state.currentLTLStates) {
            if (property.Automaton->finalStates.find(current_state) != property.Automaton->finalStates.end()) {
                return false;
            }
        }
    }

    return true;
}

void Executor::executeCall(ExecutionState &state, KInstruction *ki, Function *f,
		std::vector<ref<Expr> > &arguments) {
	fireControlFlowEvent(&state, ::cloud9::worker::CALL);

	if (f && DebugCallHistory) {
		unsigned depth = state.stack().size();

		LOG(INFO)<< "Call[" << &state << "]: " << std::string(depth, ' ') << f->getName().str();
	}

	Instruction *i = NULL;
	if (ki)
		i = ki->inst;

	if (ki && f && f->isDeclaration()) {
		switch (f->getIntrinsicID()) {
		case Intrinsic::not_intrinsic:
			// state may be destroyed by this call, cannot touch
			callExternalFunction(state, ki, f, arguments);
			break;

			// va_arg is handled by caller and intrinsic lowering, see comment for
			// ExecutionState::varargs
		case Intrinsic::vastart: {
			StackFrame &sf = state.stack().back();
			assert(sf.varargs && "vastart called in function with no vararg object");

			// FIXME: This is really specific to the architecture, not the pointer
			// size. This happens to work fir x86-32 and x86-64, however.
			Expr::Width WordSize = Context::get().getPointerWidth();
			if (WordSize == Expr::Int32) {
				executeMemoryOperation(state, true, arguments[0], sf.varargs->getBaseExpr(), 0);
			} else {
				assert(WordSize == Expr::Int64 && "Unknown word size!");

				// X86-64 has quite complicated calling convention. However,
				// instead of implementing it, we can do a simple hack: just
				// make a function believe that all varargs are on stack.
				executeMemoryOperation(state, true, arguments[0], ConstantExpr::create(48, 32), 0); // gp_offset
				executeMemoryOperation(state, true,
						AddExpr::create(arguments[0], ConstantExpr::create(4, 64)),
						ConstantExpr::create(304, 32), 0); // fp_offset
				executeMemoryOperation(state, true,
						AddExpr::create(arguments[0], ConstantExpr::create(8, 64)),
						sf.varargs->getBaseExpr(), 0); // overflow_arg_area
				executeMemoryOperation(state, true,
						AddExpr::create(arguments[0], ConstantExpr::create(16, 64)),
						ConstantExpr::create(0, 64), 0); // reg_save_area
			}
			break;
		}
		case Intrinsic::vaend:
			// va_end is a noop for the interpreter.
			//
			// FIXME: We should validate that the target didn't do something bad
			// with vaeend, however (like call it twice).
			break;

		case Intrinsic::vacopy:
			// va_copy should have been lowered.
			//
			// FIXME: It would be nice to check for errors in the usage of this as
			// well.
		default:
			LOG(FATAL)<< "Unknown intrinsic: " << f->getName().data();
		}
		if (InvokeInst *ii = dyn_cast<InvokeInst>(i))
		transferToBasicBlock(ii->getNormalDest(), i->getParent(), state);
	} else {
		// FIXME: I'm not really happy about this reliance on prevPC but it is ok, I
		// guess. This just done to avoid having to pass KInstIterator everywhere
		// instead of the actual instruction, since we can't make a KInstIterator
		// from just an instruction (unlike LLVM).

		/**
		 * Hack at here to avoid execution to uclibc
		 */
		// to be implemented at AvoidModelledFunctions
		std::vector<std::string>::iterator it =
				find(AvoidModelledFunctions.begin(), AvoidModelledFunctions.end(), f->getName().str());
		if (it != AvoidModelledFunctions.end()) {
			callExternalFunction(state, ki, f, arguments);
			if (InvokeInst *ii = dyn_cast<InvokeInst>(i))
				transferToBasicBlock(ii->getNormalDest(), i->getParent(), state);
			return;
		}

//		if (f->getName().equals("fopen") ||
//			f->getName().equals("fseek") ||
//			f->getName().equals("fread_unlocked") ||
//			f->getName().equals("fclose") ||
//			f->getName().equals("fwrite_unlocked") ||
////			f->getName().equals("fprintf") ||
//			f->getName().equals("strcat") ||
//			f->getName().equals("fgets_unlocked")) {
//			//llvm::errs() << "--------------------------enter it---------------\n";
//			callExternalFunction(state, ki, f, arguments);
//			if (InvokeInst *ii = dyn_cast<InvokeInst>(i))
//				transferToBasicBlock(ii->getNormalDest(), i->getParent(), state);
//			return;
//		}

		KFunction *kf = kmodule->functionMap[f];
		state.pushFrame(state.prevPC(), kf);
		state.pc() = kf->instructions;

		if (statsTracker)
		statsTracker->framePushed(state, &state.stack()[state.stack().size()-2]);//XXX TODO fix this ugly stuff

		// TODO: support "byval" parameter attribute
		// TODO: support zeroext, signext, sret attributes

		unsigned callingArgs = arguments.size();
		unsigned funcArgs = f->arg_size();
		if (!f->isVarArg()) {
			if (callingArgs > funcArgs) {
				LOG(WARNING) << "Calling " << f->getName().data() << " with extra arguments.";
			} else if (callingArgs < funcArgs) {
				terminateStateOnError(state, "calling function with too few arguments",
						"user.err");
				return;
			}
		} else {
			if (callingArgs < funcArgs) {
				terminateStateOnError(state, "calling function with too few arguments",
						"user.err");
				return;
			}

			StackFrame &sf = state.stack().back();
			unsigned size = 0;
			for (unsigned i = funcArgs; i < callingArgs; i++) {
				// FIXME: This is really specific to the architecture, not the pointer
				// size. This happens to work fir x86-32 and x86-64, however.
				Expr::Width WordSize = Context::get().getPointerWidth();
				if (WordSize == Expr::Int32) {
					size += Expr::getMinBytesForWidth(arguments[i]->getWidth());
				} else {
					size += llvm::RoundUpToAlignment(arguments[i]->getWidth(),
							WordSize) / 8;
				}
			}

			MemoryObject *mo = sf.varargs = memory->allocate(&state, size, true, false,
					state.prevPC()->inst);
			if (!mo) {
				terminateStateOnExecError(state, "out of memory (varargs)");
				return;
			}
			ObjectState *os = bindObjectInState(state, mo, true);
			unsigned offset = 0;
			for (unsigned i = funcArgs; i < callingArgs; i++) {
				// FIXME: This is really specific to the architecture, not the pointer
				// size. This happens to work fir x86-32 and x86-64, however.
				Expr::Width WordSize = Context::get().getPointerWidth();
				if (WordSize == Expr::Int32) {
					os->write(offset, arguments[i]);
					offset += Expr::getMinBytesForWidth(arguments[i]->getWidth());
				} else {
					assert(WordSize == Expr::Int64 && "Unknown word size!");
					os->write(offset, arguments[i]);
					offset += llvm::RoundUpToAlignment(arguments[i]->getWidth(),
							WordSize) / 8;
				}
			}
		}

		unsigned numFormals = f->arg_size();
		for (unsigned i=0; i<numFormals; ++i)
		bindArgument(kf, i, state, arguments[i]);
	}
}

Function* Executor::getCalledFunction(CallSite &cs, ExecutionState &state) {
	Function *f = cs.getCalledFunction();

	if (f) {
		std::string alias = state.getFnAlias(f->getName());
		if (alias != "") {
			llvm::Module* currModule = kmodule->module;
			Function* old_f = f;
			f = currModule->getFunction(alias);
			if (!f) {
				llvm::errs() << "Function " << alias << "(), alias for " << old_f->getName().str()
						<< " not found!\n";
				assert(f && "function alias not found");
			}
		}
	}

	return f;
}

void Executor::transferToBasicBlock(BasicBlock *dst, BasicBlock *src, ExecutionState &state) {
	// Note that in general phi nodes can reuse phi values from the same
	// block but the incoming value is the eval() result *before* the
	// execution of any phi nodes. this is pathological and doesn't
	// really seem to occur, but just in case we run the PhiCleanerPass
	// which makes sure this cannot happen and so it is safe to just
	// eval things in order. The PhiCleanerPass also makes sure that all
	// incoming blocks have the same order for each PHINode so we only
	// have to compute the index once.
	//
	// With that done we simply set an index in the state so that PHI
	// instructions know which argument to eval, set the pc, and continue.

	// XXX this lookup has to go ?
	KFunction *kf = state.stack().back().kf;
	unsigned entry = kf->basicBlockEntry[dst];
	state.pc() = &kf->instructions[entry];
	if (state.pc()->inst->getOpcode() == Instruction::PHI) {
		PHINode *first = static_cast<PHINode*>(state.pc()->inst);
		state.crtThread().incomingBBIndex = first->getBasicBlockIndex(src);
	}
}

void Executor::callExternalFunction(ExecutionState &state, KInstruction *target, Function *function,
		std::vector<ref<Expr> > &arguments) {
	// check if specialFunctionHandler wants it
	if (specialFunctionHandler->handle(state, function, target, arguments))
		return;

	callUnmodelledFunction(state, target, function, arguments);
}

void Executor::callUnmodelledFunction(ExecutionState &state, KInstruction *target,
		llvm::Function *function, std::vector<ref<Expr> > &arguments) {

/*	if (function->getName().equals("fclose")) {
		std::ostringstream oss;
		oss << arguments[0];
		LOG(WARNING) << "------------------------------" << oss.str() << "---------------------\n";
	} */

	if (NoExternals && !okExternals.count(function->getName())) {
		std::cerr << "KLEE:ERROR: Calling not-OK external function : " << function->getName().str()
				<< "\n";
		terminateStateOnError(state, "externals disallowed", "user.err");
		return;
	}

	// normal external function handling path
	// allocate 128 bits for each argument (+return value) to support fp80's;
	// we could iterate through all the arguments first and determine the exact
	// size we need, but this is faster, and the memory usage isn't significant.
	uint64_t *args = (uint64_t*) alloca(2*sizeof(*args) * (arguments.size() + 1));
	memset(args, 0, 2 * sizeof(*args) * (arguments.size() + 1));
	unsigned wordIndex = 2;
	for (std::vector<ref<Expr> >::iterator ai = arguments.begin(), ae = arguments.end(); ai != ae;
			++ai) {
		if (AllowExternalSymCalls) { // don't bother checking uniqueness
			ref<ConstantExpr> ce;
			bool success = solver->getValue(data::EXTERNAL_CALL_CONCRETIZATION, state, *ai, ce);
			assert(success && "FIXME: Unhandled solver failure");
			(void) success;
			ce->toMemory(&args[wordIndex]);
			wordIndex += (ce->getWidth() + 63) / 64;
		} else {
			ref<Expr> arg = toUnique(state, *ai);
			if (ConstantExpr *ce = dyn_cast<ConstantExpr>(arg)) {
				// XXX kick toMemory functions from here
				ce->toMemory(&args[wordIndex]);
				wordIndex += (ce->getWidth() + 63) / 64;
			} else {
				terminateStateOnExecError(state,
						"external call with symbolic argument: " + function->getName());
				return;
			}
		}
	}

	state.addressSpace().copyOutConcretes(&state.addressPool);

	if (!SuppressExternalWarnings) {
		StackTrace stack_trace = state.getStackTrace();

		std::ostringstream os;
		os << state << " Calling external: " << function->getName().str() << "(";
		for (unsigned i = 0; i < arguments.size(); i++) {
			os << arguments[i];
			if (i != arguments.size() - 1)
				os << ", ";
		}
		os << ")";

		if (state.isExternalCallSafe())
			VLOG(1) << os.str().c_str();
		else
			LOG(INFO)<< os.str().c_str();
	}

	bool success = externalDispatcher->executeCall(function, target->inst, args);
	if (!success) {
		terminateStateOnError(state, "failed external call: " + function->getName(),
				"external.err");
		return;
	}

	if (!state.addressSpace().copyInConcretes(&state.addressPool)) {
		terminateStateOnError(state, "external modified read-only object", "external.err");
		return;
	}

	Type *resultType = target->inst->getType();
	if (resultType != Type::getVoidTy(getGlobalContext())) {
		ref<Expr> e = ConstantExpr::fromMemory((void*) args, getWidthForLLVMType(resultType));
		bindLocal(target, state, e);
	}
}

void Executor::threadexecutecall(ExecutionState &state, Thread & thread, mpi_rank_t rank, Function *f,
		std::vector<ref<Expr> > &arguments, bool isfirstone) {
	if (f && DebugCallHistory) {
		unsigned depth = state.stack().size();

		LOG(INFO)<< "Call[" << &state << "]: " << std::string(depth, ' ') << f->getName().str();
	}
	assert(f && !thread.enabled && "can not execute NULL call or push frame on a stack of an active thread.");

	/**
	 * At here we check the LTL property
	 */
	if (LTLFile.size() > 0 && property) {
        if (checkLTLViolation(*property, state, rank, f, arguments) == false) {
            LOG(INFO) << "MPI-SV finds a violation of the LTL property at symbolic execution and exits. \n";
            /// change the iterations
            theStatisticManager->iteration++;
            /// change the LTL violation iterations
            theStatisticManager->iterationForLTLViolation = theStatisticManager->iteration;
            /// change the PC container
            string res;
            std::ostringstream info;
            ExprPPrinter::printConstraints(info, state.constraints());
            res = info.str();
            (theStatisticManager->pcContainer).insert(res);
            /// change each state to be delayed
            std::set<ExecutionState*>::iterator it = (state.executor->states).begin();
            while (it != (state.executor->states).end()) {
                (*it)->delayed=true;
                it++;
            }
            terminateStateOnError(state, "The LTL property is violated", "user.err");
            return;
        }
	}


    KFunction *kf = kmodule->functionMap[f];
	KInstIterator returnpoint = isfirstone?thread.prevPC:thread.stack.back().kf->instructions;
	state.pushFrame(thread,returnpoint, kf);
	//state.pushFrame(thread,thread.prevPC, kf);
	if(!isfirstone) {
		//Is this enough to identify a call ? <callerinst*, f*>, the former is it's caller, latter
		// being the called function.
		state.globalCommLog.dirtycalls[make_pair(thread.stack.back().caller.hash(),thread.stack.back().kf)]=1;
		//state.globalCommLog.dirtycalls[make_pair(thread.stack.back().callPathNode->callSite,f)]=true;

#if 1
	LOG(INFO)<<"marking state/thread "<< &state <<" "<<&thread <<" sf of function "<<f->getName().str()<< " as dirty";// pair: "<<thread.stack.back().caller.hash() <<" "<<thread.stack.back().kf;
#endif
	}
	else {
		state.globalCommLog.dirtycalls[make_pair(thread.stack.back().caller.hash(),thread.stack.back().kf)]=2;
	}

	thread.pc = kf->instructions;
	if (statsTracker)
	statsTracker->framePushed(&(thread.stack.back()), &(thread.stack[thread.stack.size()-2]));
	//statsTracker->framePushed(state, &state.stack()[state.stack().size()-2]);

	unsigned callingArgs = arguments.size();
	unsigned funcArgs = f->arg_size();
	if (!f->isVarArg()) {
		if (callingArgs > funcArgs) {
			LOG(WARNING) << "Calling " << f->getName().data() << " with extra arguments.";
		} else if (callingArgs < funcArgs) {
			terminateStateOnError(state, "calling function with too few arguments",
					"user.err");
			return;
		}
	}
	if (callingArgs < funcArgs) {
		terminateStateOnError(state, "calling function with too few arguments",
				"user.err");
		return;
	}

	StackFrame &sf = thread.stack.back();
	unsigned size = 0;
	for (unsigned i = funcArgs; i < callingArgs; i++) {
		// FIXME: This is really specific to the architecture, not the pointer
		// size. This happens to work fir x86-32 and x86-64, however.
		Expr::Width WordSize = Context::get().getPointerWidth();
		if (WordSize == Expr::Int32) {
			size += Expr::getMinBytesForWidth(arguments[i]->getWidth());
		} else {
			size += llvm::RoundUpToAlignment(arguments[i]->getWidth(),
					WordSize) / 8;
		}
	}

	MemoryObject *mo = sf.varargs = memory->allocate(&state, size, true, false,
			state.prevPC()->inst);
	if (!mo) {
		terminateStateOnExecError(state, "out of memory (varargs)");
		return;
	}
	ObjectState *os = bindObjectInState(state, mo, true);
	unsigned offset = 0;
	for (unsigned i = funcArgs; i < callingArgs; i++) {
		// FIXME: This is really specific to the architecture, not the pointer
		// size. This happens to work fir x86-32 and x86-64, however.
		Expr::Width WordSize = Context::get().getPointerWidth();
		if (WordSize == Expr::Int32) {
			os->write(offset, arguments[i]);
			offset += Expr::getMinBytesForWidth(arguments[i]->getWidth());
		} else {
			assert(WordSize == Expr::Int64 && "Unknown word size!");
			os->write(offset, arguments[i]);
			offset += llvm::RoundUpToAlignment(arguments[i]->getWidth(),
					WordSize) / 8;
		}
	}

	unsigned numFormals = f->arg_size();
	for (unsigned i=0; i<numFormals; ++i) {
		//bindArgument(kf, i, state, arguments[i]);
		getArgumentCell(thread.stack.back(), kf, i).value = arguments[i];
	}
	/*LOG(INFO)<<"pid/tid:"<<thread.tuid.second<<"/"<<thread.tuid.first<<"threadexecutecall begin";
	 LOG(INFO)<<"func: "<<kf->function->getName().str()<< " arg: "<< arguments[0];
	 LOG(INFO)<<"func: "<<thread.stack[thread.stack.size()-2].kf->function->getName().str()<<" arg: "<<
	 (thread.stack[thread.stack.size()-2].locals[thread.stack[thread.stack.size()-2].kf->getArgRegister(0)].value);
	 LOG(INFO)<<"pid/tid:"<<thread.tuid.second<<"/"<<thread.tuid.first<<"threadexecutecall end";

	 if(thread.tuid.second == 2 && thread.tuid.first ==5) {
	 state.getStackTrace().dump(LOG(INFO));
	 }*/
}

const llvm::Instruction * Executor::getInsofFunction(ExecutionState &state, Thread & thread,
		Function *f) {
	int depth;
	std::vector<StackFrame>::iterator it;
	for (it = thread.stack.begin(), depth = 0; it != thread.stack.end(); it++, depth++) {
		if (it->kf->function == f)
			return it->callPathNode->callSite;
	}

	return NULL;
}

}

