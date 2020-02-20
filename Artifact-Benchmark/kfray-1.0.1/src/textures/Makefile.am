## Makefile sous répertoire KF Ray

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@ -I../ -I../loader -I../raycaster -I../misc -I../objects -I../models

noinst_LIBRARIES = libtextures.a

libtextures_a_SOURCES = \
	perlin.c perlin.h \
	procedural.c procedural.h 
