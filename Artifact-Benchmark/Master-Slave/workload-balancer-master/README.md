workload-balancer
================
### Author

* Jared Klingenberger <klinge2@clemson.edu>

A static and a dynamic work balancer using the MPI library. Each workload unit
is a number in the interval `[0,4]` correlated with how long the process should
sleep (simulating work).

This program was written for CpSc 3620 (Distributed Programming) at Clemson
University.

Discussion
==========

### Static workload balancer

The [static balancer](static_mpi.c) uses a contiguous block
distribution algorithm. The main workload (an array found on the master node) is
divided up into `n` blocks, where n is the number of MPI processes. Rather than 
using the naïve approach of giving each process `WORKLOAD_SIZE/n` units, the 
algorithm calculates the mean and standard deviation of the total workload and 
makes sure each process gets roughly mean +/- 5% (z score 0.130). This makes the
workload fairly evenly distributed among processes.

### Dynamic workload balancer

The [dynamic balancer](dynamic_mpi.c) uses a round-robin approach to serving workloads. A single
unit is given to each worker node at a time. When the work is complete, the
node returns the finished workload to the master node with additional elapsed
time data. If there is more work to be done, the master node will send the
worker a new workload. This continues until all workloads are complete. The
master node is not a worker; it only coordinates the rest of the nodes.

A comparison of the approach of having the master node as a worker versus a
coordination-only master node (fastdynamic) can be found in [Pretty Graphs](#pretty-graphs).
The fastdynamic algorithm has `n-1` workers, while the dynamic algorithm has
`n` workers.

Known issues
============

* The static balancing algorithm could use some love and care. I am not sure the
z-score is reasonable, or if it should be scaled according to the number of
workers.

How to build
============
Use the Makefile by running `make`.

Pretty graphs
=============
![static](https://cloud.githubusercontent.com/assets/941126/6117478/c5e11c14-b082-11e4-9892-47391e886c17.png)

![dynamic](https://cloud.githubusercontent.com/assets/941126/6118192/1e2c9ece-b089-11e4-9b9b-9c29a078d744.png)

![fastdyn](https://cloud.githubusercontent.com/assets/941126/6118105/16ff6204-b088-11e4-9476-c82c70132cca.png)
