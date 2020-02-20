## Makefile pour KF-Ray GUI

NAME    = kfray_gui
SRC     = callbacks.c interface.c main_gui.c support.c functions.c
INCL    = callbacks.h  interface.h  main_gui.h  support.h functions.h

IPATH   = -I. -I.. -I../.. -DHAVE_CONFIG_H -DPACKAGE_DATA_DIR=\""/usr/local/share"\" -DPACKAGE_LOCALE_DIR=\""/usr/local/share/locale"\" `pkg-config gtk+-2.0  --cflags` 
OBJOPT  = -O2 -Wall 
EXEOPT  = -O2 -Wall
LPATH   = -L. -lm `pkg-config gtk+-2.0  --libs` 

CC      = gcc
CXX     = g++
MAKE    = make
SHELL   = /bin/sh
OBJS    = $(SRC:.c=.o)
RM      = /bin/rm -f

CFLAGS  = $(OBJOPT) $(IPATH)
LDFLAGS = $(EXEOPT) $(LPATH)

all:    $(NAME)
#		ln -s `pwd`/kfray_gui ../
		cp kfray_gui ../
		
$(NAME): $(OBJS) $(SRC) $(INCL)                                     
	$(CC) $(OBJS) $(LDFLAGS) -o $(NAME)

clean:                                                
	$(RM) $(NAME) $(OBJS) *~

depend:
	gcc $(IPATH) -MM $(SRC)
