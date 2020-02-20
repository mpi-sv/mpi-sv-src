/******************************************************************************
* File      : memHandler.c                                                    *
* Author    : Carlos Rosales Fernandez (carlos@ihpc.a-star.edu.sg)            *
* Date      : 01-09-2006                                                      *
* Revision  : 1.1                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
* Defines the eight functions related to memory allocation/deallocation used  *
* in the code: doubleMatrix, doublePointer, doubleVector, freeDoubleMatrix,   *
* freeDoublePointer, freeUintMatrix, uintMatrix, uintVector                   *
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


/* These three libraries are required by every function in this file */
#include <stdlib.h>
#include <stdio.h>
#include "errorHandler.h"
#include "memHandler.h"

/******************************************************************************
* Allocate memory for a double matrix of size ROWS x COLS.                    *
* INIT == 0 : malloc function is used for the allocation.                     *
* INIT != 0 : calloc function is used, all elements are initialized to zero.  *
******************************************************************************/
double **doubleMatrix(unsigned int ROWS, unsigned int COLS, 
                       unsigned int INIT)
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

/******************************************************************************
* Allocate memory for a double pointer array of size LENGTH.                  *
* INIT == 0 : malloc function is used for the allocation.                     *
* INIT != 0 : calloc function is used, all elements are initialized to zero.  *
******************************************************************************/
double **doublePointer(unsigned int LENGTH, unsigned int INIT)
{
    double **P;
    
    if(!INIT){
        if((P = (double **)malloc(LENGTH*sizeof(double *))) == NULL)
          errorHandler("Error - In doublePointer(): Can't allocate memory");
    }
    else{
        if((P = (double **)calloc(LENGTH,sizeof(double *))) == NULL)
          errorHandler("Error - In doublePointer(): Can't allocate memory");
    }
    
    return P;   
}

/******************************************************************************
* Allocate memory for a double vector of size LENGTH.                         *
* INIT == 0 : malloc function is used for the allocation.                     *
* INIT != 0 : calloc function is used, all elements are initialized to zero.  *
******************************************************************************/
double *doubleVector(unsigned int LENGTH, unsigned int INIT)
{
    double *V;
    
    if(!INIT)
    {
        if((V = (double *)malloc(LENGTH*sizeof(double))) == NULL)
          errorHandler("Error - In doubleVector(): Can't allocate memory");
    }
    else
    {
        if((V = (double *)calloc(LENGTH,sizeof(double))) == NULL)
          errorHandler("Error - In doubleVector(): Can't allocate memory");
    }
    
    return V;   
}

/******************************************************************************
* Free memory for a dynamically allocated double matrix.                      *
******************************************************************************/
void freeDoubleMatrix(double **M, unsigned int ROWS)
{
    unsigned int i;
    
    for(i = 0; i < ROWS; i++) free(M[i]);
    free(M);
}

/******************************************************************************
* Free memory for a dynamically allocated double pointer array.               *
******************************************************************************/
void freeDoublePointer(double **P, unsigned int ROWS)
{
    unsigned int i;
    
    for(i = 0; i < ROWS; i++) free(P[i]);
    free(P);
}

/******************************************************************************
* Frees memory for a dynamically allocated unsigned int matrix.               *
******************************************************************************/
void freeUintMatrix(unsigned int **M, unsigned int ROWS)
{
    unsigned int i;
    
    for(i = 0; i < ROWS; i++) free(M[i]);
    free(M);
}

/******************************************************************************
* Allocate memory for an unsigned int matrix of size ROWS x COLS.             *
* INIT == 0  : malloc function is used for the allocation.                    *
* INIT != 0  : calloc function is used, all elements are initialized to zero. *
******************************************************************************/
unsigned int **uintMatrix(unsigned int ROWS, unsigned int COLS, unsigned int INIT)
{
    unsigned int i;
    unsigned int **M;
    
    if(!INIT){
        if((M = (unsigned int **)malloc(ROWS*sizeof(unsigned int *))) == NULL)
            errorHandler("Error - In uintMatrix(): Can't allocate memory");
        for(i = 0; i < ROWS; i++)
            if((M[i] = (unsigned int *)malloc(COLS*sizeof(unsigned int))) == NULL)
                errorHandler("Error - In uintMatrix(): Can't allocate memory");
    }
    else{
        if((M = (unsigned int **)calloc(ROWS,sizeof(unsigned int *))) == NULL)
            errorHandler("Error - In uintMatrix(): Can't allocate memory");
        for(i = 0; i < ROWS; i++)
            if((M[i] = (unsigned int *)calloc(COLS,sizeof(unsigned int))) == NULL)
                errorHandler("Error - In uintMatrix(): Can't allocate memory");
    }
    
    return M;
}

/******************************************************************************
* Allocate memory for an unsigned int array of size LENGTH.                   *
* INIT == 0  : malloc function is used for the allocation.                    *
* INIT != 0  : calloc function is used, all elements are initialized to zero. *
******************************************************************************/
unsigned int *uintVector(unsigned int LENGTH, unsigned int INIT)
{
    unsigned int *V;
    
    if(!INIT){
        if((V = (unsigned int *)malloc(LENGTH*sizeof(unsigned int))) == NULL)
            errorHandler("Error - In uintVector(): Can't allocate memory");
    }
    else{
        if((V = (unsigned int *)calloc(LENGTH,sizeof(unsigned int))) == NULL)
            errorHandler("Error - In uintVector(): Can't allocate memory");
    }
    
    return V;
}

/* End of file memHandler.c */
