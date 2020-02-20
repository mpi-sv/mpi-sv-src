AZQMPI_NODES=4 time -p /home/yhb/MPISE_Install/CloudYU/src/MPISE/Release+Asserts/bin/klee -lib-MPI GE.o > GE4.log 2>&1
#AZQMPI_NODES=4 time -p /home/yhb/MPISE_Install/CloudYU/src/MPISE/Release+Asserts/bin/klee -lib-MPI -wild-opt -use-directeddfs-search GE.o >GE4.opt.log 2>&1

#AZQMPI_NODES=6 time -p /home/yhb/MPISE_Install/CloudYU/src/MPISE/Release+Asserts/bin/klee -max-time=1800 -lib-MPI GE.o > GE6.log 2>&1
#AZQMPI_NODES=6 time -p /home/yhb/MPISE_Install/CloudYU/src/MPISE/Release+Asserts/bin/klee -lib-MPI -wild-opt -use-directeddfs-search GE.o >GE6.opt.log 2>&1

#AZQMPI_NODES=8 time -p /home/yhb/MPISE_Install/CloudYU/src/MPISE/Release+Asserts/bin/klee -lib-MPI -wild-opt -use-directeddfs-search GE.o >GE8.opt.log 2>&1

#AZQMPI_NODES=8 time -p /home/yhb/MPISE_Install/CloudYU/src/MPISE/Release+Asserts/bin/klee -max-time=1800 -lib-MPI -wild-opt -use-directeddfs-search GE.o >GE8.opt.log 2>&1

 
 
