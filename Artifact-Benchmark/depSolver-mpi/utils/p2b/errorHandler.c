/******************************************************************************
* File      : errorHandler.c                                                  *
* Author    : Carlos Rosales Fernandez (carlos@ihpc.a-star.edu.sg)            *
* Date      : 01-09-2006                                                      *
* Revision  : 1.2                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
* Error and warning functions                                                 *
******************************************************************************/

/*******************************************************************************
* Copyright 2006, 2008 Carlos Rosales Fernandez and IHPC (A*STAR).             *
*                                                                              *
* This file is part of depSolver.                                              *
*                                                                              *
* depSolver is free software: you can redistribute it and/or modify it under   *
* the terms of the GNU GPL version 3 or (at your option) any later version.    *
*
* depSolver is distributed in the hope that it will be useful, but WITHOUT ANY *
* WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS    *
* FOR A PARTICULAR PURPOSE. See the GNU General Public License for details.    *
*                                                                              *
* You should have received a copy of the GNU General Public License along with *
* depSolver in the file COPYING.txt. If not, see <http://www.gnu.org/licenses/>*
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "errorHandler.h"

/******************************************************************************
* Function: errorHandler                                                      *
* Prints error message and exits to the system, stopping the parent program.  *
******************************************************************************/
void errorHandler(char errorText[])
{
    fprintf(stderr,"\nERROR: %s\n",errorText);
    fprintf(stderr,"*** PROGRAM TERMINATED ***\n\n");

    exit(1);
}

