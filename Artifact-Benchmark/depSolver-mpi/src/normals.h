/******************************************************************************
* File     : normals.h                                                        *
* Author   : Carlos Rosales Fernandez (carlos.rosales.fernandez(at)gmail.com) *
* Revision : 1.0 (2008-12-23)                                                 *
******************************************************************************/
/**
 * @brief Prototype declarations of functions used to calculate normal vectors
 *
 * @file
 * Prototype declarations of functions used to calculate normal vectors
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

int getLocalNormal_tria3(double X[3][3],
                         double *normal);

int getLocalNormal_tria6(double *L,
                         double X[6][3],
                         double *normal);

int getNormal_tria3(unsigned int nNodeID,
                    double **mNodes,
                    unsigned int **mElems,
                    double *normal);

int getNormal_tria6(unsigned int nNodeID,
                    double **mNodes, 
                    unsigned int **mElems,
                    double *normal);
