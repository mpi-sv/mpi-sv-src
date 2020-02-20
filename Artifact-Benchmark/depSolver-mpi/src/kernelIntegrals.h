/******************************************************************************
* File     : kernelIntegrals.h                                                *
* Author   : Carlos Rosales Fernandez (carlos.rosales.fernandez(at)gmail.com) *
* Revision : 1.0 (2008-12-23)                                                 *
******************************************************************************/
/**
 * @brief Prototype declarations for numerical integration of Green's function
 *
 * @file
 * Prototype declarations for numerical integration of Green's function
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

int intG_tria3(double X[3][3],
               double *Xeval,
               double *Int);

int intG_tria6(double X[6][3],
               double *Xeval,
               double *Int);

int intH_tria3(const unsigned int FLAG,
               double X[3][3],
               double *Xeval,
               double *Int,
               double *normal);

int intH_tria6(const unsigned int FLAG,
               double X[6][3],
               double *Xeval,
               double *Int,
               double *normal);

int intSingularG_tria3(unsigned int SinNode,
                       double X[3][3],
                       double *Xeval,
                       double *Int);

int intSingularG_tria6(unsigned int SinNode,
                       double X[6][3],
                       double *Xeval,
                       double *Int);

int intSingularH_tria3(const unsigned int FLAG,
                       unsigned int SinNode,
                       double X[3][3],
                       double *Xeval,
                       double *Int,
                       double *normal);

int intSingularH_tria6(const unsigned int FLAG,
                       unsigned int SinNode,
                       double X[6][3],
                       double *Xeval,
                       double *Int,
                       double *normal);
