/******************************************************************************
* File     : gmres-mpi.h                                                      *
* Author   : Carlos Rosales Fernandez (carlos.rosales.fernandez(at)gmail.com) *
* Revision : 1.0 (2008-12-23)                                                 *
******************************************************************************/
/**
 * @brief Prototype declarations for GMRES functions
 *
 * @file
 * Prototype declarations for GMRES functions
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

void solverGMRES_el(unsigned int preCond,
                    unsigned int nInit,
                    double **A,
                    double *B,
                    int *procSize,
                    int *procFirst,
                    int *procNodes);

int iterGMRES_el(unsigned int size,
                 unsigned int iter1,
                 unsigned int *iter2,
                 double tolerance,
                 double **A,
                 double *x,
                 double *B,
                 int *procSize,
                 int *procFirst,
                 int *procNodes);

void initRes_el(unsigned int size,
                double **A,
                double *x,
                double *B,
                double *r,
                int *procFirst);

double dotProd(unsigned int size,
               double *x,
               double *y);

void matVectProd_el(double **A,
                    double *x,
                    double *y);

double L2Norm(unsigned int size,
              double *x);
