/******************************************************************************
* File     : depolarization.c                                                 *
* Author   : Carlos Rosales Fernandez (carlos.rosales.fernandez(at)gmail.com) *
* Revision : 1.0 (2008-12-23)                                                 *
******************************************************************************/
/**
 * @brief Calculate depolarizaton factor for an arbitrary ellipsoid
 *
 * @file
 * Takes the semiaxis lengths of an ellipsoid in the input array \a axis, and
 * calculates the depolarization factors numerically using a high quality
 * Gauss-Legendre quadrature, returning them in array \a Ld.
 *
 * @param axis : [ Input ] Semi-axis lengths of an arbitrary ellipsoid
 * @param Ld   : [ Output ] Polarization factor in each of the three spatial directions
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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "depolarization.h"
#include "errorHandler.h"
#include "globalToLocal.h"
#include "shapeFunctions.h"

extern const double GaussH[64][2];

int depolarization(double *axis,
                   double *Ld)
{
    unsigned int  i;
    double  A, a2, b2, c2, Jsq, L, R, W, x;
    double  N[2], XL[2];
    double  X[2][2];

    /* Initialize */
    Ld[0] = Ld[1] = Ld[2] = 0.0;
    X[0][0] = X[0][1] = X[1][1] = 0.0;
    A = 0.5*axis[0]*axis[1]*axis[2];
    a2 = axis[0]*axis[0];
    b2 = axis[1]*axis[1];
    c2 = axis[2]*axis[2];

    /* Integrate for Lx using Gaussian quadrature */
    X[1][0] = 100*a2;
    for(i = 0; i < 64; i ++){
        L = GaussH[i][0];
        W = GaussH[i][1];
    
        shape_line2(L,N);
        Jsq = X2L_line2(X,XL,N);
        x = XL[0];
        R = W*sqrt( Jsq/( (x+a2)*(x+b2)*(x+c2) ) );
    
        Ld[0] += R/(x+a2);
    }

    /* Integrate for Ly using Gaussian quadrature */
    X[1][0] = 100*b2;
    for(i = 0; i < 64; i ++){
        L = GaussH[i][0];
        W = GaussH[i][1];
    
        shape_line2(L,N);
        Jsq = X2L_line2(X,XL,N);
        x = XL[0];
        R = W*sqrt(Jsq/( (x+a2)*(x+b2)*(x+c2) ) );
    
        Ld[1] += R/(x+b2);
    }

    /* Integrate for Lz using Gaussian quadrature */
    X[1][0] = 100*c2;
    for(i = 0; i < 64; i ++){
        L = GaussH[i][0];
        W = GaussH[i][1];
    
        shape_line2(L,N);
        Jsq = X2L_line2(X,XL,N);
        x = XL[0];
        R = W*sqrt( Jsq/( (x+a2)*(x+b2)*(x+c2) ) );
    
        Ld[2] += R/(x+c2);
    }


    Ld[0] = A*Ld[0];
    Ld[1] = A*Ld[1];
    Ld[2] = A*Ld[2];

    /* Safety checks */
    for(i = 0; i < 3; i++){
        if(Ld[i] < 0.0) errorHandler("Error: depolarization() - factors must be positive!");
    }
    if(fabs(1.0 - Ld[0] - Ld[1] - Ld[2]) > 0.05)
        printf("Warning: depolarization() - error is larger than 0.05");

    return 0;
}
