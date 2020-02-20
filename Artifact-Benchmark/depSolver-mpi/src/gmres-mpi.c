/******************************************************************************
* File     : gmres-mpi.c                                                      *
* Author   : Carlos Rosales Fernandez (carlos.rosales.fernandez(at)gmail.com) *
* Revision : 1.0 (2008-12-23)                                                 *
******************************************************************************/
/**
 * @brief General Minimal RESidual iterative solver for linear systems
 *
 * @file
 * Definitions of the six functions involved in the iterative solution of the
 * BEM linear system of equations using the General Minimal RESidual method:
 * solverGMRES_el, iterGMRES_el, initRes_el, dotProd, matVectProd_el, L2Norm.
 * This "_el" version has been modified to use a [nNodes]x[2nNodes] matrix.
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
#include <math.h>
#include <mpi.h>
#include "errorHandler.h"
#include "gmres-mpi.h"
#include "memHandler.h"

/*
   Define the type of GMRES run
   GMRES_RESTART = 0 : not activated
   GMRES_RESTART = 1 : activated
*/
#define GMRES_RESTART   0
#if GMRES_RESTART
    #define MAX_OUTER_LOOP  10
    #define MAX_ITERS       20
#else
    #define MAX_OUTER_LOOP  1
    #define MAX_ITERS       40
#endif
#define TOLERANCE 1.0E-6

FILE *fg;
extern unsigned int nNodes;
extern int          nodeMin, nodeMax, localNodes, localSize;
extern int          nprocs, proc;
extern const int    master;

/**
 * Performs the iterative solving of the linear system \a Ax=B using
 * GMRES. This "_el" version has been modified to use a [nNodes]x[2nNodes]
 * input coefficient matrix. The independent coefficient vector is overwritten
 * and contains the solution vector on output.
 *
 * @param preCond : [ Input ] Use Jacobi preconditioner (1) or not (0)
 * @param nInit   : [ Input ] Use given initial guess (!=0) or not (0)
 * @param A : [ Input ] Coefficient matrix for the linear problem
 * @param B : [ Input ] Independent coefficients; [ Output ] Solution vector
 */
void solverGMRES_el(unsigned int preCond,
                    unsigned int nInit,
                    double **A,
                    double *B,
                    int *procSize,
                    int *procFirst,
                    int *procNodes)
{
    FILE *fin;
    unsigned int col, i, iter1 = 0, iter2 = 0, j, convergence = 1, size;
    double tolerance = 1.0, diag;
    double *Btmp, *x;

    fg   = fopen("gmres.log","w");
    size = 2*nNodes;
    x    = doubleVector(size,1);
    Btmp = doubleVector(localSize,1);

    if(nInit != 0){
        fin = fopen("solution.init","r");
        for(i = 0; i < nInit; i++) fscanf(fin,"%le",&x[i]);
        fclose(fin);
    }

    /* Use Jacobi Preconditioner */
    if(preCond == 1){
        for(i = nodeMin; i <= nodeMax; i++){
            col = i - nodeMin;
            diag = 1.0/A[col][2*i];
            Btmp[2*col] = B[2*i]*diag;
            /* B[i] = B[i]*diag; */
            for(j = 0; j < size; j++) A[col][j] = diag*A[col][j];
        }
        /* Make sure all CPUs have the complete B array */
        MPI_Allgatherv(Btmp, localSize, MPI_DOUBLE, B, procSize, procFirst, MPI_DOUBLE, MPI_COMM_WORLD);
    }

    /* Set the tolerance w.r.t. the magnitute of the B */
    tolerance = L2Norm(size,B)*TOLERANCE;

    printf("-----------tolerance is-------------------%f-------------------------", tolerance);
    /* Printing the convergence performance of the solution progress */
    if( proc == master){
        fprintf(fg,"\n\nRELATIVE TOLERANCE = %10.4e\n", tolerance);
        fprintf(fg,"\n%10s%21s\n", "ITER #    ", "RELATIVE RESIDUAL");
        fprintf(fg,"%10s%21s\n", "***************", "*******************");
        fclose(fg);
        fg = fopen("gmres.log","a");
    }

    /* Iterations start */
    for(i = 1; i <= MAX_OUTER_LOOP; i++){
        iter1++;
        iter2 = 0;

        /* Solving the linear system here */
        convergence = iterGMRES_el(size,iter1,&iter2,tolerance,A,x,B,procSize,procFirst,procNodes);

        if(convergence == 0){
            if( proc == master )
                    fprintf(fg,"\n\n***PROBLEM CONVERGES IN %d ITERATIONS***\n\n", MAX_ITERS*(iter1-1)+iter2);

            /* Return solution in B */    
            for(i = 0; i < size; i++) B[i] = x[i];            
            free(x);
            break;
        }
    }

    if(convergence == 1){
        if( proc == master ){
            fprintf(fg,"Problem fails to vonverge to given tolerance in %d iterations\n\n",MAX_OUTER_LOOP*MAX_ITERS);
            errorHandler("Linear system may be ill-conditioned");
        }
    }
    if( proc == master ) fclose(fg);
}

/**
 * Iterative solution of the linear system \a Ax=B using GMRES. This "_el"
 * version has been modified to use a [nNodes]x[2nNodes] matrix. Some of the key
 * internal variables used in this function are:
 *
 * \a c : cosine in the Givens Rotation \n
 * \a s : sine in the Givens Rotation \n
 * \a h : Hessenberg matrix \n
 * \a converge : (0) problem converges; (1) problem fails to converge within
 * MAX_ITERS; (2) problem converging too slowly (try a better initial guess) \n
 *
 * @param size : [ Input ] Maximum linear dimension of input matrix \a A
 * @param iter1 : [ Input ] Current iteration number (restart loop)
 * @param iter2 : [ Output ] Current iteration number (internal loop)
 * @param tolerance : [ Input ] Convergence criteria
 * @param A : [ Input ] Coefficient matrix for the linear problem
 * @param x : [ Input ] Initial guess; [ Output ] Current solution vector
 * @param B : [ Input ] Independent coefficients; [ Output ] Solution vector
 */
int iterGMRES_el(unsigned int size,
                 unsigned int iter1,
                 unsigned int *iter2,
                 double tolerance,
                 double **A,
                 double *x,
                 double *B,
                 int *procSize,
                 int *procFirst,
                 int *procNodes)
{ 
  int i, j, k, iters = 0, converge = 1;
  double norm, htmp, h1, h2; 
  double error[2];
  double *res, *c, *s, *krlvTemp;
  double **h, **krylov;

    s = doubleVector(MAX_ITERS,1);
    c = doubleVector(MAX_ITERS,1);
    res = doubleVector(MAX_ITERS,1);
    krylov = doublePointer(MAX_ITERS,1);
    h = doubleMatrix(MAX_ITERS,MAX_ITERS,1);

    /* Determine the first Krylov subspace vector */
    krylov[0] = doubleVector(size,1);
    krlvTemp  = doubleVector(localSize,1);
    initRes_el(size,A,x,B,krlvTemp,procFirst);
    
    MPI_Allgatherv(krlvTemp, localSize, MPI_DOUBLE, 
                   krylov[0], procSize, procFirst, 
                   MPI_DOUBLE, MPI_COMM_WORLD);
                   
    norm = L2Norm(size,krylov[0]);
    for(i = 0; i < size; i++) krylov[0][i] = krylov[0][i]/norm;
    res[0]   = norm;
    error[0] = norm;

    /* If initial guess is accurate exit */
    if(error[0] < tolerance) return 0;

  /* GMRES iterations starts */
  for(i = 0; i < MAX_ITERS - 1; i++){ // BUG FIX BY ZHENBANG to avoid out of bound
    iters++;

    /* Determine Krylov[i+1] vector using Krylov[i] vector */
    krylov[i+1] = doubleVector(size,1);
    matVectProd_el(A,krylov[i],krlvTemp);
    
    MPI_Allgatherv(krlvTemp, localSize, MPI_DOUBLE, 
                   krylov[i+1], procSize, procFirst, 
                   MPI_DOUBLE, MPI_COMM_WORLD); 
                      
    norm = L2Norm(size,krylov[i+1]);

    /* orthogonalization of krylov[i+1] w.r.t. all previous krylov[i] */
    for(j = 0; j <= i; j++){
      h[j][i] = dotProd(size,krylov[j],krylov[i+1]);
      for(k = 0; k < size; k++)
          krylov[i+1][k] = krylov[i+1][k] - h[j][i]*krylov[j][k];
    }

    h[i+1][i] = L2Norm(size,krylov[i+1]); 
    if(h[i+1][i] != 0.0)
        for(j = 0; j < size; j++) krylov[i+1][j] = krylov[i+1][j]/h[i+1][i];

    /* Compute the Givens rotation that reduces the    */ 
    /* Hessenberg matrix to an upper triangular matrix */
    if(i > 0){
        for(j = 0; j <= (i-1); j++){
            h1 = c[j]*h[j][i] + s[j]*h[j+1][i];
            h2 = -s[j]*h[j][i] + c[j]*h[j+1][i];

            h[j][i] = h1;
            h[j+1][i] = h2;
        }
    }

    /* Perform the current Givens rotation */
    htmp = h[i+1][i]/h[i][i];
    if(norm != 0.0){
      c[i] = 1.0/sqrt(1.0 + htmp*htmp);
      s[i] = c[i]*htmp;

      h[i][i] = c[i]*h[i][i] + s[i]*h[i+1][i];
      h[i+1][i] = 0.0;

      res[i+1] = -s[i]*res[i];
      res[i] = c[i]*res[i];
    }

    /* Update the error and test for convergence */ 
    error[1] = fabs(res[i+1]);
    if( proc == master ){
        fprintf(fg,"%8d%25.4e\n", MAX_ITERS*(iter1-1)+iters, error[1]);
        fclose(fg);
        fg = fopen("gmres.log","a");
    }

    if(error[1] < tolerance){
      converge = 0;
      break;
    }
    else if(fabs((error[1] - error[0])/error[0]) < 1.0e-6*tolerance){
      if( proc == master )
          errorHandler("CONVERGENCE RATE TOO SLOW");
    }
    else error[0] = error[1]; 
  }

  /* Solve the triangle system of equations for the weights */
  for(i = iters-1; i >= 0; i--){
    for(j = i+1; j <= iters; j++) res[i] -= h[i][j]*res[j];
      res[i] = res[i]/h[i][i];
  }

  /* Return the solution in x */
  for(i = 0; i < size; i++)
    for(j = 0; j < iters; j++) x[i] += res[j]*krylov[j][i];

  *iter2 = iters;

  /* Free dynamically allocated memory */
  free(s);
  free(c);
  free(res);
  free(krlvTemp);
  freeDoubleMatrix(h,MAX_ITERS);
  freeDoublePointer(krylov,iters + 1);

  return converge;
}

/**
 * Computes the initial residual vector r for the GMRES solver. This "_el"
 * version has been modified to use a [nNodes]x[2nNodes] input matrix.
 *
 * @param size : [ Input ] Maximum linear dimension of input matrix \a A
 * @param A : [ Input ] Coefficient matrix for the linear problem
 * @param x : [ Input ] Current solution vector
 * @param B : [ Input ] Independent coefficients
 * @param r : [ Output ] Residual vector
 */
void initRes_el(unsigned int size,
                double **A,
                double *x,
                double *B,
                double *r,
                int *procFirst)
{
    unsigned int i;
    double norm;
    double *y;

    y    = doubleVector(localSize,0);
    norm = L2Norm(size,x);

    /* Given initial guess */
    if(norm != 0.0){
        matVectProd_el(A,x,y);
        for(i = 0; i < localSize; i++) r[i] = B[i + procFirst[proc]] - y[i];
    }
    /* Initial guess is zero */
    else for(i = 0; i < localSize; i++) r[i] = B[i + procFirst[proc]];

    free(y);
}

/**
 * Computes the inner product of two double precission vectors \a x and \a y.
 *
 * @param size : [ Input ] Length of the input vectors
 * @param x : [ Input ] Vector 1
 * @param y : [ Input ] Vector 2
 */
double dotProd(unsigned int size,
               double *x,
               double *y)
{
    unsigned int i;
    double product = 0.0;

    for(i = 0; i < size; i++) product += x[i]*y[i];

    return product;
}

/**
 * Computes the matrix-vector product y = A*x. This "_el" version has been
 * modified to use a [nNodes]x[2nNodes] matrix.
 *
 * @param A : [ Input ] Matrix os size [nNodes]x[2nNodes]
 * @param x : [ Input ] Vector of length 2nNodes
 * @param y : [ Output ] Vector of length 2nNodes
 */
void matVectProd_el(double **A,
                    double *x,
                    double *y)
{
  int col, i, j, row;

    for(i = nodeMin; i <= nodeMax; i++){

        col = i - nodeMin;
        y[2*col    ] = 0.0;
        y[2*col + 1] = 0.0;

        for(j = 0; j < nNodes; j++){
            row = 2*j;
            y[2*col    ] += A[col][row    ]*x[row    ]
                          + A[col][row + 1]*x[row + 1];
            y[2*col + 1] += A[col][row    ]*x[row + 1]
                          - A[col][row + 1]*x[row    ];
        }

    }
}

/**
 * Computes the L2 norm of vector \a x of size \b size.
 *
 * @param size : [ Input ] Vector length
 * @param x    : [ Input ] Vector of length \a size
 */
double L2Norm(unsigned int size,
              double *x)
{
    unsigned int i;
    double norm = 0.0;

    for(i = 0; i < size; i++) {
        norm += x[i]*x[i];
    }

    return sqrt(norm);
}
