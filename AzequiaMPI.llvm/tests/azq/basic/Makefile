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
AZQ_INSTALL_DIR= $(HOME)/packages/azq

INC= -I$(AZQ_INSTALL_DIR)/include 
INC_LIB= -L$(AZQ_INSTALL_DIR)/lib 
LIB=  -lidsp -linet  -lpthread 
EXE=./basic
#OPTIONS= -Wall -O3 --fast-math
OPTIONS= -Wall -g 
#DEFINES= -D_POSIX_THREADS -D__XPN_PRINT -D_DEBUG
COMPILE=$(CC) $(DEFINES) $(OPTIONS)


SOURCES= \
main.c   \
basic.c

OBJECTS= \
main.o   \
basic.o

all: $(OBJECTS)
	$(CC) -o $(EXE) $(OBJECTS) $(INC_LIB) $(LIB)  

%.o: %.c
	$(COMPILE) $(INC)  -c $? -o $@

clean:
	rm -f *.o
	rm -f basic

install:

