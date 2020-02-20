## Makefile sous répertoire KF Ray

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@ -I../ -I../raycaster -I../models -I../misc -I../loader -I../textures

noinst_LIBRARIES = libobjects.a

libobjects_a_SOURCES = \
	vector.c vector.h \
	scene.c scene.h \
	light.c light.h \
	sphere.c sphere.h \
	plan.c plan.h \
	material.c material.h

