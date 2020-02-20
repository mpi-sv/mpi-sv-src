#! /bin/sh
#
# Installation script for depSolver v2.1
# Default compiler is gcc with optimization level O3 flags.
# 
# v1.1                                     2008-11-10 - Carlos Rosales Fernandez

# Initialize the log file and the error file
LOG="./depSolver_install.log"
touch $LOG
ERR="./depSolver_install.err"
touch $ERR

# Record the local conditions for the compilation
MSG="======================================================================\n"
printf "$MSG" && printf "$MSG" > $LOG
MSG="Package  : depSolver-mpi\nVersion  : 1.0\n"
printf "$MSG" && printf "$MSG" >> $LOG
MSG="Date     : $(date +%d.%m.%Y)\n"
printf "$MSG" && printf "$MSG" >> $LOG
MSG="System   : $(uname -sr)\n"
MSG="======================================================================\n"
printf "$MSG" && printf "$MSG" >> $LOG
MSG="Copyright 2006, 2008 Carlos Rosales Fernandez and IHPC (A*STAR).\n"
printf "$MSG" && printf "$MSG" >> $LOG
MSG="License: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n"
printf "$MSG" && printf "$MSG" >> $LOG
MSG="This is free software: you are free to change and redistribute it.\n"
printf "$MSG" && printf "$MSG" >> $LOG
MSG="There is NO WARRANTY, to the extent permitted by law.\n"
printf "$MSG" && printf "$MSG" >> $LOG
MSG="======================================================================\n\n"
printf "$MSG" && printf "$MSG" >> $LOG

WORKDIR=$(pwd)
LOG="$WORKDIR/depSolver-mpi_install.log"
ERR="$WORKDIR/depSolver-mpi_install.err"

MSG="Starting installation...\n\n"
printf "$MSG" && printf "$MSG" >> $LOG
MSG="Working Directory : $WORKDIR\n\n"
printf "$MSG" && printf "$MSG" >> $LOG
MSG="Compiling depSolver-mpi binaries.... "
printf "$MSG" && printf "$MSG" >> $LOG
MSG="Done.\n"
cd src 2>> $ERR && make build 2>> $ERR && make clean 2>> $ERR || MSG="Failed.\n"

LOG="$WORKDIR/depSolver-mpi_install.log"
ERR="$WORKDIR/depSolver-mpi_install.err"
MSG="$MSG\nBuilding support utilities:\n\n"
printf "$MSG" && printf "$MSG" >> $LOG

MSG="Compiling break.......... "
printf "$MSG" && printf "$MSG" >> $LOG
MSG="Done."
cd ../utils/break 2>> $ERR && make build 2>> $ERR && make clean 2>> $ERR || MSG="Failed."

LOG="$WORKDIR/depSolver-mpi_install.log"
ERR="$WORKDIR/depSolver-mpi_install.err"
MSG="$MSG\nCompiling fieldPost...... "
printf "$MSG" && printf "$MSG" >> $LOG
MSG="Done."
cd ../fieldPost  2>> $ERR && make build 2>> $ERR && make clean 2>> $ERR || MSG="Failed."

LOG="$WORKDIR/depSolver-mpi_install.log"
ERR="$WORKDIR/depSolver-mpi_install.err"
MSG="$MSG\nCompiling forcePost...... "
printf "$MSG" && printf "$MSG" >> $LOG
MSG="Done."
cd ../forcePost 2>> $ERR && make build 2>> $ERR && make clean 2>> $ERR || MSG="Failed."

LOG="$WORKDIR/depSolver-mpi_install.log"
ERR="$WORKDIR/depSolver-mpi_install.err"
MSG="$MSG\nCompiling gplotFormat.... "
printf "$MSG" && printf "$MSG" >> $LOG
MSG="Done."
cd ../gplotFormat 2>> $ERR && make build 2>> $ERR && make clean 2>> $ERR || MSG="Failed."

LOG="$WORKDIR/depSolver-mpi_install.log"
ERR="$WORKDIR/depSolver-mpi_install.err"
MSG="$MSG\nCompiling meshgen-std.... "
printf "$MSG" && printf "$MSG" >> $LOG
MSG="Done."
cd ../meshgen-std 2>> $ERR && make build 2>> $ERR && make clean 2>> $ERR || MSG="Failed."

LOG="$WORKDIR/depSolver-mpi_install.log"
ERR="$WORKDIR/depSolver-mpi_install.err"
MSG="$MSG\nCompiling meshgen-vtk.... "
printf "$MSG" && printf "$MSG" >> $LOG
MSG="Done."
cd ../meshgen-vtk 2>> $ERR && make build 2>> $ERR && make clean 2>> $ERR || MSG="Failed."

LOG="$WORKDIR/depSolver-mpi_install.log"
ERR="$WORKDIR/depSolver-mpi_install.err"
MSG="$MSG\nCompiling p2b............ "
printf "$MSG" && printf "$MSG" >> $LOG
MSG="Done."
cd ../p2b 2>> $ERR && make build 2>> $ERR && make clean 2>> $ERR || MSG="Failed."

LOG="$WORKDIR/depSolver-mpi_install.log"
ERR="$WORKDIR/depSolver-mpi_install.err"
MSG="$MSG\nCompiling potPost........ "
printf "$MSG" && printf "$MSG" >> $LOG
MSG="Done."
cd ../potPost 2>> $ERR && make build 2>> $ERR && make clean 2>> $ERR || MSG="Failed."

LOG="$WORKDIR/depSolver-mpi_install.log"
ERR="$WORKDIR/depSolver-mpi_install.err"
printf "$MSG\n" && printf "$MSG\n" >> $LOG

cd "$WORKDIR" 2>> $ERR

# Check if Doxygen is present and if so install the reference manual
#LOG="$WORKDIR/depSolver-mpi_install.log"
#ERR="$WORKDIR/depSolver-mpi_install.err"
#MSG="\nGenerating Reference Guide... "
#printf "$MSG" && printf "$MSG" >> $LOG
#MSG="Done.\n"

#doxygen docs.cfg || MSG="Failed.\n"
#printf "$MSG\n" && printf "$MSG\n" >> $LOG

if [ "$MSG" = "Failed.\n" ]
then
  MSG="\n*** WARNING: Reference manual could not be generated. ***\n"
  printf "$MSG\n" && printf "$MSG\n" >> $LOG
fi

if test -s "$ERR"
then
  MSG="\n*** WARNING: Errors detected. Check depSolver-mpi_install.err. ***\n"
  printf "$MSG" && printf "$MSG" >> $LOG
  MSG="***          Installation may be incomplete.               ***\n\n"
  printf "$MSG" && printf "$MSG" >> $LOG
else
  MSG="\ndepSolver-mpi v1.0 installation completed successfully!\n\n"
  printf "$MSG" && printf "$MSG" >> $LOG
  printf "For details see depSolver-mpi_install.log.\n\n"
fi

MSG="======================================================================\n\n"
printf "$MSG" >> $LOG

