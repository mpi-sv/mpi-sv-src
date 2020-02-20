#KLEE=/home/yhb/MPISE_Install/CloudYU/src/MPISE/Release+Asserts/bin/klee
#OutputDir=/home/yhb/MPISE_Install/CloudYU/src/MPISE/benchmarks/runExperiment/result_kfray

#24 tasks in total



#run kfray under 6 procs
rm -f -R $2/kfray-1.0.1/kfray_opt_6
rm -f -R $2/kfray-1.0.1/kfray_6
rm -f -R $2/kfray-1.0.1/kfray_mut1_6
rm -f -R $2/kfray-1.0.1/kfray_mut1_opt_6
rm -f -R $2/kfray-1.0.1/kfray_mut2_6
rm -f -R $2/kfray-1.0.1/kfray_mut2_opt_6
rm -f -R $2/kfray-1.0.1/kfray_mut3_6
rm -f -R $2/kfray-1.0.1/kfray_mut3_opt_6

AZQMPI_NODES=6 time -p $1 -output-dir=$2/kfray-1.0.1/kfray_6 -max-time=$3 -lib-MPI -threaded-all-globals ./kfray.bc -i ../scenes/scenevide.kfr -o aaa2.ppm > $2/runExperiment/result_kfray/kfray6.log 2>&1 -unsafe 
AZQMPI_NODES=6 time -p $1 -output-dir=$2/kfray-1.0.1/kfray_opt_6 -max-time=$3 -lib-MPI -use-directeddfs-search -wild-opt -threaded-all-globals ./kfray.bc -i ../scenes/scenevide.kfr -o aaa1.ppm > $2/runExperiment/result_kfray/kfray6.opt.log 2>&1 -unsafe 
AZQMPI_NODES=6 time -p $1 -output-dir=$2/kfray-1.0.1/kfray_mut1_6 -max-time=$3 -lib-MPI -threaded-all-globals ./kfray_muts1.o -i ../scenes/scenevide.kfr -o aaa3.ppm > $2/runExperiment/result_kfray/kfray6_muts1.log 2>&1 -unsafe 
AZQMPI_NODES=6 time -p $1 -output-dir=$2/kfray-1.0.1/kfray_mut1_opt_6 -max-time=$3 -lib-MPI -use-directeddfs-search -wild-opt -threaded-all-globals ./kfray_muts1.o -i ../scenes/scenevide.kfr -o aaa4.ppm > $2/runExperiment/result_kfray/kfray6_muts1.opt.log 2>&1 -unsafe 
AZQMPI_NODES=6 time -p $1 -output-dir=$2/kfray-1.0.1/kfray_mut2_6 -max-time=$3 -lib-MPI -threaded-all-globals ./kfray_muts2.o -i ../scenes/scenevide.kfr -o aaa5.ppm > $2/runExperiment/result_kfray/kfray6_muts2.log 2>&1 -unsafe 
AZQMPI_NODES=6 time -p $1 -output-dir=$2/kfray-1.0.1/kfray_mut2_opt_6 -max-time=$3 -lib-MPI -use-directeddfs-search -wild-opt -threaded-all-globals ./kfray_muts2.o -i ../scenes/scenevide.kfr -o aaa6.ppm > $2/runExperiment/result_kfray/kfray6_muts2.opt.log 2>&1 -unsafe 
AZQMPI_NODES=6 time -p $1 -output-dir=$2/kfray-1.0.1/kfray_mut3_6 -max-time=$3 -lib-MPI -threaded-all-globals ./kfray_muts3.o -i ../scenes/scenevide.kfr -o aaa7.ppm > $2/runExperiment/result_kfray/kfray6_muts3.log 2>&1 -unsafe 
AZQMPI_NODES=6 time -p $1 -output-dir=$2/kfray-1.0.1/kfray_mut3_opt_6 -max-time=$3 -lib-MPI -use-directeddfs-search -wild-opt -threaded-all-globals ./kfray_muts3.o -i ../scenes/scenevide.kfr -o aaa8.ppm > $2/runExperiment/result_kfray/kfray6_muts3.opt.log 2>&1 -unsafe 

#run kfray under 8 procs
#sleep 120

rm -f -R $2/kfray-1.0.1/kfray_opt_8
rm -f -R $2/kfray-1.0.1/kfray_8
rm -f -R $2/kfray-1.0.1/kfray_mut1_8
rm -f -R $2/kfray-1.0.1/kfray_mut1_opt_8
rm -f -R $2/kfray-1.0.1/kfray_mut2_8
rm -f -R $2/kfray-1.0.1/kfray_mut2_opt_8
rm -f -R $2/kfray-1.0.1/kfray_mut3_8
rm -f -R $2/kfray-1.0.1/kfray_mut3_opt_8

AZQMPI_NODES=8 time -p $1 -output-dir=$2/kfray-1.0.1/kfray_8 -max-time=$3 -lib-MPI -threaded-all-globals ./kfray.bc -i ../scenes/scenevide.kfr -o aaa10.ppm > $2/runExperiment/result_kfray/kfray8.log 2>&1 -unsafe 
AZQMPI_NODES=8 time -p $1 -output-dir=$2/kfray-1.0.1/kfray_opt_8 -max-time=$3 -lib-MPI -use-directeddfs-search -wild-opt -threaded-all-globals ./kfray.bc -i ../scenes/scenevide.kfr -o aaa9.ppm > $2/runExperiment/result_kfray/kfray8.opt.log 2>&1 -unsafe 
AZQMPI_NODES=8 time -p $1 -output-dir=$2/kfray-1.0.1/kfray_mut1_8 -max-time=$3 -lib-MPI -threaded-all-globals ./kfray_muts1.o -i ../scenes/scenevide.kfr -o aaa11.ppm > $2/runExperiment/result_kfray/kfray8_muts1.log 2>&1 -unsafe 
AZQMPI_NODES=8 time -p $1 -output-dir=$2/kfray-1.0.1/kfray_mut1_opt_8 -max-time=$3 -lib-MPI -use-directeddfs-search -wild-opt -threaded-all-globals ./kfray_muts1.o -i ../scenes/scenevide.kfr -o aaa12.ppm > $2/runExperiment/result_kfray/kfray8_muts1.opt.log 2>&1 -unsafe 
AZQMPI_NODES=8 time -p $1 -output-dir=$2/kfray-1.0.1/kfray_mut2_8 -max-time=$3 -lib-MPI -threaded-all-globals ./kfray_muts2.o -i ../scenes/scenevide.kfr -o aaa13.ppm > $2/runExperiment/result_kfray/kfray8_muts2.log 2>&1 -unsafe 
AZQMPI_NODES=8 time -p $1 -output-dir=$2/kfray-1.0.1/kfray_mut2_opt_8 -max-time=$3 -lib-MPI -use-directeddfs-search -wild-opt -threaded-all-globals ./kfray_muts2.o -i ../scenes/scenevide.kfr -o aaa14.ppm > $2/runExperiment/result_kfray/kfray8_muts2.opt.log 2>&1 -unsafe 
AZQMPI_NODES=8 time -p $1 -output-dir=$2/kfray-1.0.1/kfray_mut3_8 -max-time=$3 -lib-MPI -threaded-all-globals ./kfray_muts3.o -i ../scenes/scenevide.kfr -o aaa15.ppm > $2/runExperiment/result_kfray/kfray8_muts3.log 2>&1 -unsafe 
AZQMPI_NODES=8 time -p $1 -output-dir=$2/kfray-1.0.1/kfray_mut3_opt_8 -max-time=$3 -lib-MPI -use-directeddfs-search -wild-opt -threaded-all-globals ./kfray_muts3.o -i ../scenes/scenevide.kfr -o aaa16.ppm > $2/runExperiment/result_kfray/kfray8_muts3.opt.log 2>&1 -unsafe 

#sleep 120
#run kfray under 10 procs
rm -f -R $2/kfray-1.0.1/kfray_opt_10
rm -f -R $2/kfray-1.0.1/kfray_10
rm -f -R $2/kfray-1.0.1/kfray_mut1_10
rm -f -R $2/kfray-1.0.1/kfray_mut1_opt_10
rm -f -R $2/kfray-1.0.1/kfray_mut2_10
rm -f -R $2/kfray-1.0.1/kfray_mut2_opt_10
rm -f -R $2/kfray-1.0.1/kfray_mut3_10
rm -f -R $2/kfray-1.0.1/kfray_mut3_opt_10

AZQMPI_NODES=10 time -p $1 -output-dir=$2/kfray-1.0.1/kfray_opt_10 -max-time=$3 -lib-MPI -use-directeddfs-search -wild-opt -threaded-all-globals ./kfray.bc -i ../scenes/scenevide.kfr -o aaa17.ppm > $2/runExperiment/result_kfray/kfray10.opt.log 2>&1 -unsafe 
AZQMPI_NODES=10 time -p $1 -output-dir=$2/kfray-1.0.1/kfray_10 -max-time=$3 -lib-MPI -use-directeddfs-search -threaded-all-globals ./kfray.bc -i ../scenes/scenevide.kfr -o aaa18.ppm > $2/runExperiment/result_kfray/kfray10.log 2>&1 -unsafe 
AZQMPI_NODES=10 time -p $1 -output-dir=$2/kfray-1.0.1/kfray_mut1_10 -max-time=$3 -lib-MPI -threaded-all-globals ./kfray_muts1.o -i ../scenes/scenevide.kfr -o aaa19.ppm > $2/runExperiment/result_kfray/kfray10_muts1.log 2>&1 -unsafe 
AZQMPI_NODES=10 time -p $1 -output-dir=$2/kfray-1.0.1/kfray_mut1_opt_10 -max-time=$3 -lib-MPI -use-directeddfs-search -wild-opt -threaded-all-globals ./kfray_muts1.o -i ../scenes/scenevide.kfr -o aaa20.ppm > $2/runExperiment/result_kfray/kfray10_muts1.opt.log 2>&1 -unsafe 
AZQMPI_NODES=10 time -p $1 -output-dir=$2/kfray-1.0.1/kfray_mut2_10 -max-time=$3 -lib-MPI -threaded-all-globals ./kfray_muts2.o -i ../scenes/scenevide.kfr -o aaa21.ppm > $2/runExperiment/result_kfray/kfray10_muts2.log 2>&1 -unsafe 
AZQMPI_NODES=10 time -p $1 -output-dir=$2/kfray-1.0.1/kfray_mut2_opt_10 -max-time=$3 -lib-MPI -use-directeddfs-search -wild-opt -threaded-all-globals ./kfray_muts2.o -i ../scenes/scenevide.kfr -o aaa22.ppm > $2/runExperiment/result_kfray/kfray10_muts2.opt.log 2>&1 -unsafe 
AZQMPI_NODES=10 time -p $1 -output-dir=$2/kfray-1.0.1/kfray_mut3_10 -max-time=$3 -lib-MPI -threaded-all-globals ./kfray_muts3.o -i ../scenes/scenevide.kfr -o aaa23.ppm > $2/runExperiment/result_kfray/kfray10_muts3.log 2>&1 -unsafe 
AZQMPI_NODES=10 time -p $1 -output-dir=$2/kfray-1.0.1/kfray_mut3_opt_10 -max-time=$3 -lib-MPI -use-directeddfs-search -wild-opt -threaded-all-globals ./kfray_muts3.o -i ../scenes/scenevide.kfr -o aaa24.ppm > $2/runExperiment/result_kfray/kfray10_muts3.opt.log 2>&1 -unsafe 
