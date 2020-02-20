## Makefile sous répertoire KF Ray

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@ -I../ -I../raycaster -I../models -I../loader -I../objects -I../frontend -I../textures

noinst_LIBRARIES = libmisc.a

libmisc_a_SOURCES = \
		misc.c misc.h \
		printinfo.c printinfo.h \
		image.c image.h

