## Makefile sous répertoire KF Ray

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@ -I../ -I../objects -I../raycaster -I../misc -I../models -I../textures


BUILT_SOURCES = parser_yy.h
AM_YFLAGS = -d

#bin_PROGRAMS = test_loader
#test_loader_SOURCES = parser_yy.y parser_lex.l
#test_loader_LDADD=-L../objects -lobjects -L../misc -lmisc -lm

noinst_LIBRARIES = libloader.a

libloader_a_SOURCES = \
	parser_yy.y parser_lex.l \
	parser_yy.c parser_yy.h \
	loader.c loader.h 

