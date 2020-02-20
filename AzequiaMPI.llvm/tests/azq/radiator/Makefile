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
AZQ_ÏNSTALL_DIR= /home/jarico/packages/azq

INC= -I$(AZQ_ÏNSTALL_DIR)/include 
INC_LIB= -L$(AZQ_ÏNSTALL_DIR)/lib 
LIB=  -lidsp -linet  -lrt -lpthread 
EXE=./radiator
OPTIONS= -Wall -O3 --fast-math 
#OPTIONS= -Wall -g -pg
DEFINES= -D__DEBUG -D__CHECK
#-D__SCHED_FIFO  -D__LOCKALL  
COMPILE=$(CC) $(DEFINES) $(OPTIONS)


SOURCES= \
main.c   \
radiator.c

OBJECTS= \
main.o   \
radiator.o

all: $(OBJECTS)
	$(CC) -o $(EXE) $(OBJECTS) $(INC_LIB) $(LIB)  

%.o: %.c
	$(COMPILE) $(INC)  -c $? -o $@

clean:
	rm -f *.o
	rm -f radiator
	rm -f *~

install:

