all: clean compile link
clean:
	rm -rf heat-errors.o heat-errors.llvm
compile:
	clang -c -emit-llvm -g -I ../../azqmpi/include/ -I../../config -I../../idsp/lfq/spec/ -I ../../arch/include/ -I ../../inet/ -I ../../pmi/ -I/home/mpiklee/cloud9root/src/klee-uclibc/include -include hook.h ./heat-errors.c
link:
	llvm-ld ./heat-errors.o /home/mpiklee/cloud9root/src/klee-uclibc/lib/libm.a -b ./heat-errors.llvm
 
