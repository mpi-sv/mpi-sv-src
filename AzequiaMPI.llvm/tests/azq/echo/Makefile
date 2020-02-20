#######################################################################
##   
##   Azequia Mediaframework
##   Makefile 12/02/2008
##
##   juancarl@unex.com
##
##   radiator. Test of wait related primitives
##
#######################################################################
CC=cc
RDIR=.
#
# 1. Interfaces:
# (1) Azequia
AZQ_ROOT=../..
AZQ_INTERFACE= -I$(AZQ_ROOT)/idsp/knl -I$(AZQ_ROOT)/idsp/knl/spec -I$(AZQ_ROOT)/idsp/net/spec

# (2) OSI
OSI_ROOT=/home/juancarl/Azequia/osi
OSI_INTERFACE= -D__OSI -I$(OSI_ROOT)/spec

#
# 2. Libraries
# (1) Azequia
AZQ_LIB= -L$(AZQ_ROOT)/lib -lidsp -lnet -llnk

# (2) OSI
OSI_LIB= -L$(OSI_ROOT)/lib -losi

# (3) System
SYSTEM_LIB= -lpthread -lrt 
# 
#
EXE=$(RDIR)/echo
OPTIONS= -c -Wall -D_DEBUG -O3 
COMPILE=$(CC) $(OPTIONS)
SOURCES= \
main.c\
echoClient.c\
echoServer.c
OBJECTS=\
main.o\
echoClient.o\
echoServer.o

nptl:
	$(CC) -o $(EXE) $(SOURCES) $(AZQ_INTERFACE) $(AZQ_LIB) $(SYSTEM_LIB) 

osi:
	$(CC) -o $(EXE) $(SOURCES) $(OSI_INTERFACE) $(AZQ_INTERFACE) $(AZQ_LIB) $(OSI_LIB) $(SYSTEM_LIB) 

clean:
	rm -f $(OBJECTS) $(EXE)

install:
