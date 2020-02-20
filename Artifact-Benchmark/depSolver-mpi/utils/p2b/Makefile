# Makefile for p2b v2.0
#
# COPYRIGHT & LICENSE INFORMATION
# Copyright 2006, 2008 Carlos Rosales Fernandez andIHPC (A*STAR)
# This file is part of depSolver, and it is distributed under the GNU General
# Public License version 3 or (at your option) any later version. For details
# see the acompanying file COPYING.txt or <http://www.gnu.org/licenses/>.

SHELL = /bin/sh

# Compiler
# CC = gcc

# Flags
# In Debian and Debian variants like the Ubuntu family, please add the flag
# -fno-stack-protector because libssp0-dev has a bug and will post an error
# message on return from main. This error is bogus. The code DOES NOT smash
# the stack.
LFLAGS =  -lm -O3
CFLAGS =  -c -O3

# Linking Step
p2b: errorHandler.o memHandler.o p2b.o
	$(CC) $(LFLAGS) errorHandler.o memHandler.o p2b.o -o p2b

# Compilation step
errorHandler.o : errorHandler.h errorHandler.c
	$(CC) $(CFLAGS) errorHandler.c

memHandler.o : errorHandler.h memHandler.h memHandler.c
	$(CC) $(CFLAGS) memHandler.c

# Build options
all: p2b

build: clean p2b install

install:
	mv -f ./p2b ../../bin
	cp -f ./bem_save.pcl ../../bin

clean:
	rm -f ./*.o

