AZQMPI_NODES=3 time /home/yhb/MPISE_Install/CloudYU/src/MPISE/Release+Asserts/bin/klee -max-time=3600 -lib-MPI -threaded-all-globals ./cluster.bc -infile=./dele.input -unsafe > yu.log 2>&1
