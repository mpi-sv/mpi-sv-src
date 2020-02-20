## Makefile sous répertoire KF Ray

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@ -I../ -I../objects -I../models -I../loader -I../misc -I../textures

noinst_LIBRARIES = libraycaster.a

libraycaster_a_SOURCES = \
	intersections.c intersections.h \
	raycaster.c raycaster.h \
	reflection.c reflection.h \
	refraction.c refraction.h

