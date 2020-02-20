## Makefile sous répertoire KF Ray

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@ -I../ -I../loader -I../raycaster -I../misc -I../objects -I../textures

noinst_LIBRARIES = libmodels.a

libmodels_a_SOURCES = \
	brdf.c brdf.h \
	effects.c effects.h \
	postprocessing.c postprocessing.h

