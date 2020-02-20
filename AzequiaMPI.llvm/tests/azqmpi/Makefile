#TOP = /home/mpiklee/cloud9root/src/
#LLVMROOTDIR = $(TOP)/third_party/llvm-build
#LLVMTOOLDIR = $(LLVMROOTDIR)/Debug+Asserts/bin
#LLVMPLUGIN = $(LLVMROOTDIR)/Debug+Asserts/lib/LLVMgold.so

#BINUTILSDIR = $(TOP)/third_party/binutils-install

#CC         = $(LLVMTOOLDIR)/clang -flto -isystem /usr/include/x86_64-linux-gnu -B$(LLVMROOTDIR)/Debug+Asserts/lib/clang/3.1
#AR         = $(BINUTILSDIR)/bin/ar --plugin $(LLVMPLUGIN)
#RANLIB           = $(BINUTILSDIR)/bin/ar --plugin $(LLVMPLUGIN) -s
#LD         = $(BINUTILSDIR)/bin/ld
#NM         = $(LLVMTOOLDIR)/llvm-nm

#LDFLAGS          := -Wl,-plugin-opt=also-emit-llvm -lazqmpi -L/home/mpiklee/workspace/libmumpi/AzequiaMPI.llvm/lib
#ARFLAGS          := -cru
AZQROOT     =/home/mpiklee/workspace/libmumpi/AzequiaMPI.llvm
CFLAGS = -I$(AZQROOT)/pmi -I$(AZQROOT)/pmi/simple -I$(AZQROOT)/arch/include -I$(AZQROOT)/idsp/lfq/impl -I$(AZQROOT)/idsp/lfq/spec -I$(AZQROOT)/inet -I$(AZQROOT)/inet/none/spec -I$(AZQROOT)/inet/none/impl -I/usr/local/include -I$(AZQROOT)/azqmpi/include -I$(AZQROOT)/config
all:
	clang -c -g -emit-llvm $(CFLAGS) -include hook.h $(pro) 
	#llvm-ld ./basic.o /home/mpiklee/workspace/libmumpi/AzequiaMPI.llvm/lib/libazqmpi.a -b ./basic
#$(LD) $(LDFLAGS) basic.o -b basic
klee:
	klee -libc=uclibc -posix-runtime ./basic
