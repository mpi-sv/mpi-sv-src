#######################################################################
##   
##   Azequia Mediaframework
##   
##  LNK level for supporting communications in Azequia
##
##  Author: http://gsd.unex.es
##  Date:   February 7th, 2009
##
#######################################################################

INCLUDE=-I../spec  -I. -I../.. -I../../../idsp/spec -I$(PMI_DIR)
COMPILE=$(CC) $(OPTIONS)

AR=ar
AR_OPTIONS=rvs
ARLIB=libinet.a
ARCHIVE=$(AZQ_ROOT)/lib/$(ARLIB)

SOURCES=        \
routinginit.c   \
elb.c           \
inet_tcp.c       

OBJECTS=        \
routinginit.o   \
elb.o           \
inet_tcp.o       


$(ARCHIVE): $(OBJECTS)
	$(AR) $(AR_OPTIONS) $(ARCHIVE) $?

%.o: %.c
	$(COMPILE) $(INCLUDE) -c $? -o $@

clean:
	rm -f $(OBJECTS)
	rm -f *~

install:

