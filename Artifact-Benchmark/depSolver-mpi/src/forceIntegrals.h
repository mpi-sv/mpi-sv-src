/******************************************************************************
* File     : forceIntegrals.h                                                 *
* Author   : Carlos Rosales Fernandez (carlos.rosales.fernandez(at)gmail.com) *
* Revision : 1.0 (2008-12-23)                                                 *
******************************************************************************/
/**
 * @brief Prototypes of the DEP force integral functions
 *
 * @file
 * Prototype declarations of the four functions necessary for the integrals in
 * the force calculations of depSolver: intDE_tria3, intDE_tria6, intF_tria3,
 * intF_tria6.
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

int intDE_tria3(unsigned int nOrder,
                double X[3][3],
                double *Xeval,
                double Efield[][5][5][5]);

int intDE_tria6(unsigned int nOrder,
                double X[6][3],
                double *Xeval,
                double Efield[][5][5][5]);

int intF_tria3(double X[3][3],
               double E[3][3],
               double *Int);

int intF_tria6(double X[6][3],
               double E[6][3],
               double *Int);
