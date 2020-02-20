#######################################################################
##   
##   Azequia Message Passing Interface (AZQMPI)
##   
##   Author: http://gsd.unex.es
##   Date:   November 6th, 2008
##
#######################################################################

# 1. Install dirs for libraries
AZQ_INSTALL_DIR=/home/jarico/packages/azq
AZQMPI_INSTALL_DIR=/home/jarico/packages/azqmpi

# 2. Libraries and headers
INCLUDE= -I$(AZQ_INSTALL_DIR)/include -I$(AZQMPI_INSTALL_DIR)/include 
LIB=-L$(AZQ_INSTALL_DIR)/lib -L$(AZQMPI_INSTALL_DIR)/lib 
LIBS_ARCH= -lazqmpi -lidsp -linet -lpthread  -lrt -lm

# 3. Compiler
CC=gcc

# 4. Options for the compiler
#OPTIONS=  -O3 -ffast-math
OPTIONS=  -w -Wall  -g  -pg
DEFINES= -DAZQ_MPI -D_POSIX_THREADS 

# 5. Executable
EXE=$(PWD)/integration
COMPILE=$(CC) 

# 6. Object files
OBJECTS=    \
integration.o    

# 7. Rules
all: $(OBJECTS)
	$(CC) -o $(EXE) $(OBJECTS) $(LIB) $(LIBS_ARCH) 

integration.o: integration.c
	$(COMPILE) $(DEFINES) $(OPTIONS) $(INCLUDE)  -c $? -o $@ 

cleanall: clean
clean:
	rm -f integration.o 
	rm -f *~
	rm -f integration


