times=100;

while [ "${n:=0}" -le "$times" ]
do
mpiexec -n 3 ./crooked_barrier
let n+=1
done

