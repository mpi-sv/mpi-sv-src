## Makefile KF-Ray
## Process this file with automake to produce Makefile.in

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@

# Sous-repertoires
SUBDIRS = misc objects loader models raycaster textures parallel frontend

bin_PROGRAMS = kfray 

kfray_SOURCES = \
	main.c main.h \
	arguments.c arguments.h \
	raytracer.c raytracer.h

kfray_LDADD = @PACKAGE_LIBS@ $(INTLLIBS) -Lobjects -Lraycaster -Lmodels -Lmisc -Lloader -Lparallel -Ltextures \
					 -lraycaster -lobjects -lmodels -lmisc -lloader -lparallel -ltextures

#kfray_gui_LDADD = @PACKAGE_LIBS@ $(INTLLIBS) -Lmisc -Lfrontend \
#	-lmisc -lfrontend

# Pour LAM MPI ?
# -L/usr/lib/lam -lmpi -llam -lutil
