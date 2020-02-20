make clean
mpisecc.sh -std=c89 *.c
llvm-link *.o -o cluster.bc
