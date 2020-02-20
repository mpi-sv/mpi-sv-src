/******************************************************************************
* File     : memHandler.c                                                     *
* Author   : Carlos Rosales Fernandez (carlos.rosales.fernandez(at)gmail.com) *
* Revision : 1.0 (2008-12-23)                                                 *
******************************************************************************/
/**
 * @brief Functions related to memory allocation and deallocation
 *
 * @file
 * Defines the eight functions related to memory allocation/deallocation used
 * in the code: doubleMatrix, doublePointer, doubleVector, freeDoubleMatrix,
 * freeDoublePointer, freeUintMatrix, uintMatrix, uintVector.
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


/* These three libraries are required by every function in this file */
#include <stdlib.h>
#include <stdio.h>
#include "errorHandler.h"
#include "memHandler.h"

/**
 * Allocate memory for a double matrix of size \a ROWS x \a COLS.
 * The memory is allocated in a contiguous block so that it can be 
 * treated in the code in exactly the same manner as a statically
 * allocated matrix.
 *
 * \a INIT == 0 : use malloc for the allocation. \n
 * \a INIT != 0 : use calloc, all elements are initialized to zero.
 */
 double **doubleMatrix(unsigned int ROWS,
                      unsigned int COLS,
                      unsigned int INIT)
{
    unsigned int i;
    double *memBlock;
    double **M;
    
    if( INIT == 0 ){
        /* Allocate contiguous memory block */
        if( (memBlock = (double *)malloc( ROWS*COLS*sizeof(double) )) == NULL )
            errorHandler("In doubleMatrix(): Can't allocate memory block");
            
        /* Allocate memory for the pointers to each row */
        if( (M = (double **)malloc( ROWS*sizeof(double *) )) == NULL )
            errorHandler("In doubleMatrix(): Can't allocate matrix pointers");
            
        /* Set pointers to each block using memBlock array */
        for(i = 0; i < ROWS; i++)
            M[i] = &memBlock[i*COLS];
    }
    else{
        /* Allocate contiguous memory block */
        if( (memBlock = (double *)calloc( ROWS*COLS, sizeof(double) )) == NULL )
            errorHandler("In doubleMatrix(): Can't allocate memory block");
            
        /* Allocate memory for the pointers to each row */
        if( (M = (double **)calloc( ROWS, sizeof(double *) )) == NULL )
            errorHandler("In doubleMatrix(): Can't allocate matrix pointers");
            
        /* Set pointers to each block using memBlock array */
        for(i = 0; i < ROWS; i++)
            M[i] = &memBlock[i*COLS];
    }
    
    return M;   
}

/**
 * Allocate memory for a double pointer array of size \a LENGTH.
 *
 * \a INIT == 0 : use malloc for the allocation. \n
 * \a INIT != 0 : use calloc, all elements are initialized to zero.
 */
double **doublePointer(unsigned int LENGTH,
                       unsigned int INIT)
{
    double **P;
    
    if( INIT == 0 ){
        if( (P = (double **)malloc(LENGTH*sizeof(double *))) == NULL )
          errorHandler("In doublePointer(): Can't allocate memory");
    }
    else{
        if( (P = (double **)calloc(LENGTH,sizeof(double *))) == NULL )
          errorHandler("In doublePointer(): Can't allocate memory");
    }
    
    return P;   
}

/**
 * Allocate memory for a double vector of size \a LENGTH.
 *
 * \a INIT == 0 : use malloc for the allocation. \n
 * \a INIT != 0 : use calloc, all elements are initialized to zero.
 */
double *doubleVector(unsigned int LENGTH,
                     unsigned int INIT)
{
    double *V;
    
    if( INIT == 0 ) // bug fix by zhenbang from (INIT = 0)
    {
        if( (V = (double *)malloc(LENGTH*sizeof(double))) == NULL )
          errorHandler("In doubleVector(): Can't allocate memory");
    }
    else
    {
        if( (V = (double *)calloc(LENGTH,sizeof(double))) == NULL )
          errorHandler("In doubleVector(): Can't allocate memory");
//        for (int i = 0; i < LENGTH; i++){
//            printf("-----------%f----------a[%d]--\n", V[i], i);
//        }
    }
    
    return V;   
}

/**
 * Free memory for a dynamically allocated double matrix \a M.
 */
void freeDoubleMatrix(double **M,
                      unsigned int ROWS)
{
    unsigned int i;

    // BUG fix by zhenbang to avoid invalid free
    free(M[0]);
    /*for(i = 0; i < ROWS; i++) {
        free(M[i]);
    }*/
    free(M);
}

/**
 * Free memory for a dynamically allocated double pointer array \a P.
 */
void freeDoublePointer(double **P,
                       unsigned int ROWS)
{
    unsigned int i;

    // BUG fix by zhenbang to avoid invalid free
    free(P[0]);
    //for(i = 0; i < ROWS; i++) free(P[i]);
    free(P);
}

/**
 * Free memory for a dynamically allocated unsigned int matrix \a M.
 */
void freeUintMatrix(unsigned int **M,
                    unsigned int ROWS)
{
    unsigned int i;

    free(M[0]); // added by zhenbang to avoid invalid free exception
//    for(i = 0; i < ROWS; i++) free(M[i]); // comment to avoid free exception
    free(M);
}

/**
 * Allocate memory for an unsigned int matrix of size \a ROWS x \a COLS.
 * The memory is allocated in a contiguous block so that it can be 
 * treated in the code in exactly the same manner as a statically
 * allocated matrix.
 *
 * \a INIT == 0 : use malloc for the allocation. \n
 * \a INIT != 0 : use calloc, all elements are initialized to zero.
 */
unsigned int **uintMatrix(unsigned int ROWS,
                          unsigned int COLS,
                          unsigned int INIT)
{
    unsigned int i;
    unsigned int *memBlock;
    unsigned int **M;
    
    if( INIT == 0 ){
        /* Allocate contiguous memory block */
        if( (memBlock = (unsigned int *)malloc( ROWS*COLS*sizeof(unsigned int) )) == NULL )
            errorHandler("In uintMatrix(): Can't allocate memory block");
            
        /* Allocate memory for the pointers to each row */
        if( (M = (unsigned int **)malloc( ROWS*sizeof(unsigned int *) )) == NULL )
            errorHandler("In uintMatrix(): Can't allocate matrix pointers");
            
        /* Set pointers to each block using memBlock array */
        for(i = 0; i < ROWS; i++)
            M[i] = &memBlock[i*COLS];
    }
    else{
        /* Allocate contiguous memory block */
        if( (memBlock = (unsigned int *)calloc( ROWS*COLS, sizeof(unsigned int) )) == NULL )
            errorHandler("In uintMatrix(): Can't allocate memory block");
            
        /* Allocate memory for the pointers to each row */
        if( (M = (unsigned int **)calloc( ROWS, sizeof(unsigned int *) )) == NULL )
            errorHandler("In uintMatrix(): Can't allocate matrix pointers");
            
        /* Set pointers to each block using memBlock array */
        for(i = 0; i < ROWS; i++)
            M[i] = &memBlock[i*COLS];
    }
   
    return M;
}

/**
 * Allocate memory for an unsigned int array of size \a LENGTH.
 * \a INIT == 0 : use malloc for the allocation. \n
 * \a INIT != 0 : use calloc, all elements are initialized to zero.
 */
unsigned int *uintVector(unsigned int LENGTH,
                         unsigned int INIT)
{
    unsigned int *V;
    
    if( INIT == 0 ){
        if( (V = (unsigned int *)malloc(LENGTH*sizeof(unsigned int))) == NULL )
            errorHandler("In uintVector(): Can't allocate memory");
    }
    else{
        if( (V = (unsigned int *)calloc(LENGTH,sizeof(unsigned int))) == NULL )
            errorHandler("In uintVector(): Can't allocate memory");
    }
    
    return V;
}

/* End of file memHandler.c */
