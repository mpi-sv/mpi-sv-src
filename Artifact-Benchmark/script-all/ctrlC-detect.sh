#!/bin/bash

funWithParam(){

    result_Root=./result_$1

	dir=$(ls -l $result_Root |awk '!/^d/ {print $NF}' |grep -i "log" )
	for i in $dir
	do
		cat $result_Root/$i | grep -q 'ctrl'
		if [ $? == 0 ]; then
			#echo $i  >> rerun_list
			echo -n $1' ' >> rerun_list
			echo -n ${i:3:1} >> rerun_list
			echo -n " " >> rerun_list
			echo $i | grep -q 'process10'
			if [ $? == 0 ]; then
				echo -n ${i:12:2} >> rerun_list
				echo -n " 3600 " >> rerun_list
				echo  ${i:18:1} >> rerun_list
			else
				echo -n ${i:12:1} >> rerun_list
				echo -n " 3600 " >> rerun_list
				echo ${i:17:1} >> rerun_list
			fi
		fi
	done
}

Benchmark=/root/MPISE_root/MPISE_Install/CLOUD9/src/MPISE/Artifact-Benchmark
#Benchmark=/home/weijiang/MPISE_Install/CLOUD9/src/MPISE/Artifact-Benchmark
script_file_root=script-all

if [ -e $Benchmark/$script_file_root/rerun_list ];then
     rm -rf $Benchmark/$script_file_root/rerun_list
fi

touch rerun_list

funWithParam DTG
funWithParam Matmat-MS
funWithParam Integrate
funWithParam Integrate-MS
funWithParam Diffusion2d
funWithParam Gauss_elim
funWithParam Heat
funWithParam Mandelbrot
funWithParam Mandelbrot-MS
funWithParam Sorting-MS
funWithParam Image_mani
funWithParam DepSolver
funWithParam Kfray
funWithParam Kfray-MS
funWithParam Clustalw
