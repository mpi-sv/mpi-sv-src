/******************************************************************************
* File      : doubleMatrix.c                                                  *
* Author    : Carlos Rosales Fernandez (carlos@ihpc.a-star.edu.sg)            *
* Date      : 01-09-2006                                                      *
* Revision  : 1.0                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
* Allocates memory for a double matrix of size ROWS x COLS.                   *
* INIT == 0	: malloc function is used for the allocation.                     *
* INIT != 0	: calloc function is used, all elements are initialized to zero.  *
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

#include <stdlib.h>
#include <stdio.h>

double **doubleMatrix(unsigned int ROWS, unsigned int COLS, unsigned int INIT)
{
    unsigned int i;
    double **M;
    
    if(!INIT){
       	if((M = (double **)malloc(ROWS*sizeof(double *))) == NULL)
       		errorHandler("Error - In doubleMatrix(): Can't allocate memory");
       	for(i = 0; i < ROWS; i++)
       		if((M[i] = (double *)malloc(COLS*sizeof(double))) == NULL)
       			errorHandler("Error - In doubleMatrix(): Can't allocate memory");
   	}
   	else{
       	if((M = (double **)calloc(ROWS,sizeof(double *))) == NULL)
       		errorHandler("Error - In doubleMatrix(): Can't allocate memory");
       	for(i = 0; i < ROWS; i++)
       		if((M[i] = (double *)calloc(COLS,sizeof(double))) == NULL)
       			errorHandler("Error - In doubleMatrix(): Can't allocate memory");
   	}
   	
   	return M;		
}
