# Makefile for filedPost v2.0
#
# COPYRIGHT & LICENSE INFORMATION
# Copyright 2006, 2008 Carlos Rosales Fernandez andIHPC (A*STAR)
# This file is part of depSolver, and it is distributed under the GNU General
# Public License version 3 or (at your option) any later version. For details
# see the acompanying file COPYING.txt or <http://www.gnu.org/licenses/>.

SHELL = /bin/sh

# Compiler
#CC = clang

# Flags
# In Debian and Debian variants like the Ubuntu family, please add the flag
# -fno-stack-protector because libssp0-dev has a bug and will post an error
# message on return from main. This error is bogus. The code DOES NOT smash
# the stack.
LFLAGS =  -lm -O3
CFLAGS =  -c -O3

# Linking Step
fieldPost: errorHandler.o doubleMatrix.o freeDoubleMatrix.o fieldPost.o
	$(CC) $(LFLAGS) errorHandler.o doubleMatrix.o freeDoubleMatrix.o \
                        fieldPost.o -o fieldPost

# Compilation step
doubleMatrix.o : errorHandler.c doubleMatrix.c
	$(CC) $(CFLAGS) doubleMatrix.c 

errorHandler.o : errorHandler.c
	$(CC) $(CFLAGS) errorHandler.c

freeDoubleMatrix.o : errorHandler.c freeDoubleMatrix.c
	$(CC) $(CFLAGS) freeDoubleMatrix.c

# Build options
all: fieldPost

build: clean fieldPost install

install:
	mv -f ./fieldPost ../../bin

clean:
	rm -f ./*.o

