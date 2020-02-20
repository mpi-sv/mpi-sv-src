#!/bin/bash
# usage: ./collect.sh <program_name>

result_Root=../result_$1

if [ -f ./$1.txt ];then
	rm -f ./$1.txt
fi

dir=$(ls -l $result_Root |awk '!/^d/ {print $NF}' |grep -i "log" )
for i in $dir
do
	# iteration
	echo -n $i"   ------------>    " >> ./$1.txt
	cat $result_Root/$i | grep 'MPI-SV: totally' | less -S >> ./$1.txt

	## time
	echo -n $i"   ------------>    TIME: " >> ./$1.txt
	cat $result_Root/$i | grep -q 'HaltTimer'
	if [ $? == 0 ]; then
		echo "HaltTimer" >> ./$1.txt
		timeoutflag=1
	else
		cat $result_Root/$i | grep 'real [0-9]' >>  ./$1.txt
		timeoutflag=0
	fi
	## deadlock
	echo -n $i"   ------------>    Deadlock: " >> ./$1.txt
	cat $result_Root/$i | grep -q 'MPI-SV: find a violation in the'
	if [ $? == 0 ]; then
		echo "yes" >> ./$1.txt
	else
		if [ $timeoutflag == 0 ]; then
			echo "no" >> ./$1.txt
		else
     			echo "unknown" >> ./$1.txt
		fi
	fi
	#cat $result_Root/$i | grep 'ctrl' >> ./$1.txt
	echo '----------------------------------------------' >> ./$1.txt

done
