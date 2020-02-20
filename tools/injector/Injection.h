#ifndef MPISE_PASSES_H
#define MPISE_PASSES_H

#include "llvm/Constants.h"
#include "llvm/Instructions.h"
#include "llvm/Module.h"
#include "llvm/Pass.h"
#include "llvm/CodeGen/IntrinsicLowering.h"
#include <map>

namespace llvm {
    class Function;
    class Instruction;
    class Module;
    class TargetData;
    class TargetLowering;
    class Type;
}

namespace klee {

    class MPIInjectionPass : public llvm::ModulePass {
        static char ID;

    public:
        llvm::Module *module = NULL;
        std::string insInfo;

        std::string templateFile;
        MPIInjectionPass() : llvm::ModulePass(ID) {
            init();
        }

        virtual bool runOnModule(llvm::Module &M);

    private:
        std::map<std::string, std::string> mutationMap;

        void mutant(llvm::CallInst *ci);
        void init();
    };

}

#endif
