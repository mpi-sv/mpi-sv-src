#######################################################################
##
##   Azequia Mediaframework 
##
##  Examples to test the Azequia functionality
##
##  Author: http://gsd.unex.es
##  Date:   November 5th, 2008
##
#######################################################################

CC= gcc
AZQ_INSTALL_DIR= /home/paco/shared/azq

INC= -I$(AZQ_INSTALL_DIR)/include 
INC_LIB= -L$(AZQ_INSTALL_DIR)/lib 
LIB=  -lidsp -linet  -lrt -lpthread 
EXE=./main
OPTIONS= -Wall -O3 -ffast-math
#OPTIONS= -Wall -g -pg
DEFINES= 
#-D__SCHED_FIFO  -D__LOCKALL  -D__DEBUG -D__CHECK
COMPILE=$(CC) $(DEFINES) $(OPTIONS)


SOURCES= \
main.c   \
operator.c

OBJECTS= \
main.o   \
operator.o

all: $(OBJECTS)
	$(CC) -o $(EXE) $(OBJECTS) $(INC_LIB) $(LIB)  

%.o: %.c
	$(COMPILE) $(INC)  -c $? -o $@

clean:
	rm -f *.o
	rm -f sr
	rm -f *~

install:

