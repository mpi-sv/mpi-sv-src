

AZQMPI_NODES=3 time -p /home/yhb/MPISE_Install/CloudYU/src/MPISE/Release+Asserts/bin/klee -max-time=1800 -lib-MPI -threaded-all-globals ./kfray.bc -i ../scenes/scenevide.kfr -o aaa10.ppm > kfray3_result.log 2>&1 -unsafe &

sleep 360

AZQMPI_NODES=3 time -p /home/yhb/MPISE_Install/CloudYU/src/MPISE/Release+Asserts/bin/klee -max-time=1800 -lib-MPI -use-directeddfs-search -wild-opt -threaded-all-globals ./kfray.bc -i ../scenes/scenevide.kfr -o aaa10.ppm > kfray3_opt_result.log 2>&1 -unsafe &

sleep 360

AZQMPI_NODES=6 time -p /home/yhb/MPISE_Install/CloudYU/src/MPISE/Release+Asserts/bin/klee -max-time=1800 -lib-MPI -threaded-all-globals ./kfray.bc -i ../scenes/scenevide.kfr -o aaa10.ppm > kfray6_result.log 2>&1 -unsafe &

sleep 360

AZQMPI_NODES=6 time -p /home/yhb/MPISE_Install/CloudYU/src/MPISE/Release+Asserts/bin/klee -max-time=1800 -lib-MPI -use-directeddfs-search -wild-opt -threaded-all-globals ./kfray.bc -i ../scenes/scenevide.kfr -o aaa10.ppm > kfray6_opt_result.log 2>&1 -unsafe &

