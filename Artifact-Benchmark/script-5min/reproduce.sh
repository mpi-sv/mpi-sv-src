#!/bin/bash

# usage: ./reproduce.sh <program_name> <mutate_flag> <process_num> <time_limit> <opt_flag>



				# specify the dir of KLEE and Benchmarks
# ============================================================================================== #
Klee=/root/MPISE_root/MPISE_Install/CLOUD9/src/MPISE/Release+Asserts/bin/klee
Benchmark=/root/MPISE_root/MPISE_Install/CLOUD9/src/MPISE/Artifact-Benchmark
#Klee=/home/weijiang/MPISE_Install/CLOUD9/src/MPISE/Release+Asserts/bin/klee
#Benchmark=/home/weijiang/MPISE_Install/CLOUD9/src/MPISE/Artifact-Benchmark
script_file_root=script-5min
# ============================================================================================== #




				# specify the dir of IR file
# ============================================================================================== #
if [ $1 == "DTG" ];then
     if [ $2 -eq 0 ];then
     	IR_dir=$Benchmark/dtg/dtg.o
     else
     	IR_dir=$Benchmark/dtg/dtg_muts$2.o
     fi


# ---------------------------------------------------------
elif [ $1 == "Integrate" ];then
     if [ $2 -eq 0 ];then
     	IR_dir=$Benchmark/integrate/integrate_mw.o
     else
     	IR_dir=$Benchmark/integrate/inte_muts$2.o
     fi


# ---------------------------------------------------------
elif [ $1 == "Diffusion2d" ];then
     if [ $2 -eq 0 ];then
     	if [ $3 -eq 4 ];then
               IR_dir=$Benchmark/diffusion2d/diffusion2_nd.o
     	elif [ $3 -eq 6 ];then
     		IR_dir=$Benchmark/diffusion2d/diffusion6_nd.o
     	fi
     else
     	if [ $3 -eq 4 ];then
     		IR_dir=$Benchmark/diffusion2d/diffusion2_nd_muts$2.o
     	elif [ $3 -eq 6 ];then
     		IR_dir=$Benchmark/diffusion2d/diffusion6_nd_muts$2.o
     	fi
     fi


# ---------------------------------------------------------
elif [ $1 == "Gauss_elim" ];then
     if [ $2 -eq 0 ];then
     	IR_dir=$Benchmark/gausselim/GE.o
     else
     	IR_dir=$Benchmark/gausselim/GE_muts$2.o
     fi


# ---------------------------------------------------------
elif [ $1 == "Heat" ];then
     if [ $2 -eq 0 ];then
     	IR_dir=$Benchmark/heat/heat-errors.o
     else
     	IR_dir=$Benchmark/heat/heat_muts$2.o
     fi


# ---------------------------------------------------------
elif [ $1 == "Mandelbrot" ];then
     if [ $2 -eq 0 ];then
     	IR_dir=$Benchmark/bitmap/mandel_bitmap_ori2.o
     else
     	IR_dir=$Benchmark/bitmap/mandel_bitmap_ori2_muts$2.o
     fi
     IR_dir=$IR_dir" -unsafe"


# ---------------------------------------------------------
elif [ $1 == "Image_mani" ];then
     if [ $2 -eq 0 ];then
     	IR_dir=$Benchmark/image-manip/image_mod_new.o
     else
     	IR_dir=$Benchmark/image-manip/image_muts$2.o
     fi
     IR_dir=$IR_dir" -unsafe"	


# ---------------------------------------------------------
elif [ $1 == "DepSolver" ];then
     IR_dir=$Benchmark/depSolver-mpi-$2-$3-$5/src/depsolver.bc
     IR_dir=$IR_dir" -unsafe"


# ---------------------------------------------------------
elif [ $1 == "Kfray" ];then
     if [ $2 -eq 0 ];then
     	IR_dir=$Benchmark/kfray-1.0.1/build/kfray.bc
     else
     	IR_dir=$Benchmark/kfray-1.0.1/build/kfray_muts$2.o
     fi
     IR_dir=$IR_dir" -i ../scenes/scenevide.kfr -o kfray.ppm  -unsafe"


# ---------------------------------------------------------
elif [ $1 == "Clustalw" ];then
     if [ $2 -eq 0 ];then
     	IR_dir=$Benchmark/clustalw-mpi-0.13/cluster.bc
     else
     	IR_dir=$Benchmark/clustalw-mpi-0.13/cluster_muts$2.o
     fi
     IR_dir=$IR_dir" -infile="$Benchmark"/clustalw-mpi-0.13/dele.input -unsafe"	


# ---------------------------------------------------------
elif [ $1 == "Matmat-MS" ];then
     IR_dir=$Benchmark/matmat/matMS.o
     IR_dir="-ms-support "$IR_dir
     IR_dir=$IR_dir" --libc=uclibc --posix-runtime -unsafe"


# ---------------------------------------------------------
elif [ $1 == "Integrate-MS" ];then
     IR_dir=$Benchmark/integrate/inteMS.o
     IR_dir="-ms-support "$IR_dir


# ---------------------------------------------------------
elif [ $1 == "Mandelbrot-MS" ];then
     IR_dir=$Benchmark/bitmap/mandelMS.o
     IR_dir="-ms-support "$IR_dir
     IR_dir=$IR_dir" -unsafe"


# ---------------------------------------------------------
elif [ $1 == "Sorting-MS" ];then
     IR_dir=$Benchmark/Master-Slave/mpi_master_slave.o
     IR_dir="-ms-support "$IR_dir
     IR_dir=$IR_dir" -unsafe"


# ---------------------------------------------------------
elif [ $1 == "Kfray-MS" ];then
     IR_dir=$Benchmark/kfrayMS/build/kfray.bc
     IR_dir="-ms-support "$IR_dir
     IR_dir=$IR_dir" -i ../scenes/scenevide.kfr -o kfray.ppm  -unsafe"
fi
# ============================================================================================== #



				# specify other parameters
# ============================================================================================== #
process_num=$3
time_limit=$4
if [ $5 == 1 ]; then
     opt_flag="-lib-MPI -use-directeddfs-search -threaded-all-globals -wild-opt"
else
     opt_flag="-lib-MPI -threaded-all-globals"
fi
output_dir=$Benchmark/$script_file_root/result_$1/mut$2\_process$3\_opt$5/
log_dir=$Benchmark/$script_file_root/result_$1/mut$2\_process$3\_opt$5.log
# ============================================================================================== #



				# running
# ============================================================================================== #
if [ ! -d $Benchmark/$script_file_root/result_$1  ];then
	mkdir -m 777 $Benchmark/$script_file_root/result_$1
fi

if [ -d $Benchmark/$script_file_root/result_$1/mut$2\_process$3\_opt$5  ];then
     rm -rf $Benchmark/$script_file_root/result_$1/mut$2\_process$3\_opt$5
     rm -rf $Benchmark/$script_file_root/result_$1/mut$2\_process$3\_opt$5.log
fi

if [ $1 == "DepSolver" ];then
	if [ ! -d $Benchmark/depSolver-mpi-$2-$3-$5  ];then
		cp -r $Benchmark/depSolver-mpi $Benchmark/depSolver-mpi-$2-$3-$5
	fi
	cd $Benchmark/depSolver-mpi-$2-$3-$5/src
elif [ $1 == "Kfray" ];then
	cd $Benchmark/kfray-1.0.1/build
elif [ $1 == "Kfray-MS" ];then
	cd $Benchmark/kfrayMS/build
elif [ $1 == "Image_mani" ];then
	cd $Benchmark/image-manip
elif [ $1 == "Mandelbrot" ];then
	cd $Benchmark/bitmap
fi


# output_information
echo -e "\n---------------------Benchmark Information----------------------"
echo "Program_name: "$1
echo "Mutate_flag: "$2
echo "Process_num: "$3
echo "Time_limit: "$4
if [ $5 == 1 ]; then
     echo "Mode: model-checking based boosting"
else
     echo "Mode: pure symbolic execution"
fi


AZQMPI_NODES=$process_num time -p $Klee -output-dir=$output_dir -max-time=$time_limit $opt_flag $IR_dir > $log_dir 2>&1
#echo "AZQMPI_NODES="$process_num" time -p "$Klee" -output-dir="$output_dir" -max-time="$time_limit" "$opt_flag" "$IR_dir" > "$log_dir" 2>&1"
if [ -d $Benchmark/depSolver-mpi-$2-$3-$5  ];then
	rm -r $Benchmark/depSolver-mpi-$2-$3-$5
fi

echo -e "\n-----------------------Output Information-------------------------"
cat $Benchmark/$script_file_root/result_$1/mut$2\_process$3\_opt$5.log | grep 'MPI-SV: totally'
cat $Benchmark/$script_file_root/result_$1/mut$2\_process$3\_opt$5.log | grep -q 'HaltTimer'
if [ $? == 0 ]; then
	echo "Timecost: TO"
	timeoutflag=1
else
	echo -n "Timecost: "
	cat  $Benchmark/$script_file_root/result_$1/mut$2\_process$3\_opt$5.log | grep 'real [0-9]'
	timeoutflag=0
fi

echo -n "Deadlock: "
cat $Benchmark/$script_file_root/result_$1/mut$2\_process$3\_opt$5.log | grep -q 'find a violation'
if [ $? == 0 ]; then
	echo "yes"
else
	if [ $timeoutflag == 0 ]; then
		echo "no"
	else
		echo "unknown"
	fi
fi
echo ""


# /home/MPISE_Install/CLOUD9_WJ/src/MPISE/benchmarks/
# (1) Mandelbrot: bitmap/dot.bmp
# (2) Image_mani: image-manip/dot.bmp
# (3) Matmat-MS: matmat/data

# /home/zbchen/mpisv/src/MPISV/Artifact-Benchmark/
# (1) Mandelbrot-MS: bitmap/dot.bmp









