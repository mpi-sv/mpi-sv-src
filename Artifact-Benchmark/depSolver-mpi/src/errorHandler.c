/******************************************************************************
* File     : errorHandler.c                                                   *
* Author   : Carlos Rosales Fernandez (carlos.rosales.fernandez(at)gmail.com) *
* Revision : 1.0 (2008-12-23)                                                 *
******************************************************************************/
/**
 * @brief Error and warning function definitions
 *
 * @file
 * Functions used for error and warning handling.
 */

/*******************************************************************************
* Copyright 2006, 2008 Carlos Rosales Fernandez and IHPC (A*STAR).             *
*                                                                              *
* This file is part of depSolver-mpi.                                          *
*                                                                              *
* depSolver-mpi is free software: you can redistribute it and/or modify it     *
* under the terms of the GNU GPL v3 or (at your option) any later version.     *
*                                                                              *
* depSolver-mpi is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS*
* FOR A PARTICULAR PURPOSE. See the GNU General Public License for details.    *
*                                                                              *
* You should have received a copy of the GNU General Public License along with *
* depSolver-mpi. If not, see <http://gnu.org/licenses/gpl.html>.               *
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include "errorHandler.h"

extern FILE *file_log;
extern int proc;

/**
 * Prints error message to stderr and the log file and exits to the system,
 * stopping the parent program.
 *
 * @param errorText : [ Input ] Error message string
 */
void errorHandler(char errorText[])
{
    fprintf(stderr,"\nP(%d) ERROR: %s\n",proc+1,errorText);
    fprintf(stderr,"*** PROGRAM TERMINATED ***\n\n");

    fprintf(file_log,"\nP(%d) ERROR: %s\n",proc+1,errorText);
    fprintf(file_log,"*** PROGRAM TERMINATED ***\n\n");
    fclose(file_log);

    MPI_Finalize();
    exit(1);
}

/**
 * Prints warning message to stderr and to the file log.
 *
 * @param warningText : [ Input ] Warning message string
 */
void warningHandler(char warningText[])
{
    fprintf(stderr,"\nWARNING: %s\n",warningText);
    fprintf(file_log,"\nWARNING: %s\n",warningText);
}
