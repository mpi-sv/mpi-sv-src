/******************************************************************************
* File     : assembly-mpi.h                                                   *
* Author   : Carlos Rosales Fernandez (carlos.rosales.fernandez(at)gmail.com) *
* Revision : 1.0 (2008-12-23)                                                 *
******************************************************************************/
/**
 * @brief Declaration of matrix assembly functions.
 *
 * @file
 * Matrix assembly function prototypes.
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

int electricFormA_tria3(double **mNodes,
                        unsigned int **mElems,
                        unsigned int *vBCType,
                        unsigned int **vInterfaces,
                        double **vMatParam,
                        double *vProbParam,
                        double **mA,
                        double **mBC,
                        double *vB);

int electricFormA_tria6(double **mNodes,
                        unsigned int **mElems,
                        unsigned int *vBCType,
                        unsigned int **vInterfaces,
                        double **vMatParam,
                        double *vProbParam,
                        double **mA,
                        double **mBC,
                        double *vB);
