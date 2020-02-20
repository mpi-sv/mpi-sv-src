#AM_CFLAGS = -I$(top_srcdir)/idsp/$(modeldir)/impl -I$(top_srcdir)/idsp/$(modeldir)/spec -I$(top_srcdir)/inet -I$(top_srcdir)/$(devicedir)/spec -I$(top_srcdir)/$(devicedir)/impl -I$(hwloc_root)/include -I$(osi_root)/include -I$(top_srcdir)/pmi/simple -I$(top_srcdir)/azqmpi/include
AM_CFLAGS = -I$(top_srcdir)/pmi -I$(top_srcdir)/pmi/simple -I$(top_srcdir)/arch/include -I$(top_srcdir)/idsp/$(modeldir)/impl -I$(top_srcdir)/idsp/$(modeldir)/spec -I$(top_srcdir)/inet -I$(top_srcdir)/$(devicedir)/spec -I$(top_srcdir)/$(devicedir)/impl -I$(hwloc_root)/include -I$(top_srcdir)/azqmpi/include

lib_LIBRARIES           = lib/libazqmpi.a
lib_libazqmpi_adir      = $(includedir)
noinst_HEADERS          =
lib_libazqmpi_a_HEADERS =
lib_libazqmpi_a_SOURCES =
EXTRA_DIST              =

include config/Makefile.am

include pmi/Makefile.am
include pmi/simple/Makefile.am

include arch/include/Makefile.am
include arch/src/Makefile.am


# Warning: include's paths should not include any variable because
# automake does not correctly generate the Makefile.in. A bad example:
# include idsp/$(modeldir)/impl/Makefile.am. Use instead conditionals.
if WANT_USE_BLK
include idsp/blk/spec/Makefile.am
include idsp/blk/impl/Makefile.am
endif

if WANT_USE_LFQ
include idsp/lfq/spec/Makefile.am
include idsp/lfq/impl/Makefile.am
endif


include inet/Makefile.am

if WANT_USE_TCPIP
include inet/tcpip/spec/Makefile.am
include inet/tcpip/impl/Makefile.am
endif

if WANT_USE_UDPIP
include inet/udpip/spec/Makefile.am
include inet/udpip/impl/Makefile.am
endif

if WANT_USE_NONE
include inet/none/spec/Makefile.am
include inet/none/impl/Makefile.am
endif


if WANT_BUILD_AZQMPI
include azqmpi/include/Makefile.am
include azqmpi/src/attr/Makefile.am
include azqmpi/src/bindings/f77/Makefile.am
include azqmpi/src/coll/Makefile.am
include azqmpi/src/comn/Makefile.am
include azqmpi/src/dtype/Makefile.am
include azqmpi/src/errhnd/Makefile.am
include azqmpi/src/group/Makefile.am
include azqmpi/src/misc/Makefile.am
include azqmpi/src/private/Makefile.am
include azqmpi/src/pt2pt/Makefile.am
include azqmpi/src/topol/Makefile.am

#Herman added hooker of apis for mpise
include azqmpi/src/hook/Makefile.am
include tests/azqmpi/Makefile.am

endif

#SUBDIRS = tests/azqmpi
#EXTRA_DIST = inet util doc
#DIST_SUBDIRS = $(top_srcdir)/doc $(top_srcdir)/util

install-exec-hook:
if WANT_BUILD_AZQMPI
	mkdir -p $(bindir)

	cp $(top_srcdir)/bin/g2tsd $(bindir)
	cp $(top_srcdir)/bin/mpicc $(bindir)
	cp $(top_srcdir)/bin/mpicc_compilar $(bindir)

	for cmd in mpdallexit.py mpdboot.py mpdlib.py mpdman.py mpd.py mpdtrace.py mpiexec.py mpdgdbdrv.py ; do 	\
		if [ -s $(top_srcdir)/$$cmd ] ; then continue ; fi ;	\
		sed -e 's#/usr/bin/env python.*#/usr/bin/env ${PYTHON}#' $(top_srcdir)/bin/$$cmd > $(bindir)/temp$$cmd ;	\
		$(INSTALL) -m 755 $(bindir)/temp$$cmd $(bindir)/$$cmd ;	\
		(rm -f $(bindir)/temp$$cmd > /dev/null 2>&1 || exit 0) ;	\
		(cd $(bindir) && ln -sf $$cmd $${cmd%\.*}) ;	\
	done

	sed -e 's#AZQMPI_ROOT=.*#AZQMPI_ROOT="${prefix}"#'	\
			-e 's#HWLOC_ROOT=.*#HWLOC_ROOT="${hwloc_root}"#'	\
			-e 's#LIBS=.*#LIBS="-lazqmpi ${LIBS}"#' $(bindir)/mpicc > $(bindir)/tempmpicc
	$(INSTALL) -m 755 $(bindir)/tempmpicc $(bindir)/mpicc
	(rm -f $(bindir)/tempmpicc > /dev/null 2>&1 || exit 0)
	sed -e 's#.*envToSend\['\''LD_LIBRARY_PATH'\''\] =.*#        envToSend\['\''LD_LIBRARY_PATH'\''\] = "${hwloc_root}/lib"#'	\
		$(bindir)/mpiexec.py > $(bindir)/tempmpiexec.py
	$(INSTALL) -m 755 $(bindir)/tempmpiexec.py $(bindir)/mpiexec.py
	(rm -f $(bindir)/tempmpiexec.py > /dev/null 2>&1 || exit 0)
else
	mv $(libdir)/libazqmpi.a $(libdir)/libazq.a
endif

dist-hook:
	cp -rp $(top_srcdir)/bin $(distdir)
	cp -rp $(top_srcdir)/doc $(distdir)
	cp -rp $(top_srcdir)/tests/azq $(distdir)/tests
	cp -rp $(top_srcdir)/util $(distdir)
