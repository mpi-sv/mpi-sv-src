# Makefile for gplotFormat v2.0
#
# COPYRIGHT & LICENSE INFORMATION
# Copyright 2006, 2008 Carlos Rosales Fernandez andIHPC (A*STAR)
# This file is part of depSolver, and it is distributed under the GNU General
# Public License version 3 or (at your option) any later version. For details
# see the acompanying file COPYING.txt or <http://www.gnu.org/licenses/>.

SHELL = /bin/sh

# Compiler
#CC = gcc

# Flags
# In Debian and Debian variants like the Ubuntu family, please add the flag
# -fno-stack-protector because libssp0-dev has a bug and will post an error
# message on return from main. This error is bogus. The code DOES NOT smash
# the stack.
LFLAGS =  -lm -O3
CFLAGS =  -c -O3

# Linking Step
gplotForm: errorHandler.o gplotForm.o
	$(CC) $(LFLAGS) errorHandler.o gplotForm.o -o gplotForm

# Compilation step
errorHandler.o : errorHandler.c
	$(CC) $(CFLAGS) errorHandler.c

# Build options
all: gplotForm

build: clean gplotForm install

install:
	mv -f ./gplotForm ../../bin

clean:
	rm -f ./*.o

