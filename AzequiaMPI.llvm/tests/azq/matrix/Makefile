###############################################################
#                                                  #
# Azequia MediaFramework                                      #
# Makefile, v1.0.0 07/05/16                                   #
###############################################################
CC=gcc
CFLAGS=-g -Wall
RM=rm
RFLAGS=-rf

# Azequia root directory
AZQROOT=../../..

INC=-I$(AZQROOT)/include

OBJ=./matrix_master.o      \
    ./matrix_slave.o       \
    ./main.o

LIB=-L$(AZQROOT)/lib          \
    -lazq -llnk -lpthread -lrt

EXE=./matrix

all : $(EXE)

$(EXE) : $(OBJ)
	$(CC) -o $(EXE) $(OBJ) $(LIB)

%.o : %.c
	$(CC) $(CFLAGS) $(INC) -c $? -o $@

clean:
	$(RM) $(RFLAGS) $(OBJ) $(EXE)
