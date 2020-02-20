#######################################################################
##   
##   Azequia Mediaframework
##   Makefile 1/10/2007
##
##   juancarl@unex.com
##
##   Test de Azequia/pequines/linux
##
#######################################################################
CC=cc
RDIR=.
#
# 1. Interfaces:
# (1) Azequia
AZQ_ROOT=/home/juancarl/Azequia/Azequia_02_Octubre_2007
AZQ_INTERFACE= -I$(AZQ_ROOT)/idsp/knl -I$(AZQ_ROOT)/idsp/knl/spec -I$(AZQ_ROOT)/idsp/net/spec -I$(AZQ_ROOT)/chn/spec 
#
# 2. Bibliotecas
# (1) Azequia
AZQ_LIB= -L$(AZQ_ROOT)/lib -lchn -lidsp -lnet -llnk
# (2) Pequines
PEQUINES_ROOT=/home/juancarl/Azequia/ponnhi/pequines/pequines_XILK_0.0
PEQUINES_LIB= -L$(PEQUINES_ROOT)/lib -lpequines  
# (3) System
SYSTEM_LIB=  -pthread  -lrt 
# 
#
EXE=$(RDIR)/adder_no_chn
OPTIONS= -c -Wall -O3 -D_DEBUG
COMPILE=$(CC) $(OPTIONS)

SOURCES= \
main.c\
adder_no_chn_operator.c

OBJECTS=\
main.o\
adder_no_chn_operator.o

nptl: $(OBJECTS)
	$(CC) -o $(EXE) $(OBJECTS) $(AZQ_LIB) $(SYSTEM_LIB) 

pequines: $(OBJECTS)
	$(CC) -o $(EXE) $(OBJECTS) $(AZQ_LIB) $(PEQUINES_LIB) $(SYSTEM_LIB) 

%.o: %.c
	$(COMPILE) $(AZQ_INTERFACE)  -c $? -o $@

clean:
	rm -f $(OBJECTS) $(EXE)

install:
