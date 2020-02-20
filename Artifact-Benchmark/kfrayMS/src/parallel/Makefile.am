## Makefile sous répertoire KF Ray

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@ -I../ -I../objects -I../raycaster -I../misc -I../models -I../textures -I../loader


noinst_LIBRARIES = libparallel.a

libparallel_a_SOURCES = \
	master.c master.h \
	slave.c slave.h 

