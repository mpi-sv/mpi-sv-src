//===-- RaiseAsm.cpp ------------------------------------------------------===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <llvm/Support/CallSite.h>
#include <llvm/Analysis/DebugInfo.h>
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/APInt.h"
#include "llvm/Linker.h"
#include "llvm/Support/Path.h"
#include "Injection.h"
#include <sstream>

using namespace llvm;
using namespace klee;

char MPIInjectionPass::ID = 0;

void MPIInjectionPass::init() {
  mutationMap["MPI_Recv"] = "__Inj_MPI_Recv";
  mutationMap["MPI_Irecv"] = "__Inj_MPI_Irecv";
}


Module *linkWithTemplate(Module *module,
                              const std::string &libraryName) {

  Linker linker("mutation", module, false);

  llvm::sys::Path libraryPath(libraryName);
  bool native = false;

  if (linker.LinkInFile(libraryPath, native)) {
    assert(0 && "linking template failed!");
  }

  return linker.releaseModule();
}

void MPIInjectionPass::mutant(llvm::CallInst *ci) {

  std::string funName = ci->getCalledFunction()->getName().str();
  std::string muName = mutationMap[funName];

  // link the template bitecode file
  module = linkWithTemplate(module, templateFile);

  // get the mutant function
  Function *muFun = module->getFunction(muName);

  // modify the callinstr to the mutant function (to be implemented)
  CallSite cs(ci);
  std::vector<Value *> args(cs.arg_begin(), cs.arg_end());
  Instruction *new_instr = CallInst::Create(muFun, args, "", ci);
  CallInst *new_ci = cast<CallInst>(new_instr);
  new_ci->setCallingConv(ci->getCallingConv());
  new_ci->setAttributes(ci->getAttributes());
  if (ci->isTailCall()) {
    new_ci->setTailCall(true);
  }
  new_ci->setDebugLoc(ci->getDebugLoc());
  ci->replaceAllUsesWith(new_ci);
  new_ci->takeName(ci);
  ci->eraseFromParent(); // remove the instruction pointed by ci

}

bool MPIInjectionPass::runOnModule(Module &M) {

  bool changed = true;
  module = &M;

  std::vector<CallInst *> recv_vector;

  for (Module::iterator fi = M.begin(), fe = M.end(); fi != fe; fi++) {
    for (Function::iterator bi = fi->begin(), be = fi->end(); bi != be; bi++) {
      for (BasicBlock::iterator ii = bi->begin(), ie= bi->end(); ii != ie; ii++) {
        if (isa<CallInst>(ii)) {
          CallInst *ci = (CallInst *) (&(*ii));
          Function *f = ci->getCalledFunction();

          if (f != NULL && mutationMap.count(f->getName().str()) > 0) {
            recv_vector.push_back(ci);
          }
        }
      }
    }
  }

  // randomly select a recv call instruction
  int size = recv_vector.size();
  srand(time(NULL));
  int index = rand()%size;
  CallInst *ci_tbm = recv_vector[index]; // the pointer to the callinstr to be modified

  //save information of the instruction
  if (MDNode *N = ci_tbm->getMetadata("dbg")){
    DILocation loc(N);
    std::stringstream ss;
    ss << ci_tbm->getCalledFunction()->getName().str();
    ss << ":";
    ss << loc.getFilename().str();
    ss << ":";
    ss << loc.getLineNumber();
    insInfo = ss.str();
  }

  // do the mutation
  mutant(ci_tbm);

  return changed;
}
