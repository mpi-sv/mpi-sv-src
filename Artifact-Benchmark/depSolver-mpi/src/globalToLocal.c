/******************************************************************************
* File     : globalToLocal.c                                                  *
* Author   : Carlos Rosales Fernandez (carlos.rosales.fernandez(at)gmail.com) *
* Revision : 1.0 (2008-12-23)                                                 *
******************************************************************************/
/**
 * @brief Global to local coordinate transformation functions
 *
 * @file
 * Definition of the three global to local transformation functions needed for
 * numerical integration within depSolver: X2L_line2, X2L_tria3, X2L_tria6.
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
#include "globalToLocal.h"


/**
 * Takes a vector \a N(L) with the shape functions at the local variables and a
 * matrix \a X with the global variables of the element and returns a vector
 * \a XL that is the transformed \a L in global space, and the Jacobian.
 *
 * Works for linear interpolation in line elements (2-noded lines).
 *
 * @param N  : [ Input ]  Shape functions for a point \a L in local coordinates
 * @param X  : [ Input ]  Global coordinates of the nodes defining the element
 * @param XL : [ Output ] Global coordinates corresponding to the given \a N
 */
double X2L_line2(double X[2][2],
                 double *XL,
                 double *N)
{
    double dx, dy;

    /* Global coordinates */
    XL[0] = N[0]*X[0][0] + N[1]*X[1][0];
    XL[1] = N[0]*X[0][1] + N[1]*X[1][1];

    /* Jacobian */
    dx = 0.5*(X[1][0] - X[0][0]);
    dy = 0.5*(X[1][1] - X[0][1]); 
    return ( dx*dx + dy*dy );
}


/**
 * Takes a vector \a N(L) with the shape functions at the local variables and a
 * matrix \a X with the global variables of the element and returns a vector
 * \a XL that is the transformed \a L in global space, and the Jacobian.
 *
 * Works for linear interpolation in triangular elements (3-noded triangles).
 *
 * @param N  : [ Input ]  Shape functions for a point \a L in local coordinates
 * @param X  : [ Input ]  Global coordinates of the nodes defining the element
 * @param XL : [ Output ] Global coordinates corresponding to the given \a N
 */
double X2L_tria3(double X[3][3],
                 double *XL,
                 double *L,
                 double *N)
{
    double dx1, dx2, dy1, dy2, dz1, dz2, j1, j2, j3;

    /* Global coordinates */
    XL[0] = N[0]*X[0][0] + N[1]*X[1][0] + N[2]*X[2][0];
    XL[1] = N[0]*X[0][1] + N[1]*X[1][1] + N[2]*X[2][1];
    XL[2] = N[0]*X[0][2] + N[1]*X[1][2] + N[2]*X[2][2];
    
    /* Derivatives of (x,y,z) in global space with respect to L1 and L2 */
    dx1 = X[0][0] - X[2][0];
    dx2 = X[1][0] - X[2][0];
    dy1 = X[0][1] - X[2][1];
    dy2 = X[1][1] - X[2][1];
    dz1 = X[0][2] - X[2][2];
    dz2 = X[1][2] - X[2][2];

    /* Jacobian */
    j1 = dy1*dz2 - dy2*dz1;
    j2 = dx2*dz1 - dx1*dz2;
    j3 = dx1*dy2 - dx2*dy1;

    return (j1*j1 + j2*j2 + j3*j3);
}


/**
 * Takes a vector \a N(L) with the shape functions at the local variables and a
 * matrix \a X with the global variables of the element and returns a vector
 * \a XL that is the transformed \a L in global space, and the Jacobian.
 *
 * Works for quadratic interpolation in triangular elements (6-noded triangles).
 *
 * @param N  : [ Input ]  Shape functions for a point \a L in local coordinates
 * @param X  : [ Input ]  Global coordinates of the nodes defining the element
 * @param XL : [ Output ] Global coordinates corresponding to the given \a N
 */
double X2L_tria6(double X[6][3],
                 double *XL,
                 double *L,
                 double *N)
{
    double ax, ay, az, dx1, dx2, dy1, dy2, dz1, dz2, j1, j2, j3, L1, L2;

    /* Global coordinates */
    XL[0] = N[0]*X[0][0] + N[1]*X[1][0] + N[2]*X[2][0] 
          + N[3]*X[3][0] + N[4]*X[4][0] + N[5]*X[5][0];
          
    XL[1] = N[0]*X[0][1] + N[1]*X[1][1] + N[2]*X[2][1] 
          + N[3]*X[3][1] + N[4]*X[4][1] + N[5]*X[5][1];
          
    XL[2] = N[0]*X[0][2] + N[1]*X[1][2] + N[2]*X[2][2] 
          + N[3]*X[3][2] + N[4]*X[4][2] + N[5]*X[5][2];

    /* Auxiliar quantities */
    L1 = L[0];
    L2 = L[1];
    ax = X[1][0] - X[3][0] + X[4][0] - X[5][0];
    ay = X[1][1] - X[3][1] + X[4][1] - X[5][1];
    az = X[1][2] - X[3][2] + X[4][2] - X[5][2];

    /* Derivatives of (x,y,z) in global space with respect to L1 and L2 */
    dx1 = 4.0*(L1*(X[0][0] + X[4][0] - 2.0*X[5][0]) + L2*ax + X[5][0]) - X[0][0] - 3.0*X[4][0];   
    dx2 = 4.0*(L1*ax + L2*(X[2][0] + X[4][0] - 2.0*X[3][0]) + X[3][0]) - X[2][0] - 3.0*X[4][0];
    dy1 = 4.0*(L1*(X[0][1] + X[4][1] - 2.0*X[5][1]) + L2*ay + X[5][1]) - X[0][1] - 3.0*X[4][1];
    dy2 = 4.0*(L1*ay + L2*(X[2][1] + X[4][1] - 2.0*X[3][1]) + X[3][1]) - X[2][1] - 3.0*X[4][1];
    dz1 = 4.0*(L1*(X[0][2] + X[4][2] - 2.0*X[5][2]) + L2*az + X[5][2]) - X[0][2] - 3.0*X[4][2];
    dz2 = 4.0*(L1*az + L2*(X[2][2] + X[4][2] - 2.0*X[3][2]) + X[3][2]) - X[2][2] - 3.0*X[4][2];

    /* Jacobian */
    j1 = dy1*dz2 - dy2*dz1;
    j2 = dx2*dz1 - dx1*dz2;
    j3 = dx1*dy2 - dx2*dy1;

    return (j1*j1 + j2*j2 + j3*j3);
}

