#KLEE=/home/yhb/MPISE_Install/CloudYU/src/MPISE/Release+Asserts/bin/klee
#OutputDir=/home/yhb/MPISE_Install/CloudYU/src/MPISE/benchmarks/runExperiment/result_kfray

#24 tasks in total


#run kfray under 2 procs
rm -f -R $2/kfray-1.0.1/kfray_opt_2
AZQMPI_NODES=2 ; $1 -output-dir=$2/kfray-1.0.1/kfray_opt_2 -max-time=$3 -model-generation-path=$4 -lib-MPI -use-directeddfs-search -wild-opt -threaded-all-globals ./kfray.bc -i scenevide.kfr -o aaa1.ppm -unsafe
