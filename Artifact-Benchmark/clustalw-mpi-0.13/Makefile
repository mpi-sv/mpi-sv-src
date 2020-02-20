install: clustalw-mpi

clean:
	/bin/rm -f *.o

OBJECTS = clustalw-mpi.o interface.o sequence.o showpair.o malign.o malign_mpi_progressive.o \
        malign_mpi_pdiff.o util.o trees.o gcgcheck.o \
	prfalign_mpi_pdiff.o prfalign_mpi_progressive.o pairalign_new.o \
  	calcgapcoeff.o calcprf1.o calcprf2.o calctree.o \
        readmat.o alnscore.o random.o parallel_compare.o \
        prfalign.o stupid.o

HEADERS = general.h clustalw.h

CC	= mpicc 

TREES_FLAG = -DSERIAL_NJTREE

#
# Choose either dynamic or static scheduling for pairalign():
# the default is to use dynamic scheduling. You might also
# want to check PAIRALIGN_NCHUNK in clustalw.h. The
# larger this marco, the smaller the chunk size, default is 100.
#
PAIRALIGN_FLAG = -DDYNAMIC_SCHEDULING_PAIRALIGN 
#PAIRALIGN_FLAG = -DSTATIC_SCHEDULING_PAIRALIGN

CFLAGS  = -c -O3
#CFLAGS  = -c -O3 -funroll-all-loops
LFLAGS	= -lm 

clustalw-mpi: $(OBJECTS) 
	$(CC) -o $@ $(OBJECTS) $(LFLAGS)

interface.o : interface.c $(HEADERS) param.h
	$(CC) $(CFLAGS) $*.c

readmat.o : readmat.c $(HEADERS) matrices.h
	$(CC) $(CFLAGS) $*.c

trees.o : trees.c $(HEADERS) dayhoff.h
	$(CC) $(TREES_FLAG) $(CFLAGS) $*.c

pairalign_new.o : pairalign_new.c $(HEADERS)
	$(CC) $(PAIRALIGN_FLAG) $(CFLAGS) $*.c
parallel_compare.o : parallel_compare.c $(HEADERS) dayhoff.h
	$(CC) $(PAIRALIGN_FLAG) $(CFLAGS) $*.c



.c.o :
	$(CC) $(CFLAGS) $?

