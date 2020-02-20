#KLEE=/home/yhb/MPISE_Install/CloudYU/src/MPISE/Release+Asserts/bin/klee
#OutputDir=/home/yhb/MPISE_Install/CloudYU/src/MPISE/benchmarks/runExperiment/result_kfray
#Benchmark=/home/yhb/MPISE_Install/CloudYU/src/MPISE/benchmarks


#AZQMPI_NODES=6 time -p $KLEE -output-dir=$Benchmark/kfray-1.0.1/kfray_6 -max-time=60 -lib-MPI -threaded-all-globals ./kfray.bc -i ../scenes/scenevide.kfr -o aaa2.ppm > $OutputDir/kfray6.log 2>&1 -unsafe &

#Benchmark=/home/yhb//MPISE_Install/CloudYU/src/MPISE/benchmarks


AZQMPI_NODES=3 time -p /home/yhb/MPISE_Install/CloudYU/src/MPISE/Release+Asserts/bin/klee -max-time=1800 -lib-MPI -use-directeddfs-search -wild-opt -threaded-all-globals -libc=uclibc -posix-runtime ./kfray.bc -i scenevide.kfr -o aaa10.ppm > kfray_result.log 2>&1 -unsafe
