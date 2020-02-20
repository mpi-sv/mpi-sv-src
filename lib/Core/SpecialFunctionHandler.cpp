//===-- SpecialFunctionHandler.cpp ----------------------------------------===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Memory.h"
#include "SpecialFunctionHandler.h"
#include "TimingSolver.h"

#include "klee/ExecutionState.h"
#include "klee/util/ExprPPrinter.h"
#include "klee/data/ExprSerializer.h"
#include "klee/Statistics.h"

#include "klee/Internal/Module/KInstruction.h"
#include "klee/Internal/Module/InstructionInfoTable.h"
#include "klee/Internal/Module/KModule.h"

#include "klee/Executor.h"
#include "MemoryManager.h"

#include "cloud9/worker/WorkerCommon.h"

#include "klee/Interpreter.h" //added yhb

#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Type.h"
#include "llvm/DerivedTypes.h"
#include "llvm/InstrTypes.h"
#include "llvm/LLVMContext.h"
#include "llvm/Support/CommandLine.h"//Herman
#include "mpise/commlog.h"//Added by herman.
#include "mpise/generate.h"
#include "mpise/analysis.h"
#include "mpise/prepare.h"
#include <glog/logging.h>

#include <errno.h>
#include <stdarg.h>
#include <sys/syscall.h>
#include <execinfo.h>

using namespace llvm;
using namespace klee;
using namespace std;

/// \todo Almost all of the demands in this file should be replaced
/// with terminateState calls.

///
static Instruction *aaa;

struct HandlerInfo {
	const char *name;
	SpecialFunctionHandler::Handler handler;
	bool doesNotReturn; /// Intrinsic terminates the process
	bool hasReturnValue; /// Intrinsic has a return value
	bool doNotOverride; /// Intrinsic should not be used if already defined
};

// FIXME: We are more or less committed to requiring an intrinsic
// library these days. We can move some of this stuff there,
// especially things like realloc which have complicated semantics
// w.r.t. forking. Among other things this makes delayed query
// dispatch easier to implement.
HandlerInfo handlerInfo[] = {
#define add(name, handler, ret) { name, \
                                  &SpecialFunctionHandler::handler, \
                                  false, ret, false }
#define addDNR(name, handler) { name, \
                                &SpecialFunctionHandler::handler, \
                                true, false, false }
		addDNR("__assert_rtn", handleAssertFail), addDNR("__assert_fail", handleAssertFail),
				addDNR("_assert", handleAssert), addDNR("abort", handleAbort),
				addDNR("klee_abort", handleAbort), addDNR("klee_silent_exit", handleSilentExit),
				addDNR("klee_report_error", handleReportError),
				addDNR("klee_thread_terminate", handleThreadTerminate),
				addDNR("klee_process_terminate", handleProcessTerminate),

		add("calloc", handleCalloc, true), add("free", handleFree, false),
				add("klee_assume", handleAssume, false),
#if 0
		add("klee_shadow_check", handleShadowCheck, false),
#endif
		add("klee_record_expr", handleRecordExpr, false), add("klee_event", handleEvent, false),
				add("klee_check_memory_access", handleCheckMemoryAccess, false),
				add("klee_get_valuef", handleGetValue, true),
				add("klee_get_valued", handleGetValue, true),
				add("klee_get_valuel", handleGetValue, true),
				add("klee_get_valuell", handleGetValue, true),
				add("klee_get_value_i32", handleGetValue, true),
				add("klee_get_value_i64", handleGetValue, true),
				add("klee_define_fixed_object", handleDefineFixedObject, false),
				add("klee_get_obj_size", handleGetObjSize, true),
				add("klee_get_errno", handleGetErrno, true),
				add("klee_is_symbolic", handleIsSymbolic, true),
				add("klee_make_symbolic", handleMakeSymbolic, false),
				add("klee_mark_global", handleMarkGlobal, false),
				add("klee_merge", handleMerge, false),
				add("klee_prefer_cex", handlePreferCex, false),
				add("klee_print_expr", handlePrintExpr, false),
				add("klee_print_range", handlePrintRange, false),
				add("klee_set_forking", handleSetForking, false),
				add("klee_stack_trace", handleStackTrace, false),
				add("klee_make_shared", handleMakeShared, false),
				add("klee_get_context", handleGetContext, false),
				add("klee_get_wlist", handleGetWList, true),
				add("klee_thread_preempt", handleThreadPreempt, false),
				add("klee_thread_sleep", handleThreadSleep, false),
				add("klee_thread_notify", handleThreadNotify, false),
				add("klee_warning", handleWarning, false),
				add("klee_warning_once", handleWarningOnce, false),
				add("klee_alias_function", handleAliasFunction, false),

		add("klee_thread_create", handleThreadCreate, false),
				add("klee_process_fork", handleProcessFork, true),

		add("klee_branch", handleBranch, true), add("klee_fork", handleFork, true),

		add("klee_debug", handleDebug, false),

		add("klee_get_time", handleGetTime, true), add("klee_set_time", handleSetTime, false),

		add("klee_begin_checked", handleBeginChecked, false),
				add("klee_end_checked", handleEndChecked, false),
				add("klee_memcmp", handleMemCmp, true),

		add("malloc", handleMalloc, true), add("realloc", handleRealloc, true),
				add("valloc", handleValloc, true),

		// operator delete[](void*)
		add("_ZdaPv", handleDeleteArray, false),
		// operator delete(void*)
		add("_ZdlPv", handleDelete, false),

		// operator new[](unsigned int)
		add("_Znaj", handleNewArray, true),
		// operator new(unsigned int)
		add("_Znwj", handleNew, true), add("_ZnamRKSt9nothrow_t", handleNew, true),

		// FIXME-64: This is wrong for 64-bit long...

		// operator new[](unsigned long)
		add("_Znam", handleNewArray, true),
		// operator new(unsigned long)
		add("_Znwm", handleNew, true), add("_ZnwmRKSt9nothrow_t", handleNew, true),

		add("syscall", handleSyscall, true),
		// MPI communicate handler
		add("klee_mpi_comm", handleMPIcomm,true),
				add("klee_mpi_nonblock",handleMPInonblock,true),
				add("klee_get_anysource_candi",handleGetAnysrcCandi,true),
				add("klee_disable_sync_chk",handleDisableSyncChk,true),
				add("klee_enable_sync_chk",handleEnableSyncChk,true),
				add("klee_fire_mpi7para",handleFireMPI7para,true),
				add("klee_fire_mpi2para",handleFireMPI2para,true),
				add("klee_fire_mpi1para",handleFireMPI1para,true),
				add("klee_fire_mpi0para",handleFireMPI0para,true),
				//added by yhb, klee_fire_slave
				add("klee_fire_slave",handleFireMPISlave,true),
				add("klee_set_mpi_ndcnt",handleSetNodeCount,true),
				add("klee_keep_block",handleKeepBlock,true)

#undef addDNR
#undef add  
		};

extern cl::opt<bool> SyncOpt;
SpecialFunctionHandler::SpecialFunctionHandler(Executor &_executor) :
		executor(_executor) {
}

void SpecialFunctionHandler::prepare() {
	unsigned N = sizeof(handlerInfo) / sizeof(handlerInfo[0]);

	for (unsigned i = 0; i < N; ++i) {
		HandlerInfo &hi = handlerInfo[i];
		Function *f = executor.kmodule->module->getFunction(hi.name);

		// No need to create if the function doesn't exist, since it cannot
		// be called in that case.

		if (f && (!hi.doNotOverride || f->isDeclaration())) {
			// Make sure NoReturn attribute is set, for optimization and
			// coverage counting.
			if (hi.doesNotReturn)
				f->addFnAttr(Attribute::NoReturn);

			// Change to a declaration since we handle internally (simplifies
			// module and allows deleting dead code).
			if (!f->isDeclaration())
				f->deleteBody();
		}
	}
}

void SpecialFunctionHandler::bind() {
	unsigned N = sizeof(handlerInfo) / sizeof(handlerInfo[0]);

	for (unsigned i = 0; i < N; ++i) {
		HandlerInfo &hi = handlerInfo[i];
		Function *f = executor.kmodule->module->getFunction(hi.name);

		if (f && (!hi.doNotOverride || f->isDeclaration()))
			handlers[f] = std::make_pair(hi.handler, hi.hasReturnValue);
	}
}

bool SpecialFunctionHandler::handle(ExecutionState &state, Function *f, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	handlers_ty::iterator it = handlers.find(f);
	if (it != handlers.end()) {
		Handler h = it->second.first;
		bool hasReturnValue = it->second.second;
		// FIXME: Check this... add test?
		if (!hasReturnValue && !target->inst->use_empty()) {
			executor.terminateStateOnExecError(state,
					"expected return value from void special function");
		} else {
			(this->*h)(state, target, arguments);
		}
		return true;
	} else {
		return false;
	}
}

void SpecialFunctionHandler::processMemoryLocation(ExecutionState &state, ref<Expr> address,
		ref<Expr> size, const std::string &name, resolutions_ty &resList) {
	Executor::ExactResolutionList rl;
	executor.resolveExact(state, address, rl, name);

	for (Executor::ExactResolutionList::iterator it = rl.begin(), ie = rl.end(); it != ie; ++it) {
		const MemoryObject *mo = it->first.first;
		const ObjectState *os = it->first.second;
		ExecutionState *s = it->second;

		// FIXME: Type coercion should be done consistently somewhere.
		bool res;
		bool success = executor.solver->mustBeTrue(data::OTHER, *s,
				EqExpr::create(ZExtExpr::create(size, Context::get().getPointerWidth()),
						mo->getSizeExpr()), res);
		assert(success && "FIXME: Unhandled solver failure");

		if (res) {
			resList.push_back(std::make_pair(std::make_pair(mo, os), s));
		} else {
			executor.terminateStateOnError(*s, "wrong size given to memory operation", "user.err");
		}
	}
}

bool SpecialFunctionHandler::writeConcreteValue(ExecutionState &state, ref<Expr> address,
		uint64_t value, Expr::Width width) {
	ObjectPair op;

	if (!state.addressSpace().resolveOne(cast<ConstantExpr>(address), op)) {
		executor.terminateStateOnError(state, "invalid pointer for writing concrete value into",
				"user.err");
		return false;
	}

	ObjectState *os = state.addressSpace().getWriteable(op.first, op.second);

	os->write(op.first->getOffsetExpr(address), ConstantExpr::create(value, width));

	return true;
}

/****/

// reads a concrete string from memory
std::string SpecialFunctionHandler::readStringAtAddress(ExecutionState &state,
		ref<Expr> addressExpr) {
	ObjectPair op;
	addressExpr = executor.toUnique(state, addressExpr);
	if (!isa<ConstantExpr>(addressExpr))
		return std::string("<KLEE<symaddr>>");

	ref<ConstantExpr> address = cast<ConstantExpr>(addressExpr);
	if (!state.addressSpace().resolveOne(address, op))
		return std::string("<KLEE<multires>>");

	const MemoryObject *mo = op.first;
	const ObjectState *os = op.second;

	char *buf = new char[mo->size];
	unsigned ioffset = 0;

	ref<Expr> offset_expr = SubExpr::create(address, op.first->getBaseExpr());
	if (isa<ConstantExpr>(offset_expr)) {
		ref<ConstantExpr> value = cast<ConstantExpr>(offset_expr.get());
		ioffset = value.get()->getZExtValue();
	} else
		return std::string("<KLEE<invalstring>>");

	assert(ioffset < mo->size);

	unsigned i;
	for (i = 0; i < mo->size - ioffset - 1; i++) {
		ref<Expr> cur = os->read8(i + ioffset);
		cur = executor.toUnique(state, cur);
		if (!isa<ConstantExpr>(cur)) //XXX: Should actually concretize the value...
			return std::string("<KLEE<symchar>>");
		buf[i] = cast<ConstantExpr>(cur)->getZExtValue(8);
	}
	buf[i] = 0;

	std::string result(buf);
	delete[] buf;
	return result;
}

/****/

void SpecialFunctionHandler::handleAbort(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(arguments.size() == 0 && "invalid number of arguments to abort");

	executor.terminateStateOnError(state, "abort failure", "abort.err");
}

void SpecialFunctionHandler::handleSilentExit(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(arguments.size() == 1 && "invalid number of arguments to exit");

	executor.terminateState(state, true);
}

void SpecialFunctionHandler::handleAliasFunction(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(arguments.size() == 2 && "invalid number of arguments to klee_alias_function");
	std::string old_fn = readStringAtAddress(state, arguments[0]);
	std::string new_fn = readStringAtAddress(state, arguments[1]);
	//std::cerr << "Replacing " << old_fn << "() with " << new_fn << "()\n";
	if (old_fn == new_fn)
		state.removeFnAlias(old_fn);
	else
		state.addFnAlias(old_fn, new_fn);
}

void SpecialFunctionHandler::handleAssert(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(arguments.size() == 3 && "invalid number of arguments to _assert");

	executor.terminateStateOnError(state,
			"ASSERTION FAIL: " + readStringAtAddress(state, arguments[0]), "assert.err");
}

void SpecialFunctionHandler::handleAssertFail(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(arguments.size() == 4 && "invalid number of arguments to __assert_fail");

	executor.terminateStateOnError(state,
			"ASSERTION FAIL: " + readStringAtAddress(state, arguments[0]), "assert.err");
}

void SpecialFunctionHandler::handleReportError(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(arguments.size() == 4 && "invalid number of arguments to klee_report_error");

	// arguments[0], arguments[1] are file, line

	executor.terminateStateOnError(state, readStringAtAddress(state, arguments[2]),
			readStringAtAddress(state, arguments[3]).c_str());
}

void SpecialFunctionHandler::handleMerge(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	// nop
}

void SpecialFunctionHandler::handleNew(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	// XXX should type check args
	assert(arguments.size() >= 1 && "invalid number of arguments to new");

	executor.executeAlloc(state, arguments[0], false, target);
}

void SpecialFunctionHandler::handleDelete(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	// FIXME: Should check proper pairing with allocation type (malloc/free,
	// new/delete, new[]/delete[]).

	// XXX should type check args
	assert(arguments.size() == 1 && "invalid number of arguments to delete");
	executor.executeFree(state, arguments[0]);
}

void SpecialFunctionHandler::handleNewArray(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	// XXX should type check args
	assert(arguments.size() == 1 && "invalid number of arguments to new[]");
	executor.executeAlloc(state, arguments[0], false, target);
}

void SpecialFunctionHandler::handleDeleteArray(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	// XXX should type check args
	assert(arguments.size() == 1 && "invalid number of arguments to delete[]");
	executor.executeFree(state, arguments[0]);
}

void SpecialFunctionHandler::handleMalloc(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	// XXX should type check args
	assert(arguments.size() == 1 && "invalid number of arguments to malloc");
	executor.executeAlloc(state, arguments[0], false, target);
}

void SpecialFunctionHandler::handleValloc(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {

	// XXX ignoring for now the "multiple of page size " requirement
	//- executing the regular alloc
	// XXX should type check args
	assert(arguments.size() == 1 && "invalid number of arguments to valloc");
	executor.executeAlloc(state, arguments[0], false, target);
}

void SpecialFunctionHandler::handleAssume(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(arguments.size() == 1 && "invalid number of arguments to klee_assume");

	ref<Expr> e = arguments[0];

	if (e->getWidth() != Expr::Bool)
		e = NeExpr::create(e, ConstantExpr::create(0, e->getWidth()));

	bool res;
	bool success = executor.solver->mustBeFalse(data::CHECK_ASSUMPTION, state, e, res);
	assert(success && "FIXME: Unhandled solver failure");
	if (res) {
		executor.terminateStateOnError(state, "invalid klee_assume call (provably false)",
				"user.err");
	} else {
		executor.addConstraint(state, e);
	}
}

void SpecialFunctionHandler::handleIsSymbolic(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(arguments.size() == 1 && "invalid number of arguments to klee_is_symbolic");

	//arguments[0]->print(std::cerr);

	executor.bindLocal(target, state,
			ConstantExpr::create(!isa<ConstantExpr>(arguments[0]), Expr::Int32));
}

void SpecialFunctionHandler::handlePreferCex(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(arguments.size() == 2 && "invalid number of arguments to klee_prefex_cex");

	ref<Expr> cond = arguments[1];
	if (cond->getWidth() != Expr::Bool)
		cond = NeExpr::create(cond, ConstantExpr::alloc(0, cond->getWidth()));

	Executor::ExactResolutionList rl;
	executor.resolveExact(state, arguments[0], rl, "prefex_cex");

	assert(rl.size() == 1 && "prefer_cex target must resolve to precisely one object");

	rl[0].first.first->cexPreferences.push_back(cond);
}

void SpecialFunctionHandler::handlePrintExpr(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(arguments.size() == 2 && "invalid number of arguments to klee_print_expr");

	std::string msg_str = readStringAtAddress(state, arguments[0]);
	std::cerr << msg_str << ":" << arguments[1] << std::endl;

	for (ConstraintManager::constraint_iterator it = state.constraints().begin();
			it != state.constraints().end(); it++) {
		std::cerr << *it << std::endl;
	}
}

void SpecialFunctionHandler::handleSetForking(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(arguments.size() == 1 && "invalid number of arguments to klee_set_forking");
	ref<Expr> value = executor.toUnique(state, arguments[0]);

	if (ConstantExpr *CE = dyn_cast<ConstantExpr>(value)) {
		state.forkDisabled = CE->isZero();
	} else {
		executor.terminateStateOnError(state, "klee_set_forking requires a constant arg",
				"user.err");
	}
}

void SpecialFunctionHandler::handleStackTrace(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	state.getStackTrace().dump(std::cout);
}

void SpecialFunctionHandler::handleWarning(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(arguments.size() == 1 && "invalid number of arguments to klee_warning");

	std::string msg_str = readStringAtAddress(state, arguments[0]);
	LOG(WARNING)<< state.stack().back().kf->function->getName().data() << ": "
	<< msg_str.c_str();
}

void SpecialFunctionHandler::handleDebug(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(arguments.size() >= 1 && "invalid number of arguments to klee_debug");

	std::string formatStr = readStringAtAddress(state, arguments[0]);

	// XXX Ugly hack, need to use libffi here... Ashamed of myself

	if (arguments.size() == 2 && arguments[1]->getWidth() == sizeof(long) * 8) {
		// Special case for displaying strings

		std::string paramStr = readStringAtAddress(state, arguments[1]);

		fprintf(stderr, formatStr.c_str(), paramStr.c_str());
		return;
	}

	std::vector<int> args;

	for (unsigned int i = 1; i < arguments.size(); i++) {
		if (!isa<ConstantExpr>(arguments[i])) {
			fprintf(stderr, "%s: %s\n", formatStr.c_str(), "<nonconst args>");
			return;
		}

		ref<ConstantExpr> arg = cast<ConstantExpr>(arguments[i]);

		if (arg->getWidth() != sizeof(int) * 8) {
			fprintf(stderr, "%s: %s\n", formatStr.c_str(), "<non-32-bit args>");
			return;
		}

		args.push_back((int) arg->getZExtValue());
	}

	switch (args.size()) {
	case 0:
		fprintf(stderr, "%s", formatStr.c_str());
		break;
	case 1:
		fprintf(stderr, formatStr.c_str(), args[0]);
		break;
	case 2:
		fprintf(stderr, formatStr.c_str(), args[0], args[1]);
		break;
	case 3:
		fprintf(stderr, formatStr.c_str(), args[0], args[1], args[2]);
		break;
	default:
		executor.terminateStateOnError(state, "klee_debug allows up to 3 arguments", "user.err");
		return;
	}
}

void SpecialFunctionHandler::handleWarningOnce(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(arguments.size() == 1 && "invalid number of arguments to klee_warning_once");

	std::string msg_str = readStringAtAddress(state, arguments[0]);
	LOG_EVERY_N(WARNING, 1)
	<< state.stack().back().kf->function->getName().data() << ": " << msg_str.c_str();
}

void SpecialFunctionHandler::handlePrintRange(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(arguments.size() == 2 && "invalid number of arguments to klee_print_range");

	std::string msg_str = readStringAtAddress(state, arguments[0]);
	std::cerr << msg_str << ":" << arguments[1];
	if (!isa<ConstantExpr>(arguments[1])) {
		// FIXME: Pull into a unique value method?
		ref<ConstantExpr> value;
		bool success = executor.solver->getValue(data::OTHER, state, arguments[1], value);
		assert(success && "FIXME: Unhandled solver failure");
		bool res;
		success = executor.solver->mustBeTrue(data::OTHER, state,
				EqExpr::create(arguments[1], value), res);
		assert(success && "FIXME: Unhandled solver failure");
		if (res) {
			std::cerr << " == " << value;
		} else {
			std::cerr << " ~= " << value;
			std::pair<ref<Expr>, ref<Expr> > res = executor.solver->getRange(data::OTHER, state,
					arguments[1]);
			std::cerr << " (in [" << res.first << ", " << res.second << "])";
		}
	}
	std::cerr << "\n";
}

void SpecialFunctionHandler::handleGetObjSize(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	// XXX should type check args
	assert(arguments.size() == 1 && "invalid number of arguments to klee_get_obj_size");
	Executor::ExactResolutionList rl;
	executor.resolveExact(state, arguments[0], rl, "klee_get_obj_size");
	for (Executor::ExactResolutionList::iterator it = rl.begin(), ie = rl.end(); it != ie; ++it) {
		executor.bindLocal(target, *it->second,
				ConstantExpr::create(it->first.first->size, Expr::Int32));
	}
}

void SpecialFunctionHandler::handleGetErrno(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	// XXX should type check args
	assert(arguments.size() == 0 && "invalid number of arguments to klee_get_obj_size");
	executor.bindLocal(target, state, ConstantExpr::create(errno, Expr::Int32));
}

void SpecialFunctionHandler::handleCalloc(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	// XXX should type check args
	assert(arguments.size() == 2 && "invalid number of arguments to calloc");

	ref<Expr> size = MulExpr::create(arguments[0], arguments[1]);
	executor.executeAlloc(state, size, false, target, true);
}

void SpecialFunctionHandler::handleRealloc(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	// XXX should type check args
	assert(arguments.size() == 2 && "invalid number of arguments to realloc");
	ref<Expr> address = arguments[0];
	ref<Expr> size = arguments[1];

	Executor::StatePair zeroSize = executor.fork(state, Expr::createIsZero(size), true,
			KLEE_FORK_INTERNAL);

	if (zeroSize.first) { // size == 0
		executor.executeFree(*zeroSize.first, address, target);
	}
	if (zeroSize.second) { // size != 0
		Executor::StatePair zeroPointer = executor.fork(*zeroSize.second,
				Expr::createIsZero(address), true, KLEE_FORK_INTERNAL);

		if (zeroPointer.first) { // address == 0
			executor.executeAlloc(*zeroPointer.first, size, false, target);
		}
		if (zeroPointer.second) { // address != 0
			Executor::ExactResolutionList rl;
			executor.resolveExact(*zeroPointer.second, address, rl, "realloc");

			for (Executor::ExactResolutionList::iterator it = rl.begin(), ie = rl.end(); it != ie;
					++it) {
				executor.executeAlloc(*it->second, size, false, target, false, it->first.second);
			}
		}
	}
}

void SpecialFunctionHandler::handleFree(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	// XXX should type check args
	assert(arguments.size() == 1 && "invalid number of arguments to free");
	executor.executeFree(state, arguments[0]);
}

void SpecialFunctionHandler::handleMakeShared(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {

	assert(arguments.size() == 2 && "invalid number of arguments to klee_make_shared");

	resolutions_ty resList;

	processMemoryLocation(state, arguments[0], arguments[1], "make_shared", resList);

	for (resolutions_ty::iterator it = resList.begin(); it != resList.end(); it++) {
		const MemoryObject *mo = it->first.first;
		const ObjectState *os = it->first.second;
		ExecutionState *s = it->second;

		if (mo->isLocal) {
			executor.terminateStateOnError(*s, "cannot share local object", "user.err");
			continue;
		}

		unsigned int bindCount = 0;
		for (ExecutionState::processes_ty::iterator pit = s->processes.begin();
				pit != s->processes.end(); pit++) {
			if (pit->second.addressSpace.findObject(mo) != NULL)
				bindCount++;
		}

		if (bindCount != 1) {
			executor.terminateStateOnError(*s, "cannot shared already forked object", "user.err");
			continue;
		}

		ObjectState *newOS = state.addressSpace().getWriteable(mo, os);
		newOS->isShared = true;

		// Now bind this object in the other address spaces
		for (ExecutionState::processes_ty::iterator pit = s->processes.begin();
				pit != s->processes.end(); pit++) {
			if (pit == s->crtProcessIt)
				continue; // Skip the current process

			pit->second.addressSpace.bindSharedObject(mo, newOS);
		}
	}
}

void SpecialFunctionHandler::handleGetContext(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(arguments.size() == 2 && "invalid number of arguments to klee_get_context");

	ref<Expr> tidAddr = executor.toUnique(state, arguments[0]);
	ref<Expr> pidAddr = executor.toUnique(state, arguments[1]);

	if (!isa<ConstantExpr>(tidAddr) || !isa<ConstantExpr>(pidAddr)) {
		executor.terminateStateOnError(state, "klee_get_context requires constant args",
				"user.err");
		return;
	}

	if (!tidAddr->isZero()) {
		if (!writeConcreteValue(state, tidAddr, state.crtThread().getTid(),
				executor.getWidthForLLVMType(Type::getInt64Ty(getGlobalContext()))))
			return;
	}

	if (!pidAddr->isZero()) {
		if (!writeConcreteValue(state, pidAddr, state.crtProcess().pid,
				executor.getWidthForLLVMType(Type::getInt32Ty(getGlobalContext()))))
			return;
	}
}

void SpecialFunctionHandler::handleGetTime(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(arguments.empty() && "invalid number of arguments to klee_get_time");

	executor.bindLocal(target, state,
			ConstantExpr::create(state.stateTime,
					executor.getWidthForLLVMType(target->inst->getType())));
}

void SpecialFunctionHandler::handleSetTime(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(arguments.size() == 1 && "invalid number of arguments to klee_set_time");

	if (!isa<ConstantExpr>(arguments[0])) {
		executor.terminateStateOnError(state, "klee_set_time requires a constant argument",
				"user.err");
		return;
	}

	state.stateTime = cast<ConstantExpr>(arguments[0])->getZExtValue();
}

void SpecialFunctionHandler::handleGetWList(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(arguments.empty() && "invalid number of arguments to klee_get_wlist");

	wlist_id_t id = state.getWaitingList();

	executor.bindLocal(target, state,
			ConstantExpr::create(id, executor.getWidthForLLVMType(target->inst->getType())));
}

void SpecialFunctionHandler::handleThreadPreempt(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(arguments.size() == 1 && "invalid number of arguments to klee_thread_preempt");

	if (!isa<ConstantExpr>(arguments[0])) {
		executor.terminateStateOnError(state, "klee_thread_preempt", "user.err");
	}

	executor.schedule(state, !arguments[0]->isZero());
}

void SpecialFunctionHandler::handleThreadSleep(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {

	assert(arguments.size() == 1 && "invalid number of arguments to klee_thread_sleep");

	ref<Expr> wlistExpr = executor.toUnique(state, arguments[0]);

	if (!isa<ConstantExpr>(wlistExpr)) {
		executor.terminateStateOnError(state, "klee_thread_sleep", "user.err");
		return;
	}

	state.sleepThread(cast<ConstantExpr>(wlistExpr)->getZExtValue());
	executor.schedule(state, false);
}

void SpecialFunctionHandler::handleThreadNotify(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(arguments.size() == 2 && "invalid number of arguments to klee_thread_notify");

	ref<Expr> wlist = executor.toUnique(state, arguments[0]);
	ref<Expr> all = executor.toUnique(state, arguments[1]);

	if (!isa<ConstantExpr>(wlist) || !isa<ConstantExpr>(all)) {
		executor.terminateStateOnError(state, "klee_thread_notify", "user.err");
		return;
	}

	if (all->isZero()) {
		executor.executeThreadNotifyOne(state, cast<ConstantExpr>(wlist)->getZExtValue());
	} else {
		// It's simple enough such that it can be handled by the state class itself
		state.notifyAll(cast<ConstantExpr>(wlist)->getZExtValue());
	}
}

void SpecialFunctionHandler::handleThreadCreate(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(arguments.size() == 3 && "invalid number of arguments to klee_thread_create");

	ref<Expr> tid = executor.toUnique(state, arguments[0]);
	//Herman added
	//LOG(INFO)<<"handleThreadCreate:"<<cast<ConstantExpr>(arguments[0])->getZExtValue()<<"\t"<<cast<ConstantExpr>(tid)->getZExtValue()<<"\n";
	if (!isa<ConstantExpr>(tid)) {
		executor.terminateStateOnError(state, "klee_thread_create", "user.err");
		return;
	}

	executor.executeThreadCreate(state, cast<ConstantExpr>(tid)->getZExtValue(), arguments[1],
			arguments[2]);
}

void SpecialFunctionHandler::handleThreadTerminate(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(arguments.empty() && "invalid number of arguments to klee_thread_terminate");

	executor.executeThreadExit(state);
}

void SpecialFunctionHandler::handleBranch(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(arguments.size() == 2 && "invalid number of arguments to klee_branch");

	if (!isa<ConstantExpr>(arguments[1])) {
		executor.terminateStateOnError(state, "symbolic reason in klee_branch", "user.err");
		return;
	}

	// We must check that klee_branch is correctly used - the use case of the
	// return value must be a comparison instruction
	Instruction *inst = target->inst;

	if (!inst->hasOneUse()) {
		executor.terminateStateOnError(state, "klee_branch must be used once", "user.err");
		return;
	}

	User *user = *inst->use_begin();

	if (!isa<CmpInst>(user) || inst->getParent() != cast<Instruction>(user)->getParent()) {
		executor.terminateStateOnError(state, "klee_branch must be used together with a comparison",
				"user.err");
		return;
	}

	// We just bind the result to the first argument, and mark the reason

	state.crtForkReason = cast<ConstantExpr>(arguments[1])->getZExtValue();
	state.crtSpecialFork = cast<Instruction>(user);

	executor.bindLocal(target, state, arguments[0]);
}

void SpecialFunctionHandler::handleFork(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(arguments.size() == 1 && "invalid number of arguments to klee_fork");

	if (!isa<ConstantExpr>(arguments[0])) {
		executor.terminateStateOnError(state, "symbolic reason in klee_fork", "user.err");
		return;
	}

	uint64_t reason = cast<ConstantExpr>(arguments[0])->getZExtValue();

	executor.executeFork(state, target, reason);
}

void SpecialFunctionHandler::handleProcessFork(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(arguments.size() == 1 && "invalid number of arguments to klee_process_fork");

	ref<Expr> pid = executor.toUnique(state, arguments[0]);

	if (!isa<ConstantExpr>(pid)) {
		executor.terminateStateOnError(state, "klee_process_fork", "user.err");
		return;
	}

	executor.executeProcessFork(state, target, cast<ConstantExpr>(pid)->getZExtValue());
}

void SpecialFunctionHandler::handleProcessTerminate(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(arguments.empty() && "invalid number of arguments to klee_process_terminate");

	executor.executeProcessExit(state);
}

void SpecialFunctionHandler::handleCheckMemoryAccess(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(arguments.size() == 2 && "invalid number of arguments to klee_check_memory_access");

	ref<Expr> address = executor.toUnique(state, arguments[0]);
	ref<Expr> size = executor.toUnique(state, arguments[1]);
	if (!isa<ConstantExpr>(address) || !isa<ConstantExpr>(size)) {
		executor.terminateStateOnError(state, "check_memory_access requires constant args",
				"user.err");
	} else {
		ObjectPair op;

		if (!state.addressSpace().resolveOne(cast<ConstantExpr>(address), op)) {
			executor.terminateStateOnError(state, "check_memory_access: memory error", "ptr.err",
					executor.getAddressInfo(state, address));
		} else {
			ref<Expr> chk = op.first->getBoundsCheckPointer(address,
					cast<ConstantExpr>(size)->getZExtValue());
			if (!chk->isTrue()) {
				executor.terminateStateOnError(state, "check_memory_access: memory error",
						"ptr.err", executor.getAddressInfo(state, address));
			}
		}
	}
}

void SpecialFunctionHandler::handleGetValue(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(arguments.size() == 1 && "invalid number of arguments to klee_get_value");

	executor.executeGetValue(state, arguments[0], target);
}

void SpecialFunctionHandler::handleDefineFixedObject(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(arguments.size() == 2 && "invalid number of arguments to klee_define_fixed_object");
	assert(
			isa<ConstantExpr>(arguments[0])
					&& "expect constant address argument to klee_define_fixed_object");
	assert(
			isa<ConstantExpr>(arguments[1])
					&& "expect constant size argument to klee_define_fixed_object");

	uint64_t address = cast<ConstantExpr>(arguments[0])->getZExtValue();
	uint64_t size = cast<ConstantExpr>(arguments[1])->getZExtValue();
	MemoryObject *mo = executor.memory->allocateFixed(address, size, state.prevPC()->inst);
	executor.bindObjectInState(state, mo, false);
	mo->isUserSpecified = true; // XXX hack;
}

void SpecialFunctionHandler::makeSymbolic(ExecutionState &state,
		std::vector<ref<Expr> > &arguments) {
	std::string name;

	// FIXME: For backwards compatibility, we should eventually enforce the
	// correct arguments.
	if (arguments.size() == 2) {
		name = "unnamed";
	} else {
		// FIXME: Should be a user.err, not an assert.
		assert(arguments.size() == 3 && "invalid number of arguments to klee_make_symbolic");
		name = readStringAtAddress(state, arguments[2]);
	}

	resolutions_ty resList;

	processMemoryLocation(state, arguments[0], arguments[1], "make_symbolic", resList);

	for (resolutions_ty::iterator it = resList.begin(); it != resList.end(); it++) {
		const MemoryObject *mo = it->first.first;
		const ObjectState *os = it->first.second;
		ExecutionState *s = it->second;

		mo->setName(name);

		if (os->readOnly) {
			executor.terminateStateOnError(*s, "cannot make readonly object symbolic", "user.err");
		} else {
			executor.executeMakeSymbolic(*s, mo, os->isShared);
		}
	}
}

void SpecialFunctionHandler::handleMakeSymbolic(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	makeSymbolic(state, arguments);
}

void SpecialFunctionHandler::handleEvent(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(arguments.size() == 2 && "invalid number of arguments to klee_event");

	if (!isa<ConstantExpr>(arguments[0]) || !isa<ConstantExpr>(arguments[1])) {
		executor.terminateStateOnError(state, "klee_event requires a constant arg", "user.err");
		return;
	}

	ref<ConstantExpr> type = cast<ConstantExpr>(arguments[0]);
	ref<ConstantExpr> value = cast<ConstantExpr>(arguments[1]);

	executor.executeEvent(state, (unsigned int) type->getZExtValue(),
			(long int) value->getZExtValue());
}

void SpecialFunctionHandler::handleMarkGlobal(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(arguments.size() == 1 && "invalid number of arguments to klee_mark_global");

	Executor::ExactResolutionList rl;
	executor.resolveExact(state, arguments[0], rl, "mark_global");

	for (Executor::ExactResolutionList::iterator it = rl.begin(), ie = rl.end(); it != ie; ++it) {
		const MemoryObject *mo = it->first.first;
		assert(!mo->isLocal);
		mo->isGlobal = true;
	}
}

void SpecialFunctionHandler::handleSyscall(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(arguments.size() >= 1 && "invalid number of arguments to syscall");

	if (ConstantExpr *syscallNo = dyn_cast<ConstantExpr>(arguments[0])) {
		switch (syscallNo->getZExtValue()) {
		/* Signal syscalls */
		case SYS_rt_sigaction:
		case SYS_sigaltstack:
		case SYS_signalfd:
		case SYS_signalfd4:
		case SYS_rt_sigpending:
		case SYS_rt_sigprocmask:
		case SYS_rt_sigreturn:
		case SYS_rt_sigsuspend:
			LOG(INFO)<< "Blocked syscall " << syscallNo->getZExtValue();
			executor.bindLocal(target, state,
					ConstantExpr::create(0,
							executor.getWidthForLLVMType(
									target->inst->getType())));
			break;
			default:
			executor.callUnmodelledFunction(state, target,
					executor.kmodule->module->getFunction("syscall"),
					arguments);
			break;
		}
	} else {
		executor.terminateStateOnError(state,
				"syscall requires a concrete syscall number", "user.err");
	}
}

void SpecialFunctionHandler::handleRecordExpr(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(arguments.size() == 2 && "invalid number of arguments to klee_record_expr");

	assert(isa<ConstantExpr>(arguments[0]) && "FIXME");
}

void SpecialFunctionHandler::handleBeginChecked(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	LOG(INFO)<< "Enabling full checks on path";
}

void SpecialFunctionHandler::handleEndChecked(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	LOG(INFO)<< "Disabling full checks on path";

}

void SpecialFunctionHandler::handleMemCmp(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(arguments.size() == 3 && "Invalid number of args in klee_memcpy");

	assert(isa<ConstantExpr>(arguments[0]) && "FIXME");
	assert(isa<ConstantExpr>(arguments[1]) && "FIXME");
	assert(isa<ConstantExpr>(arguments[2]) && "FIXME");

	ObjectPair op1, op2;
	bool success;

	uint64_t address1 = cast<ConstantExpr>(arguments[0])->getZExtValue();
	uint64_t address2 = cast<ConstantExpr>(arguments[1])->getZExtValue();

	success = state.addressSpace().resolveOne(cast<ConstantExpr>(arguments[0]), op1);
	assert(success && "FIXME");

	success = state.addressSpace().resolveOne(cast<ConstantExpr>(arguments[1]), op2);
	assert(success && "FIXME");

	unsigned size = cast<ConstantExpr>(arguments[2])->getZExtValue();

	ref<Expr> expr1 = op1.second->read(address1 - op1.first->address, 8 * size);
	ref<Expr> expr2 = op2.second->read(address2 - op2.first->address, 8 * size);

	bool res;
	success = executor.solver->mustBeTrue(data::OTHER, state, EqExpr::create(expr1, expr2), res);
	assert(success && "FIXME");

	executor.bindLocal(target, state,
			ConstantExpr::create(res, executor.getWidthForLLVMType(target->inst->getType())));
}

void SpecialFunctionHandler::handleMPIcomm(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	/*assert(arguments.size() == 3 && "Invalid number of args in klee_MPI_comm");
	 assert(isa < ConstantExpr > (arguments[0]) && "FIXME");
	 assert(isa < ConstantExpr > (arguments[1]) && "FIXME");
	 assert(isa < ConstantExpr > (arguments[2]) && "FIXME");
	 mpi_rank_t src = dyn_cast < ConstantExpr > (arguments[0])->getZExtValue();
	 mpi_rank_t dest = dyn_cast < ConstantExpr > (arguments[1])->getZExtValue();
	 unsigned int type = dyn_cast < ConstantExpr
	 > (arguments[2])->getZExtValue();

	 CommLogItem *newitem = new CommLogItem(src, dest, (CommType) type,
	 (MatchType) 0);
	 std::vector<CommLogItem *> &selflog = state.globalCommLog.mpicommlog[src];

	 switch (type) {
	 case (COMM_BSEND): {
	 //XXX:deal with self-sending? And we need to know the legal range of ranks.
	 //assert(src != dest && "Send to self is forbidden!");
	 assert(dest >= 0 && "illegal dest of sending!");
	 CommLogItem * peerReceive;
	 std::vector<CommLogItem *> &peerlog = state.globalCommLog.mpicommlog[dest];

	 if (peerlog.empty()) {
	 newitem->matched = UNMATCHED;
	 (newitem->set_match_candidate).insert(dest); //push into mc

	 } else {
	 //XXX: it should not be the last one of a vector,if asynchronously.
	 peerReceive = peerlog[peerlog.size() - 1];
	 peerReceive->matched = newitem->matched = newitem->opMatched(
	 peerReceive);

	 if (newitem->matched == MATCHED) {
	 //std::set<mpi_rank_t>::iterator it =find(peerReceive->set_match_candidate.begin(),
	 //  	    	  	    			peerReceive->set_match_candidate.end(),newitem->rank);
	 std::set<mpi_rank_t>::iterator it =
	 peerReceive->set_match_candidate.find(newitem->rank);
	 if (it != peerReceive->set_match_candidate.end())
	 peerReceive->set_match_candidate.erase(it);
	 }
	 //XXX: matched ,partially matched,perhaps here we need fork?
	 else if (newitem->matched == PATIALLY_MATCHED) {
	 (peerReceive->set_match_candidate).insert(src);
	 } else if (newitem->matched == UNMATCHED) {
	 (newitem->set_match_candidate).insert(dest);
	 }
	 }
	 //XXX:NOT sure whether it works.
	 // Any send would leads a switch to receiver:
	 //                     receiver           |  sender
	 //    if queue not empty  wait(cond1)     |  post(cond1)
	 //                        post(cond2)     |  wait(cond2)
	 // unmatched : would be blocked waiting for the queue of receiver to be empty;
	 // matched,partially matched: sender would be blocked too.
	 state.next_thread_candidate = state.crtThread().getTid() + dest - src;
	 //selflog.push_back(newitem);
	 //LOG(INFO) << src <<" send to "<< dest <<"Matching:"<< newitem->matched;
	 newitem->print(LOG(INFO));
	 selflog.push_back(newitem);
	 break;
	 }
	 case (COMM_RECV): {
	 //assert(src != dest && "Receive from self.");
	 assert(dest >= -1 && "illegal peer of receive!");
	 CommLogItem * peerSend = NULL;

	 if (dest >= 0) {
	 std::vector<CommLogItem *> &peerlog = state.globalCommLog.mpicommlog[dest];
	 if (peerlog.empty()) {
	 newitem->matched = UNMATCHED;
	 (newitem->set_match_candidate).insert(dest); //push into mc
	 //XXX:NOT sure whether it works.
	 state.next_thread_candidate = state.crtThread().getTid() + dest
	 - src;
	 } else {
	 peerSend = peerlog[peerlog.size() - 1];
	 peerSend->matched = newitem->matched = newitem->opMatched(
	 peerSend);
	 if (newitem->matched == MATCHED) {
	 std::set<mpi_rank_t>::iterator it =
	 peerSend->set_match_candidate.find(newitem->rank);
	 if (it != peerSend->set_match_candidate.end())
	 peerSend->set_match_candidate.erase(it);
	 } else { //This should be UNMATCHED,never partialy matched.
	 (newitem->set_match_candidate).insert(dest); //push into mc
	 //XXX:NOT sure whether it works.
	 state.next_thread_candidate = state.crtThread().getTid()
	 + dest - src;
	 }

	 }
	 }				 	// end of if (dest>=0)
	 else if (dest == -1) {
	 //Checking for every rank
	 for (unsigned long i = 0;
	 i < state.crtProcessIt->second.threads.size() - 1; i++) {
	 if (i == (unsigned long) src)
	 continue;
	 std::vector<CommLogItem *> &peerlog = state.globalCommLog.mpicommlog[i];
	 peerSend = peerlog.empty() ? NULL : peerlog[peerlog.size() - 1];
	 if (peerSend)
	 peerSend->matched = newitem->opMatched(peerSend);

	 if (peerSend && peerSend->matched) {
	 newitem->matched = PATIALLY_MATCHED;
	 newitem->set_match_candidate.insert(peerSend->rank);
	 }
	 }
	 //Now we need to suspend this rank!
	 if (state.wildrecwlstid == 0)
	 state.wildrecwlstid = state.getWaitingList();
	 cout << "Rank" << src << " Sleep for wildcard receiving!" << endl;
	 cout << "MC={";
	 for (std::set<mpi_rank_t>::iterator it =
	 newitem->set_match_candidate.begin();
	 it != newitem->set_match_candidate.end(); it++)
	 cout << *it << ",";
	 cout << "}" << endl;
	 state.sleepThread(state.wildrecwlstid);
	 executor.schedule(state, false);
	 }				 	// end of if (dest == -1)
	 else {
	 LOG(WARNING)<< "Illegal dest of receive!";//Should not be reached
	 }
	 //newitem->print(LOG(INFO));
	 selflog.push_back(newitem);
	 break;
	 }
	 case (COMM_BARR): {
	 //barriers has no dest. So we make use of it. dest = N(i.e.commicator size)
	 assert(dest > 0 && "Barriers have all procs to participate");
	 newitem->print(LOG(INFO));
	 //selflog.push_back(newitem);
	 std::vector<CommLogItem *> &wildlog = state.globalCommLog.mpicommlog[-1];
	 if (wildlog.size() == 0 || wildlog.back()->matched == MATCHED) {
	 for (int i = 0; i < dest; i++) {
	 if (i == src)
	 continue;
	 newitem->set_match_candidate.insert(i);
	 }
	 wildlog.push_back(newitem);
	 } else {
	 wildlog.back()->set_match_candidate.erase(src);
	 if (wildlog.back()->set_match_candidate.size() == 0)
	 wildlog.back()->matched = MATCHED;
	 else
	 wildlog.back()->matched = PATIALLY_MATCHED;

	 delete newitem;
	 }
	 break;
	 }

	 default:
	 break;

	 }
	 return;*/
}

void SpecialFunctionHandler::handleGetAnysrcCandi(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	/*	ExecutionState::threads_ty::iterator it = state.crtThreadIt;
	 ExecutionState *lastState = &state;
	 //Communication log of this rank.
	 std::vector<CommLogItem *> &comlog = state.globalCommLog.mpicommlog[it->second.getTid()
	 - 1];
	 // Get match set candidate of this particular wild-card receive.
	 CommLogItem * wildrevlog = comlog[comlog.size() - 1];
	 std::set<mpi_rank_t>::iterator mcit =
	 wildrevlog->set_match_candidate.begin();
	 std::set<mpi_rank_t>::iterator itlastelment =
	 wildrevlog->set_match_candidate.end();
	 char candi_str[5];
	 int lastcandi;
	 wildrevlog->matched = MATCHED;
	 if (wildrevlog->set_match_candidate.size() >= 1) {
	 for (itlastelment--; mcit != itlastelment; mcit++) {
	 sprintf(candi_str, "%d", *mcit);
	 LOG(WARNING)<<"ANY_SOURCE is rewritened to be"<<candi_str;
	 Executor::StatePair sp = executor.fork(*lastState, KLEE_FORK_MULTI);
	 //LOG(WARNING) <<sp.first->stack().back().kf->function->getName().str();
	 //assert(sp.first->stack().back().kf->function->getName().str().compare("MPI_Recv")==0);
	 executor.bindLocal(target, *sp.first,
	 ConstantExpr::create(*mcit,
	 executor.getWidthForLLVMType(
	 target->inst->getType())));

	 }
	 lastcandi = *itlastelment;
	 sprintf(candi_str, "%d", lastcandi);
	 LOG(WARNING)<<"ANY_SOURCE is rewritened to be"<<candi_str;
	 it = state.crtThreadIt;
	 executor.bindLocal(target, state,
	 ConstantExpr::create(lastcandi,
	 executor.getWidthForLLVMType(target->inst->getType())));
	 } else // SHOULD not reach here!! It means dead lock/LEAK of hooking.
	 {
	 LOG(WARNING)<< "Get candidate for wildcard receive failed, so we quit. "
	 <<"Possible deadlock or A LEAK of hooking APIs."
	 <<"Check hook.c to make sure the all api needed is hooked!";
	 //executor.bindLocal(target, state, ConstantExpr::create(-1,
	 //              executor.getWidthForLLVMType(target->inst->getType())));
	 executor.terminateStateOnExit(state);
	 }*/
}

void SpecialFunctionHandler::handleMPInonblock(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
#define SYNC_ERR_CHK

#ifdef SYNC_ERR_CHK
	assert(arguments.size() == 4 && "Invalid number of args in handleMPInonblock");
	assert(isa<ConstantExpr>(arguments[0]) && "FIXME");
	assert(isa<ConstantExpr>(arguments[1]) && "FIXME");
	assert(isa<ConstantExpr>(arguments[2]) && "FIXME");
	assert(isa<ConstantExpr>(arguments[3]) && "FIXME");
	void * buf = (void *) dyn_cast<ConstantExpr>(arguments[0])->getZExtValue();
	long size = dyn_cast<ConstantExpr>(arguments[1])->getZExtValue();
	int type = dyn_cast<ConstantExpr>(arguments[2])->getZExtValue();
	void * req = (void *) dyn_cast<ConstantExpr>(arguments[3])->getZExtValue();
	//FIXME!! here we turn off the map size optimization
	switch (type) {
	case (COMM_ISEND): {
		state.globalCommLog.mpibuffermap[buf].req = req;
		state.globalCommLog.mpibuffermap[buf].size = size;
		state.globalCommLog.mpibuffermap[buf].state = INIT;
		state.globalCommLog.mpibuffermap[buf].commtype = COMM_ISEND;
		state.globalCommLog.mpibuffermap[buf].ownerinfo = state.getCurrentStackCallerInfo();
		break;
	}
	case (COMM_IRECV): {
		state.globalCommLog.mpibuffermap[buf].req = req;
		state.globalCommLog.mpibuffermap[buf].size = size;
		state.globalCommLog.mpibuffermap[buf].state = INIT;
		state.globalCommLog.mpibuffermap[buf].commtype = COMM_IRECV;
		state.globalCommLog.mpibuffermap[buf].ownerinfo = state.getCurrentStackCallerInfo();
		break;
	}
	case (COMM_WAIT): {
		std::map<buf_type, bufferInfo>::iterator it = state.globalCommLog.mpibuffermap.begin();
		std::map<buf_type, bufferInfo>::iterator end = state.globalCommLog.mpibuffermap.end();
		// for a wait, we do not have it's corresponding buff.
		for (; it != end; it++) {
			if (it->second.req == req) {
				it->second.state = FIN;
				if (SyncOpt) {
					state.globalCommLog.mpibuffermap.erase(it);
				}
				break;
			}
		}
		break;
	}
	case (COMM_TEST): {
		//LOG(INFO) <<"NOT support yet";

		break;
	}
	case (MPI_FINDATATRANS): {
		std::map<buf_type, bufferInfo>::iterator it = state.globalCommLog.mpibuffermap.begin();
		std::map<buf_type, bufferInfo>::iterator end = state.globalCommLog.mpibuffermap.end();
		for (; it != end; it++) {
			if (it->second.req == req) {
				it->second.state = READY;
				break;
			}
		}
		break;
	}
	default:
		LOG(INFO)<< "handleMPInonblock NOT supportted comm kind " << type;
		break;
	}
#endif
	return;
}

void SpecialFunctionHandler::handleDisableSyncChk(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(state.crtThread().getTid() >= 0 && state.crtThread().getTid() < 128 && "UNKOWN tid");
	assert(isa<ConstantExpr>(arguments[0]) && "FIXME");

	bool isdatatransfer = dyn_cast<ConstantExpr>(arguments[0])->getZExtValue();
	int index = state.crtThread().getTid();
	bool last_chk_enable = state.globalCommLog.sync_chk_enable[index];

	if (SyncOpt) {
		state.globalCommLog.sync_chk_enable[index] = false;
		//LOG(INFO)<<index<<" disable syncchk, file"<<state.prevPC()->info->file <<" line "<<state.prevPC()->info->line;
	} else if (isdatatransfer) {
		state.globalCommLog.sync_chk_enable[index] = false;
		//LOG(INFO)<<index<<" disable syncchk, file"<<state.prevPC()->info->file <<" line "<<state.prevPC()->info->line;
	}
	executor.bindLocal(target, state,
			ConstantExpr::create(last_chk_enable,
					executor.getWidthForLLVMType(target->inst->getType())));
	return;

}

void SpecialFunctionHandler::handleEnableSyncChk(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(state.crtThread().getTid() >= 0 && state.crtThread().getTid() < 128 && "UNKOWN tid");
	assert(isa<ConstantExpr>(arguments[0]) && "FIXME");

	bool isdatatransfer = dyn_cast<ConstantExpr>(arguments[0])->getZExtValue();
	int index = state.crtThread().getTid();
	bool last_chk_enable = state.globalCommLog.sync_chk_enable[index];

	if (SyncOpt) {
		state.globalCommLog.sync_chk_enable[index] = true;
		//LOG(INFO)<<index<<" enable syncchk, file"<<state.prevPC()->info->file <<" line "<<state.prevPC()->info->line;
	} else if (isdatatransfer) {
		state.globalCommLog.sync_chk_enable[index] = true;
		//LOG(INFO)<<index<<" enable syncchk, file"<<state.prevPC()->info->file <<" line "<<state.prevPC()->info->line;
	}
	executor.bindLocal(target, state,
			ConstantExpr::create(last_chk_enable,
					executor.getWidthForLLVMType(target->inst->getType())));
	return;

}

//MPI_Isend/MPI_Irecv(void*buf, int count, MPI_Datatype type, int dest, int tag, MPI_Comm, MPI_Request *);
// Two things to treat sends\recvs:
//1.  push the log into log table.
//2.  if this send /recv is blocking, sleep the current thread.
/*
 * YU: handler to handle the mpi operation with 7 parameters. e.g. send/receives
 * */
void SpecialFunctionHandler::handleFireMPI7para(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(state.crtThread().getTid() >= 0 && state.crtThread().getTid() < 128 && "UNKOWN tid");
	assert(isa<ConstantExpr>(arguments[0]) && "FIXME");
	assert(arguments.size() == 9 && "Invalid number of args in handleFireMPI7para");
	int i = 0;
	//state.getStackTrace().dump(LOG(INFO));
	//LOG(INFO)<<" handleFireMPI7para, file"<<state.prevPC()->info->file <<" line "<<state.prevPC()->info->line;
	CommLogItem *newitem = new CommLogItem();
	newitem->mpi_op = (CommType) dyn_cast<ConstantExpr>(arguments[0])->getZExtValue();
	newitem->funcname = newitem->getFuncname(); // just convert mpi_op to funcname
	//llvm::Function * f = state.globalCommLog.getFatherFunction(state, newitem->funcname);
	//newitem->callins = state.executor->getInsofFunction(state, state.crtThread(), f);
#if 0
	LOG(INFO)<<"SpecialFunctionHandler get function: \n";
	int e,j;
	target->inst->dump();
	LOG(INFO)<<"arguments : \n";
	CallSite cs((Instruction *)target->inst);
	for (j = 2, e = 9; j != e; ++j) {
		cs.getArgument(j)->dump();
	}
	LOG(INFO)<<"src/dest: "<<arguments[6];

	//state.executor->eval(state.stack().back().caller, 6, state).value.get()->dump();

#endif
	for (i = 2; i <= 8; i++) {
		if(i==5){
			LOG(INFO)<<"comm "<< i << arguments[i] << "\n";
		}
		assert(isa<ConstantExpr>(arguments[i]) && "FIXME");
//LOG(INFO)<<arguments[i];
		newitem->arguments.push_back(arguments[i]);
	}
	i = dyn_cast<ConstantExpr>(arguments[1])->getZExtValue();
	assert(i >= 0 && i <= 128);
	newitem->rank = i;
	newitem->index =
			state.globalCommLog.mpicommlog[i].begin() == state.globalCommLog.mpicommlog[i].end() ?
					0 : state.globalCommLog.mpicommlog[i].back()->index + 1; // return the index of last element plus one
	state.globalCommLog.mpicommlog[i].push_back(newitem);

	//associate every MPI operation with its static location
	newitem->file=state.executor->lastFile;
	newitem->lineNumber=state.executor->lastLineNumber;

   if(newitem->isWildcard(true)) //added by yhb
	    state.wildContained=true;

	if (newitem->isBlock()) { // only when block we handle the actually execution, otherwise just record it in the globalCommLog.mpicommlog
		if (state.globalCommLog.wlstids[i] == 0) {
			state.globalCommLog.wlstids[i] = state.getWaitingList();
		}
		state.globalCommLog.rank_blocked[i] = true;

		if (state.crtThread().getEnabled() == true) {
			state.sleepThread(state.globalCommLog.wlstids[i]);

		}
		if (state.globalCommLog.allNodesBlocked()) {
			//If all nodes blocked, but some are enabled, this means that in such process,
			// one or more non-blocking ops are still in the stack of this process.
			// we let it go, till all process blocked & not enabled, we check.
			if (!state.globalCommLog.allNodesNotEnabled(state))
				executor.schedule(state, false);
			//state.dumpStack();
			else
				state.globalCommLog.check(state, 0);
		}
		executor.schedule(state, false);
	}
 	return;
}

void SpecialFunctionHandler::handleFireMPI1para(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	LOG(INFO)<<state.crtThread().getPid()<< " /"<<state.crtThread().getTid()<<" args "<<arguments[2]
	<<" stack back: "<<state.stack().back().kf->function->getName().str() << " args "<<
	(state.stack().back().locals[state.stack().back().kf->getArgRegister(0)].value);
	assert(state.crtThread().getTid() >= 0 && state.crtThread().getTid() < 128 && "UNKOWN tid");
	assert(isa < ConstantExpr > (arguments[0]) && "FIXME");
	assert(arguments.size() == 3 && "Invalid number of args in handleFireMPI1para");
	int i;
//LOG(INFO)<<" handleFireMPI1para, file"<<state.prevPC()->info->file <<" line "<<state.prevPC()->info->line;
	CommLogItem *newitem = new CommLogItem();
	newitem->mpi_op = (CommType) dyn_cast < ConstantExpr > (arguments[0])->getZExtValue();
	newitem->funcname = newitem->getFuncname();
	//llvm::Function * f = state.globalCommLog.getFatherFunction(state, newitem->funcname);
	//newitem->callins = state.executor->getInsofFunction(state, state.crtThread(), f);

	assert(isa < ConstantExpr > (arguments[2]) && "FIXME");
//klee_debug(arguments[2]);
	newitem->arguments.push_back(arguments[2]);

	i = dyn_cast < ConstantExpr > (arguments[1])->getZExtValue();
	assert(i >= 0 && i <= 128);
	newitem->rank = i;
	newitem->index =
	state.globalCommLog.mpicommlog[i].begin() == state.globalCommLog.mpicommlog[i].end() ?
	0 : state.globalCommLog.mpicommlog[i].back()->index + 1;
	state.globalCommLog.mpicommlog[i].push_back(newitem);
   /*YU: a wlstids[rank]=threadID*/
	if (newitem->isBlock()) {
		if (state.globalCommLog.wlstids[i] == 0) {
			state.globalCommLog.wlstids[i] = state.getWaitingList();
		}
		state.globalCommLog.rank_blocked[i] = true;

		if (state.crtThread().getEnabled() == true) {
			state.sleepThread(state.globalCommLog.wlstids[i]);
		}

		if (state.globalCommLog.allNodesBlocked()) {
			//If all nodes blocked, but some are enabled, this means that in such process,
			// one or more non-blocking ops are still in the stack of this process.
			// we let it go, till all process blocked & not enabled, we check.
			if (!state.globalCommLog.allNodesNotEnabled(state))
			executor.schedule(state, false);
			//state.dumpStack();
			else
			state.globalCommLog.check(state, 0);
		}
		/*YU:SE and move forward*/
		executor.schedule(state, false);
	}
}

//We decide to not let MPI_Init appears in the trace.
void SpecialFunctionHandler::handleFireMPI2para(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(state.crtThread().getTid() >= 0 && state.crtThread().getTid() < 128 && "UNKOWN tid");
	assert(isa<ConstantExpr>(arguments[0]) && "FIXME");
	assert(arguments.size() == 4 && "Invalid number of args in handleFireMPI1para");
	int i = 0;
	//LOG(INFO)<<" handleFireMPI2para, file"<<state.prevPC()->info->file <<" line "<<state.prevPC()->info->line;

	if ((CommType) dyn_cast<ConstantExpr>(arguments[0])->getZExtValue() != COMM_INIT) {
		CommLogItem *newitem = new CommLogItem();
		newitem->mpi_op = (CommType) dyn_cast<ConstantExpr>(arguments[0])->getZExtValue();
		newitem->funcname = newitem->getFuncname();
		//llvm::Function * f = state.globalCommLog.getFatherFunction(state, newitem->funcname);
		//newitem->callins = state.executor->getInsofFunction(state, state.crtThread(), f);

		for (i = 2; i <= 3; i++) {
			assert(isa<ConstantExpr>(arguments[i]) && "FIXME");
			newitem->arguments.push_back(arguments[i]);
		}
		i = dyn_cast<ConstantExpr>(arguments[1])->getZExtValue();
		assert(i >= 0 && i <= 128);
		newitem->rank = i;
		newitem->index =
				state.globalCommLog.mpicommlog[i].begin()
						== state.globalCommLog.mpicommlog[i].end() ?
						0 : state.globalCommLog.mpicommlog[i].back()->index + 1;
		state.globalCommLog.mpicommlog[i].push_back(newitem);

		if (newitem->isBlock()) {
			if (state.globalCommLog.wlstids[i] == 0) {
				state.globalCommLog.wlstids[i] = state.getWaitingList();
			}
			state.globalCommLog.rank_blocked[i] = true;
			if (state.crtThread().getEnabled() == true) {
				state.sleepThread(state.globalCommLog.wlstids[i]);

			}
			if (state.globalCommLog.allNodesBlocked()) {
				//If all nodes blocked, but some are enabled, this means that in such process,
				// one or more non-blocking ops are still in the stack of this process.
				// we let it go, till all process blocked & not enabled, we check.
				if (!state.globalCommLog.allNodesNotEnabled(state))
					executor.schedule(state, false);
				//state.dumpStack();
				else
					state.globalCommLog.check(state, 0);
			}
			executor.schedule(state, false);
		}
	} else {
		i = dyn_cast<ConstantExpr>(arguments[1])->getZExtValue();
		assert(i >= 0 && i <= 128);
		state.globalCommLog.rank2thread[i] = state.crtThread().getTuid();
		LOG(INFO)<<"rank "<<i<< " pid,tid: "<<state.crtThread().getPid()<<","<<state.crtThread().getTid();
	}
//if(!newitem->funcname.compare("MPI_Finalize"))
}

//finalize && what ?



/*
 * YU
 * this is the handler for MPI_Finalize function
 * which means we generate a normal terminate path,
 * then we can use csp to model the global communication and do model checking
 * */
void SpecialFunctionHandler::handleFireMPI0para(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(state.crtThread().getTid() >= 0 && state.crtThread().getTid() < 128 && "UNKOWN tid");
	assert(isa<ConstantExpr>(arguments[0]) && "FIXME");
	assert(arguments.size() == 2 && "Invalid number of args in handleFireMPI0para");
	int i = 0;
//LOG(INFO)<<" handleFireMPI0para, file"<<state.prevPC()->info->file <<" line "<<state.prevPC()->info->line;
	CommLogItem *newitem = new CommLogItem();
	newitem->mpi_op = (CommType) dyn_cast<ConstantExpr>(arguments[0])->getZExtValue();
	newitem->funcname = newitem->getFuncname();
	//llvm::Function * f = state.globalCommLog.getFatherFunction(state, newitem->funcname);
	//newitem->callins = state.executor->getInsofFunction(state, state.crtThread(), f);
	i = dyn_cast<ConstantExpr>(arguments[1])->getZExtValue();
	assert(i >= 0 && i <= 128);
	newitem->rank = i;
	newitem->index =
			state.globalCommLog.mpicommlog[i].begin() == state.globalCommLog.mpicommlog[i].end() ?
					0 : state.globalCommLog.mpicommlog[i].back()->index + 1;
	state.globalCommLog.mpicommlog[i].push_back(newitem);
	/*since process[i] finalize, we can dump the communication operation of rank[i]*/
	if (!newitem->funcname.compare("PMPI_Finalize")) {
		//dump PC
//		LOG(INFO)<<"Now PC is: ";
//		state.globalConstraints.dumpPC();
		std::vector<CommLogItem *>::iterator it;
		it = state.globalCommLog.mpicommlog[i].begin();
		LOG(INFO)<< "Dump the operations of rank " << i<< "******************";
		while (it != state.globalCommLog.mpicommlog[i].end()) {
			//(*it)->print(LOG(INFO));
			LOG(INFO)<< (*it)->funcname;
			it++;
		}
		LOG(INFO)<< "**************************************************";
		if (state.globalCommLog.wlstids[i] == 0) {
			state.globalCommLog.wlstids[i] = state.getWaitingList();
		}
		state.globalCommLog.rank_blocked[i] = true;
		state.globalCommLog.rank_finalized[i] = true;

		if (state.crtThread().getEnabled() == true) {
			state.sleepThread(state.globalCommLog.wlstids[i]);
		}

		if (state.globalCommLog.allNodesFinished() && WithLibMPI && WithWildChoice) {
			 string res;
					std::ostringstream info;
					ExprPPrinter::printConstraints(info, state.constraints());
					res = info.str();
					LOG(INFO)<<res;
					(theStatisticManager->pcContainer).insert(res);

			theStatisticManager->iteration++;
			LOG(INFO)<<"All ranks finished normally."<<endl;
//    if ((state.executor->pcPATused).containsCM(state.globalConstraints)) {
//				LOG(INFO) << "Dump pcPATused---------------------------------"<<&state;
//				(state.executor->pcPATused).dumpPC();
//				LOG(INFO) << "Dump pcPATused end-----------------------------";
//				LOG(INFO) << "Dump pc of the contained-----------------------------";
//				state.globalConstraints.dumpPC();
//				LOG(INFO) << "Dump pc of the contained end-----------------------------";
//				if (!state.delayed)
//				  LOG(INFO) << "HAHAHAHA";
//				else
//					LOG(INFO) << "HUHUHU";
//			}

			if( (!(state.executor->pcPATused).containsCM(state.globalConstraints) || state.globalConstraints.size() == 0)  &&
					(state.wildContained || state.executor->property) ){ //only having wildcard or having LTL properties, we do pruning.
				state.executor->pcPATused=state.globalConstraints; // added yhb
							LOG(INFO)<<"*********************Using csp to model the communication  && call PAT to analyze the model**********************"<<endl;
							bool MSPattern=false;
							int tasks=-1;
							int slaves=-1;
							int& x=tasks;
							int& y=slaves;
							if(state.detectedMSPattern){
							// we need to do some preparation for Modeling Master-Slave pattern
           prepareMS(state.globalCommLog.mpicommlog,x,y);
					   dumpMS(state.globalCommLog.mpicommlog);
					   MSPattern=true;
							}
  							temporalProperty temp;
//							temp.rank1=1;
//							temp.index1=0;
//							temp.rank2=2;
//							temp.index2=0;
							// default mode for deadlock detection!
							temp.rank1=-1;
							temp.index1=-1;
							temp.rank2=-1;
							temp.index2=-1;
							//generate_csp(state.globalCommLog.mpicommlog,MSPattern,temp);
							generate_csp(&state, state.globalCommLog.mpicommlog,MSPattern,temp, false,tasks,slaves,false); // last boolean variable is for optimization.
							bool csp_pruning=true; // an arbiter to control whether do pruning or not!
							//returnlist* re = get_wildcard();
							int result=getResult();
							assert(result!=-1);//the case that model checker timeout
							//if(!re->empty()) {
							if(result==1) {
								theStatisticManager->iterationForDeadlock=theStatisticManager->iteration;//added by yhb to indicate that a deadlock happened
								//string statemark = get_state_mark(re,state.globalCommLog.mpicommlog);
								//LOG(INFO)<<"handleFireMPI0para, got the counter example path: "<<statemark;

								// new added by yhb, since our modeling is complete!
								std::set<ExecutionState*>::iterator it = (state.executor->states).begin();
								while (it != (state.executor->states).end()) {
		           (*it)->delayed=true;
									 it++;
								}

								// new deleted by yhb, since our modeling is complete!
//								state.executor->wildmatchesmark=statemark;
//								state.executor->preprocessAddedstates(state,statemark);
							}
							// deadlockfree, then we prune the states under the same pc condition, but generated by different matchings!
							else {
								assert(result==0); //check model checker gives deadlock free
					if (csp_pruning) {
						std::set<ExecutionState*>::iterator it = (state.executor->states).begin();
						//(state.executor->pcPATused).dumpPC();
						while (it != (state.executor->states).end()) {
							LOG(INFO) << "state: " << *it << "\n";
							if ((state.executor->pcPATused).containsCM((*it)->globalConstraints)) {
								//  (state.executor->removedStates).insert(*it);
								 // we use the delayed handling to replace the original pruning mode
                                (*it)->delayed=true;
							}
//							else
//								(*it)->globalConstraints.dumpPC();
							it++;
						}
					}
				}
			}
			//Now we can not kill other states if csp tells us that there are no deadlock.
			/*pruning since deadlock free*/
//			else {
//				state.executor->preprocessAddedstates(state,"");
//			}
		}
		else if(state.globalCommLog.allNodesFinished() && WithLibMPI)
		{
			LOG(INFO)<<"All ranks finished normally, since no wildcard, we don't need pat to do model checking";
			string res;
			std::ostringstream info;
			ExprPPrinter::printConstraints(info, state.constraints());
			res = info.str();
			LOG(INFO)<<res;
			(theStatisticManager->pcContainer).insert(res);
			theStatisticManager->iteration++; // added by yhb to indicate the iteration we found deadlock!
		}

		if (theStatisticManager->iterationForDeadlock == -1 && theStatisticManager->iterationForLTLViolation == -1) {
            if (state.globalCommLog.allNodesBlocked() || state.globalCommLog.allNodesFinished())
                state.globalCommLog.check(state, 0);

            executor.schedule(state, false);
		}
	}
}

/*added by yhb to test the master-slave pattern  handleFireMPISlave*/
void SpecialFunctionHandler::handleFireMPISlave(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments){
	LOG(INFO)<< "recognize slave send successfully! \n";
}

void SpecialFunctionHandler::handleSetNodeCount(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(state.crtThread().getTid() >= 0 && state.crtThread().getTid() < 128 && "UNKOWN tid");
	assert(arguments.size() == 1 && isa<ConstantExpr>(arguments[0]) && "FIXME");
	state.globalCommLog.mpinodes = dyn_cast<ConstantExpr>(arguments[0])->getZExtValue();
}

void SpecialFunctionHandler::handleKeepBlock(ExecutionState &state, KInstruction *target,
		std::vector<ref<Expr> > &arguments) {
	assert(state.crtThread().getTid() >= 0 && state.crtThread().getTid() < 128 && "UNKOWN tid");

	Thread &thread0 = state.threads.find(state.globalCommLog.rank2thread[0])->second;
	int rank = state.crtThread().getTid() - thread0.getTid();//get the process rank of current process/thread
	assert(thread0.getTid() == 4);
	assert(rank >= 0 && rank <= 256 && "invalid rank in handlekeepblock.");
	CommLogItem * currentop = state.globalCommLog.mpicommlog[rank].back(); // the last mpi of current thread, which is the newest encountered operation!
	if (!state.crtThread().getEnabled()) {//check whether current thread is enabled!
		LOG(WARNING)<<"Disabled thread go into keepblock!!!!!!!!!!";
		//executor.schedule(state, true);
	}

	if (rank >= 0 && state.globalCommLog.rank_blocked[rank] && state.crtThread().getEnabled()) {
		assert(
				state.crtThread().getTuid()
						== state.threads.find(state.globalCommLog.rank2thread[rank])->second.getTuid());
		if (state.globalCommLog.wlstids[rank] == 0) {
			state.globalCommLog.wlstids[rank] = state.getWaitingList();
		}
		LOG(INFO)<<"blocking rank "<<rank<<" for state: "<<&state<< " op: "<<*currentop;
		executor.bindLocal(target, state,ConstantExpr::create(true,executor.getWidthForLLVMType(target->inst->getType()))); //ADDED TO FIX THE ISA ERROR!
		state.sleepThread(state.globalCommLog.wlstids[rank]);
		if (state.globalCommLog.allNodesNotEnabled(state)) { //all blocked and not enabled?
			state.globalCommLog.check(state, 0);
		}
		/*ADD THE TWO LINES BELOW TO FIX THE ISA ERROR*/
		executor.schedule(state, false);
		return;
	}
	executor.bindLocal(target, state,
			ConstantExpr::create(state.globalCommLog.rank_blocked[rank],
					executor.getWidthForLLVMType(target->inst->getType())));
	if(state.globalCommLog.rank_blocked[rank])
		executor.schedule(state, false);
	return;
}
