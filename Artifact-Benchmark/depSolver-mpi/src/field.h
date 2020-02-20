/******************************************************************************
* File      : field.h                                                         *
* Author   : Carlos Rosales Fernandez (carlos.rosales.fernandez(at)gmail.com) *
* Revision : 1.0 (2008-12-23)                                                 *
******************************************************************************/
/**
 * @brief Electric field function prototypes
 *
 * @file
 * Prototypes of the functions necessary to calculate the electric field
 * in depSolver: field_tria3 and field_tria6.
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

int field_tria3(double *Xin,
                double **mNodes,
                unsigned int **mElems,
                double *vB,
                double *mE);

int field_tria6(double *Xin,
                double **mNodes,
                unsigned int **mElems,
                double *vB,
                double *mE);

