#AZQMPI_NODES=2 time /home/yhb/MPISE_Install/CloudYU/src/MPISE/Release+Asserts/bin/klee -max-time=3600 -lib-MPI -threaded-all-globals ./depsolver.bc -unsafe > depSolver-mpi.log 2>&1

AZQMPI_NODES=3 time /home/yhb/MPISE_Install/CloudYU/src/MPISE/Release+Asserts/bin/klee -max-time=3600 -lib-MPI -threaded-all-globals ./depsolver.bc -unsafe > depSolver-mpi.log 2>&1

#AZQMPI_NODES=3 time /home/yhb/MPISE_Install/CloudYU/src/MPISE/Release+Asserts/bin/klee -max-time=3600 -lib-MPI -use-directeddfs-search -wild-opt -threaded-all-globals ./depsolver.bc -unsafe > depSolver-mpi.opt.log 2>&1
