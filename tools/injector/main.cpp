#include "klee/ExecutionState.h"
#include "klee/Interpreter.h"
#include "klee/Internal/Support/ModuleUtil.h"
#include "klee/Init.h"


#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/system_error.h"
#include "llvm/Support/CallSite.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/InlineAsm.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Support/raw_os_ostream.h"

#include <dirent.h>
#include <llvm/PassManager.h>

#include "Injection.h"

using namespace llvm;
using namespace klee;


static void parseArguments(int argc, char **argv) {
  std::vector<std::string> arguments;

  for (int i=1; i<argc; i++) {
    arguments.push_back(argv[i]);
  }

  int numArgs = arguments.size() + 1;
  char **argArray = new char*[numArgs+1];
  argArray[0] = argv[0];
  argArray[numArgs] = 0;
  for (int i=1; i<numArgs; i++) {
    argArray[i] = new char[arguments[i-1].size() + 1];
    std::copy(arguments[i-1].begin(), arguments[i-1].end(), argArray[i]);
    argArray[i][arguments[i-1].size()] = '\0';
  }

  cl::ParseCommandLineOptions(numArgs, (char**) argArray, " klee\n");
  for (int i=1; i<numArgs; i++) {
    delete[] argArray[i];
  }
  delete[] argArray;
}

Module* loadByteCode(const std::string &InputFile) {
  std::string ErrorMsg;
  Module *mainModule = 0;
  OwningPtr<MemoryBuffer> BufferPtr;
  error_code ec=MemoryBuffer::getFileOrSTDIN(InputFile.c_str(), BufferPtr);
  if (ec) {
    LOG(FATAL) << "error loading program " << InputFile.c_str() << ": " <<
    ec.message().c_str();
  }
  mainModule = getLazyBitcodeModule(BufferPtr.take(), getGlobalContext(), &ErrorMsg);
  if (mainModule) {
    if (mainModule->MaterializeAllPermanently(&ErrorMsg)) {
      delete mainModule;
      mainModule = 0;
    }
  }
  if (!mainModule) {
    LOG(FATAL) << "error loading program " << InputFile.c_str() << ": " <<
    ec.message().c_str();
  }

  return mainModule;
}

cl::opt<std::string> templateFileName(
        "template-file",
        cl::desc("template file name"));

cl::opt<std::string> outputFileName(
        "output-file",
        cl::desc("name of the file after mutation"));

extern std::string InputFile;

int main(int argc, char **argv, char **envp) {  

  llvm::InitializeNativeTarget();

  llvm::errs() << "\n";

  parseArguments(argc, argv);

  Module *mainModule = loadByteCode();

  PassManager pm;
  MPIInjectionPass *ip = new MPIInjectionPass();
  ip->templateFile = templateFileName;
  pm.add(ip);
  pm.run(*mainModule);

  //mainModule->dump();

  // write the result to a bc file
  std::ios::openmode io_mode = std::ios::out | std::ios::trunc | std::ios::binary;
  std::ostream *f;
  std::string outputfile = outputFileName;

  f = new std::ofstream(outputfile.c_str(), io_mode);
  if (!f) {
    errs() << "Out of memory";
    return 0;
  } else if (!f->good()) {
    LOG(WARNING) << "Error opening: " << outputfile.c_str();
    delete f;
    f = NULL;
    return 0;
  }

  llvm::raw_os_ostream* rfs = new llvm::raw_os_ostream(*f);
  WriteBitcodeToFile(mainModule, *rfs);
  delete rfs;
  delete f;

  outs() << "Mutation is finished!\n";
  outs() << "Input file is : " << InputFile << "\n";
  outs() << "Template file is : " << templateFileName << "\n";
  outs() << "Output file is : " << outputfile << "\n";
  outs() << "The mutant MPI call is : " << ip->insInfo << "\n";
  return 0;
}
