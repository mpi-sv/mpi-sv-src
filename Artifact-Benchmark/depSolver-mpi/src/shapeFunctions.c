/******************************************************************************
* File     : shapeFunctions.c                                                 *
* Author   : Carlos Rosales Fernandez (carlos.rosales.fernandez(at)gmail.com) *
* Revision : 1.0 (2008-12-23)                                                 *
******************************************************************************/
/**
 * @brief Shape functions
 *
 * @file
 * This file contains the definition of the three shape (or interpolating)
 * functions required by depSolver: shape_line2, shape_tria3, shape_tria6.
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
#include "shapeFunctions.h"

/**
 * Returns a vector N with the shape functions for linear interpolation in a
 * line element (2-noded line).
 */
int shape_line2(double L,
                double *N)
{
    /* Shape function definition */
    N[0] = 0.5*( 1.0 - L );
    N[1] = 0.5*( 1.0 + L );

    return 0;
}

/**
 * Returns a vector N with the shape functions for linear interpolation in a
 * isoparametric triangular element (3-noded triangle).
 */
int shape_tria3(double *L,
                double *N)
{
    /* Shape function definition for node order 1-2-3 */
    N[0] = L[0];
    N[1] = L[1];
    N[2] = 1.0 - L[0] - L[1];

    return 0;
}

/**
 * Returns a vector N with the shape functions for quadratic interpolation in a
 * isoparametric triangular element (6-noded triangle).
 */
int shape_tria6(double *L,
                double *N)
{
    double L1, L2, L3;

    /* Auxiliar quantities */
    L1 = L[0];
    L2 = L[1];
    L3 = 1.0 - L1 - L2;

    /* Shape function definition for node order 1-2-3-4-5-6 */
    N[0] = L1*(2.0*L1 - 1.0);
    N[1] = 4.0*L1*L2;
    N[2] = L2*(2.0*L2 - 1.0);
    N[3] = 4.0*L2*L3;
    N[4] = L3*(2.0*L3 - 1.0);
    N[5] = 4.0*L1*L3;

    return 0;
}

