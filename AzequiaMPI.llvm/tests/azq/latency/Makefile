#######################################################################
##   
##   Azequia Mediaframework
##   Makefile 1/10/2007
##
##   juancarl@unex.com
##
##   latency. Test de velocidad de Azequia
##
#######################################################################
CC=gcc
RDIR=.
#
# 1. Interfaces:
# (1) Azequia
#AZQ_ROOT=../../..
#AZQ_INTERFACE= -I$(AZQ_ROOT)/idsp/knl -I$(AZQ_ROOT)/idsp/knl/spec -I$(AZQ_ROOT)/idsp/net/spec
AZQ_INTERFACE = -I$(AZQ_INSTALL_DIR)/include

# (2) OSI
OSI_ROOT=/home/juancarl/Azequia/osi
OSI_INTERFACE= -D__OSI -I$(OSI_ROOT)/spec

#
# 2. Libraries
# (1) Azequia
#AZQ_LIB= -L$(AZQ_ROOT)/lib -lidsp -lnet -llnk
AZQ_LIB= -L$(AZQ_INSTALL_DIR)/lib -lidsp -lnet -llnk 

# (2) OSI
OSI_LIB= -L$(OSI_ROOT)/lib -losi

# (3) System
SYSTEM_LIB= -lpthread -lrt 
# 
#
EXE=$(RDIR)/latency
#-D_DEBUG
OPTIONS= -Wall -g
#-Wall -O3 -ffast-math
COMPILE=$(CC) 
SOURCES= \
main.c\
latency.c
OBJECTS=\
main.o\
latency.o


%.o: %.c
	$(COMPILE) $(AZQ_INTERFACE)  -c $? -o $@

all: nptl

nptl: $(OBJECTS)
	$(CC) $(OPTIONS) -o $(EXE) $(OBJECTS) $(AZQ_INTERFACE) $(AZQ_LIB) $(SYSTEM_LIB) 

osi:
	$(CC) $(OPTIONS) -o $(EXE) $(OBJECTS) $(OSI_INTERFACE) $(AZQ_INTERFACE)  $(AZQ_LIB) $(OSI_LIB) $(SYSTEM_LIB) 

cleanall: clean

clean:
	rm -f $(OBJECTS) $(EXE)

install:
