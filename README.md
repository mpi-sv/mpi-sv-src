# MPI-SV: A Symbolic Verifier for MPI Programs

**MPI-SV** is an **automatic** symbolic verifier for verifying MPI **C** programs. **MPI-SV** supports the verification of **non-blocking** MPI programs. The technique combines **symbolic execution** and **model checking** in a synergistic manner to enlarge the scope of verifiable properties and improve the scalability of verification.

For more details, please visit https://mpi-sv.github.io

****

**Noted:** This Github repository is developed based on [the Cloud9 main repository](https://github.com/dslab-epfl/cloud9).

****

# Source Installation Guide

First, we need to prepare an installation directory and build MPI-SV under this directory.

```MPISV_Install
mkdir MPISV_Install
```

# 1. [Cloud9's setting up](http://cloud9.epfl.ch/setting-up)
We implemented MPI-SV based on [Cloud9]([http://cloud9.epfl.ch/home]), which is a <font color=red>parallel symbolic execution engine</font> that scales on shared-nothing clusters of commodity hardware. It can test systems ranging from command line utilities to Internet servers and distributed systems, thanks to its support for a symbolic POSIX OS environment. Cloud9 builds upon the KLEE symbolic execution engine.

|-- **MPISV_Install** - **<font color=red>Install_ROOT</font>**. <br/>
&emsp; &emsp; &emsp;|-- **depot_tools** <br/>
&emsp; &emsp; &emsp;|-- **glog** <br/>
&emsp; &emsp; &emsp;|-- **CLOUD9** - **<font color=red>CLOUD9_ROOT</font>**.<br/>

## 1.1 Installing depot_tools  

Cloud9 is made out of components that reside in different repositories, either under DSLab administration, or outside on the Internet. The repositories are put together with the help of an utility called depot_tools, originally developed for Google's Chromium project.

- Check out the depot_tools code somewhere on your machine (for instance, in your home directory)

```
	cd MPISV_Install
	git clone https://chromium.googlesource.com/chromium/tools/depot_tools
```
- Add depot_tools to your path [**optional**]:

```
	vim ~/.bashrc
	export PATH=$PATH:$Install_ROOT/depot_tools
	source ~/.bashrc
```
## 1.2 Installing Prerequisite Packages

- Install the required Ubuntu development packages

```
	sudo add-apt-repository universe
	sudo apt-get update     	         
	sudo apt-get install dejagnu flex bison protobuf-compiler libprotobuf-dev libboost-thread-dev libboost-system-dev build-essential libcrypto++-dev git subversion autoconf m4 libtool automake boolector libgdiplus

```
- Install Google's glog library (this is for logging, Please download from [Github](https://github.com/google/glog/releases/tag/v0.3.3))

```
	cd MPISV_Install
	tar -xzvf glog-0.3.3.tar.gz 
	cd glog-0.3.3
	./configure
	make clean
	make -j3
	sudo make install
	sudo ldconfig 
```
The last command is necessary to force the linker cache to pick up the new library addition.

- Update Env for your path:

```
	vim ~/.bashrc
	export GLOG_minloglevel=0			# output the VLOG(0) other than VLOG(1)
	export GLOG_stderrthreshold=1
	export GLOG_max_log_size=1
	export GLOG_logtostderr=1			# make LOG(INFO) output to stdout
	export GLOG_v=0
	source ~/.bashrc
```

## 1.3 Checking out Cloud9
Create a fresh directory for the Cloud9 installation.

```
mkdir MPISV_Install/CLOUD9
cd CLOUD9
python2.7 ../depot_tools/gclient.py  config --name src git+https://github.com/dslab-epfl/cloud9-depot.git
sudo python2.7 ../depot_tools/gclient.py sync
```
The gclient sync command takes a lot of time. It checks out the entire Cloud9 code, and executes several one-time hooks: (1)<font color=red>Download & build</font> LLVM and Clang; (2)<font color=red>Download & build</font> a custom version of Binutils; (3)Generate project Makefiles for a subset of components. Here are the most important parts of the code structure:

|-- **src**  - Cloud9 root	<br/>
&emsp; &emsp; &emsp;|-- **build** -Build scripts (gyp wrapper, LLVM & binutils downloader, global all.gyp file)<br/>
&emsp; &emsp; &emsp;|-- **cloud9** - Symbolic execution engine (✖️)<br/> 
&emsp; &emsp; &emsp;|-- **MPISV** - Symbolic execution engine (☑️)<br/>
&emsp; &emsp; &emsp;|-- **klee-uclibc** - Modified C library to support symbolic execution <br/>
&emsp; &emsp; &emsp;|-- **third_party** - Symbolic execution engine <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **gyp** - The Gyp build system code<br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **stp** - The STP solver <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **llvm** - LLVM source code <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **llvm-build** - LLVM binaries <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **binutils** - Binutils source code <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **binutils-install** - Binutils binaries <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **...**  <br/>
&emsp; &emsp; &emsp;|-- **testing_targets** - LLVM bitcode <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **build** - common LLVM build scripts (env. setup, global gyp files) <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **...** <br/>
&emsp; &emsp; &emsp;|-- **...** <br/>


  
## 1.4 Building Cloud9
- Build Klee's uclibc

```
	cd $CLOUD9_ROOT/src/klee-uclibc
	make clean
	make -j3
```
- Build STP

```
	cd $CLOUD9_ROOT/src/third_party/stp
	./scripts/configure --with-prefix=$(pwd)
	make clean
		echo 123 > ./src/main/versionString.stamp
	    make
		# modify the files under "./third_party/stp/src/parser"
		# yyparse (void) —> yyparse (void* YYPARSE_PARAM)
		# parsecvc.cpp(line 1488), parsesmt.cpp(line 1394), parsesmt2.cpp(line 1375)]
	make -j3

```
- Build Cloud9 itself (✖️ <font color=red>replaced by MPISV</font>)

```
	cd $CLOUD9_ROOT/src
	make clean
	make -j3
	cd $CLOUD9_ROOT/src/cloud9
	./configure --with-llvmsrc=../third_party/llvm --with-llvmobj=../third_party/llvm-build --with-uclibc=../klee-uclibc --enable-posix-runtime --with-stp=../third_party/stp
	make clean
	make -j3
```
**[required]**

- Build boolector

```
	cd $CLOUD9_ROOT/src/third_party/boolector/
	./configure 
	make clean
	make
	sudo cp libboolector.a /usr/lib/
```
**[Optional, since they are built in 1.3]**

- Build binutils (10 mins)

```s
	cd $CLOUD9_ROOT/src/third_party/binutils
	./configure --prefix=$CLOUD9_ROOT/src/third_party/binutils-install --enable-gold --enable-plugins
	make clean
	make
	make install
```
- Build llvm (30 mins)

	 Solve the problem **"’'bits/c++config.h’ file not found"**

```
	sudo cp /usr/include/x86_64-linux-gnu/c++/4.8/bits/* /usr/include/c++/4.8/bits/
```


```
	cd $CLOUD9_ROOT/src/third_party/llvm-build/
	../llvm/configure --enable-optimized --enable-assertions --with-binutils-include=$CLOUD9_ROOT/src/third_party/binutils-install/include
	make clean 
	make -j2

```

# 2. Mono & PAT
## 2.1 Install [Mono2.10.8.1](https://download.mono-project.com/sources/mono/mono-2.10.8.1.tar.gz) (20 mins)

```
cd ~/
tar -xvf mono-2.10.8.1.tar.gz
cd mono-2.10.8.1
./configure
make
sudo make install
```
## 2.2 Install [PAT3.4.0](https://pat.comp.nus.edu.sg)

Please put it under the root directory like **/root** or **/home/user**

```
cd  ~/
tar -xvf PAT3.4.0.tar.gz
mv PAT3.4.0 pat
```
mono "./PAT 3.exe" for testing(after updating the env about pat as shown in 3.4).


# 3. MPISV
## 3.1 **AzequiaMPI**

We use a multi-threaded library for MPI, called AzequiaMPI, as the MPI environment model for symbolic execution. We use Clang to compile an MPI program to LLVM bytecode, which is then linked with the pre-compiled MPI library AzequiaMPI. (after updating env about clang as shown in 3.4)

```
cd $CLOUD9_ROOT/src/MPISV/AzequiaMPI.llvm/
./build.sh
./remake.sh 
```

Update Env for your path:

```
export AZQMPI_NODES=$1			# the number of processes
export AZQMPI_BYSOCKET=0
export AZQMPI_HOSTS=1			# the number of machines
export AZQMPI_NO_BINDING=0		# communicating whin network?
```



## 3.2 **some problems to fix before make:**

- Solve the problem **"’klee/data/DebugInfo.pb.h’ file not found"**

```
	cd $CLOUD9_ROOT/src/MPISV/lib/Data
	protoc --cpp_out=$(pwd) *.proto 
```
- Solve the problem about **"libboost"**

```
	cd /usr/lib/x86_64-linux-gnu
	sudo cp libboost_thread.a ./libboost_thread-mt.a
	sudo cp libboost_system.a ./libboost_system-mt.a
```
## 3.3 **make**

```
cd $CLOUD9_ROOT/src/MPISV
./configure --with-llvmsrc=../third_party/llvm --with-llvmobj=../third_party/llvm-build --with-uclibc=../klee-uclibc --enable-posix-runtime --with-stp=../third_party/stp --with-libMPI=./AzequiaMPI.llvm/lib --with-runtime=Release+Asserts CFLAGS="-g -O0" CXXFLAGS="-g -O0"
make clean
make 
```
## 3.4 **update Env**

```
vim ~/.bashrc
export PATH=$PATH:$Install_ROOT/depot_tools:$Install_ROOT/CLOUD9/src/third_party/llvm-build/Release+Asserts/bin:$Install_ROOT/CLOUD9/src/MPISV/Release+Asserts/bin:/root/pat:$Install_ROOT/CLOUD9/src/MPISV
source ~/.bashrc
```
- **\$Install\_ROOT/depot\_tools** for gclient
- **\$Install\_ROOT/CLOUD9/src/third_party/llvm-build/Release+Asserts/bin** for clang, llvm 
- **\$Install\_ROOT/CLOUD9/src/MPISV/Release+Asserts/bin** for klee
- **/root/pats** for pat
- **\$Install\_ROOT/CLOUD9/src/MPISV** for mpisvcc.sh


## 3.5 **modify mpisvcc.sh**

```
AZQROOT=$Install_ROOT/CLOUD9/src/third_party/AzequiaMPI.llvm
KLEEROOT=$Install_ROOT/CLOUD9/src/MPISV
```



# 4. Test
Here we run an example for testing.

```
cd $CLOUD9_ROOT/src/MPISV/benchmarks/dtg/
// compile an MPI program to LLVM bytecode
mpisvcc.sh dtg.c -o dtg.o
// then linked with the pre-compiled MPI library AzequiaMPI
AZQMPI_NODES=5
klee -lib-MPI -use-directeddfs-search -threaded-all-globals dtg.o
klee -lib-MPI -use-directeddfs-search -threaded-all-globals -wild-opt dtg.o
// get the CSP model
cd
geidt *.csp
```

# 5. Benchmark
|-- **MPISV** <br/>
&emsp; &emsp; &emsp;|-- **Artifact-Benchmark** <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **bitmap** <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **clustalw-mpi-0.13** <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **depSolver-mpi** <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **diffusion2d** <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **dtg** <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **gausselim** <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **heat** <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **image-manip** <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **integrate** <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **kfray-1.0.1** <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **kfray-MS** <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **Master-Slave** <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **matmat** <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **Others** - Store the script related to the data collection <br/> 
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **script-all** <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **ALL** <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **"program".sh** -- run for each program <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **collect.sh** -- collect information for each program from log file <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **run.sh** -- script for **Program-oriented** parallelism  <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **ALL_quick** (8 hours) <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **collect.sh** -- collect information for each program from log file <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **collect_all.sh** -- collect information for all program from log file <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **para_list** - the parameters for running <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **run.sh** -- script for **Task-oriented** parallelism  <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **ctrlC-detect.sh** -- detect the "ctrl-C" error <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **ctrlC-rerun.sh**  -- rerun these programs met error <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **collect-all.py** -- generate excel file <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **<font color=red>reproduce.sh</font>** -- command description <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **script-5min** <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **5_min** <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **"program".sh** <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **collect.sh** <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **run.sh** <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **5_min\_quick** (15 minutes) <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **collect.sh** <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **collect_all.sh** <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **para_list** <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **run.sh** <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **ctrlC-detect.sh** <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **ctrlC-rerun.sh** <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **collect-5min.py** <br/>
&emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp; &emsp;|-- **<font color=red>reproduce.sh</font>** <br/>


Use the following command to reproduce the result of one verification task in the paper.

```
reproduce.sh <program_name> <mutate_flag> <process_num> <time_limit> <opt_flag>
```

**\<program_name\>** : DTG, Integrate, ...

**\<mutate_flag\>** : 0, 1, 2, ...

**\<process_num\>** : 4, 6, 8, ...

**\<time_limit\>** : 3600, ..., in seconds.

**\<opt_flag\>** : 1 or 0, where 1 represents using model-checking based boosting, 0 represents using pure symbolic execution.

After a few seconds, when the command finishes, you can view the following result：

```
-----------------Benchmark Information------------------
Program_name: xxx
Mutate_flag: xxx
process_num: xxx
Time_limit: xxx
Mode: xxx

-------------------Output Information------------------
MPI-SV: totally xxx iterations
Timecost: xxx
Deadlock: xxx
```

MPI-SV generates the log file of the verification task in the following file.
```
./script-all/result_<program_name>/mut<mutate_flag>_process<process_num>_opt<opt_flag>.log
```

***

We can also run the **run.sh** in the folers **script-all** and **scrpt-5min** to reproduce the results of all programs and the programs with the verificaiton time that is less than 5 minutes. We can run the script with **n** processes in parallel (e.g., 8 processes) according to the physical machine.

```
./run.sh 8
```
* Some verification tasks may be killed by "ctrl-C" when running the above command, which will influence the experimental results. We can use the following commands to check whether the problem exists.

```
	./ctrlC-detect.sh
```
* For those verification tasks killed "ctrl-C", you can find them in the file "./script-xxx/rerun_list", which looks like:

```
	DTG 0 5 3600 0
	Integrate 1 8 3600 1
```
* Then, if there do exist the tasks killed unexpectedly, you can use the following commands to rerun these verification tasks after **setting the parameters "Nproc" (number of current tasks) and "total_mission" (total number of tasks that are killed by ctrl-C, i.e., the number of lines in rerun_list) in ctrlC-rerun.sh**.

```
	./ctrlC-rerun.sh
```

After successfully running the reproducing command (i.e., all the task are completed successfully), you can obtain the excel file containing all the results by runing the following commands.

```
python3 collect-xxx.py
```
