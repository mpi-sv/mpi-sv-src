/******************************************************************************
* File     : kernelIntegrals.c                                                *
* Author   : Carlos Rosales Fernandez (carlos.rosales.fernandez(at)gmail.com) *
* Revision : 1.0 (2008-12-23)                                                 *
******************************************************************************/
/**
 * @brief Numerical integration of Green's function kernels
 *
 * @file
 * Definition of the eight functions that perform the numerical integration of
 * the Green's function kernel: intG_tria3, intG_tria6, inH_tria3,
 * intH_tria6, intSingularG_tria3, intSingularG_tria6, intSingularH_tria3,
 * intSingularH_tria6.
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
#include "constants.h"
#include "globalToLocal.h"
#include "kernelIntegrals.h"
#include "shapeFunctions.h"

/* Matrices needed for regular integration */
extern const double TGauss[TNGAUSS][3];

/* Matrices needed for weakly singular integration */
extern const double Gauss[NGAUSS][2], GaussJacobi[NGAUSS][2];

/* Matrices needed for strongly singular integration */
extern const double TSGauss[TSNGAUSS][3];

/**
 * Computes the integral of the Green's function G for one element. The output
 * is the vector \a Int, which contains the integrals of the Green's function
 * times each of the element's shape functions \a N over the whole element. This
 * represents the partial contribution of each of the nodes in the element.
 *
 * Works for linear interpolation in triangular elements (3-noded triangles).
 *
 * @param X  : [ Input ]  Global coordinates of the nodes defining the element
 * @param Xeval : [ Input ] Coordinates of evaluation point
 * @param Int   : [ Output ] Integral of the Green's function times the shape functions
 */
int intG_tria3(double X[3][3],
               double *Xeval,
               double *Int)
{
    unsigned int i;
    double A, dx, dy, dz, Jsq, W;
    double L[2], N[3], XL[3];

    /* Initialize */
    Int[0] = Int[1] = Int[2] = 0.0; 

    /* Integrate using Gauss-Legendre quadrature */
    for(i = 0; i < TNGAUSS; i++){
        L[0] = TGauss[i][0];
        L[1] = TGauss[i][1];
        W = TGauss[i][2];

        /* Shape functions N and Jacobian J for these (L1,L2) values */
        shape_tria3(L,N);       
        Jsq = X2L_tria3(X,XL,L,N);

        /* Auxiliar quantities */
        dx = Xeval[0] - XL[0];
        dy = Xeval[1] - XL[1];
        dz = Xeval[2] - XL[2];
        A = W*sqrt( Jsq/( dx*dx + dy*dy + dz*dz ) );

        /* Add contribution for each node in this element */
        Int[0] += A*N[0];
        Int[1] += A*N[1];
        Int[2] += A*N[2];
    }

    return 0;
}

/**
 * Computes the integral of the Green's function G for one element. The output
 * is the vector \a Int, which contains the integrals of the Green's function
 * times each of the element's shape functions \a N over the whole element. This
 * represents the partial contribution of each of the nodes in the element.
 *
 * Works for quadratic interpolation in triangular elements (6-noded triangles).
 *
 * @param X  : [ Input ]  Global coordinates of the nodes defining the element
 * @param Xeval : [ Input ] Coordinates of evaluation point
 * @param Int   : [ Output ] Integral of the Green's function times the shape functions
 */
int intG_tria6(double X[6][3],
               double *Xeval,
               double *Int)
{
    const unsigned int NODES_IN_ELEM = 6;
    unsigned int i, j;
    double A, dx, dy, dz, Jsq, W;
    double L[2], N[6], XL[3];

    /* Initialize */
    for(i = 0; i < NODES_IN_ELEM; i++) Int[i] = 0.0; 

    /* Integrate using Gauss-Legendre quadrature */
    for(i = 0; i < TNGAUSS; i++){
        L[0] = TGauss[i][0];
        L[1] = TGauss[i][1];
        W = TGauss[i][2];

        /* Shape functions N and Jacobian J for these (L1,L2) values */
        shape_tria6(L,N);       
        Jsq = X2L_tria6(X,XL,L,N);

        /* Auxiliar quantities */
        dx = Xeval[0] - XL[0];
        dy = Xeval[1] - XL[1];
        dz = Xeval[2] - XL[2];
        A = W*sqrt( Jsq/( dx*dx + dy*dy + dz*dz ) );
    
        /* Add contribution for each node in this element */
        for(j = 0; j < NODES_IN_ELEM; j++) Int[j] += A*N[j];    
    }

    return 0;
}


/**
 * Computes the integral of the Green's function derivative H for one
 * element. The output is the vector \a Int, which contains the integrals of the
 * Green's function derivative times each of the element's shape
 * functions \a N over the whole element. This represents the partial
 * contribution of each of the nodes in the element.
 *
 * Works for linear interpolation in triangular elements (3-noded triangles).
 *
 * @param FLAG  : [ Input ] Use normal derivative (0) or gradient (!=0)
 * @param X     : [ Input ] Global coordinates of the nodes defining the element
 * @param Xeval : [ Input ] Coordinates of evaluation point
 * @param Int   : [ Output ] Integral of the Green's function times the shape functions
 * @param normal : [ Input ] Vecotr normal to this element's surface 
 */
int intH_tria3(const unsigned int FLAG,
               double X[3][3],
               double *Xeval,
               double *Int,
               double *normal)
{
    const unsigned int NODES_IN_ELEM = 3;
    unsigned int i, j;
    double A, B, C, dx, dy, dz, Jsq, nxdx, rsq, W;
    double L[2], N[3], XL[3];

    /* Initialize */
    if(!FLAG) Int[0] = Int[1] = Int[2] = 0.0;
    else for(i = 0; i < 9; i++) Int[i] = 0.0;

    /* Integrate using Gauss-Legendre quadrature */
    for(i = 0; i < TNGAUSS; i++){
        L[0] = TGauss[i][0];
        L[1] = TGauss[i][1];
        W = TGauss[i][2];

        /* Shape functions N and Jacobian J for these (L1,L2) values */
        shape_tria3(L,N);       
        Jsq = X2L_tria3(X,XL,L,N);

        /* Auxiliar quantities */
        dx = Xeval[0] - XL[0];
        dy = Xeval[1] - XL[1];
        dz = Xeval[2] - XL[2];
        rsq = dx*dx + dy*dy + dz*dz;
        A = W*sqrt( Jsq/(rsq*rsq*rsq) );

        /* Add contribution for each node in this element */
        if(!FLAG){
            nxdx = dx*normal[0] + dy*normal[1] + dz*normal[2];
            A = A*nxdx;
            Int[0] -= A*N[0];
            Int[1] -= A*N[1];
            Int[2] -= A*N[2];
        }
        else{
            C = A*dz;
            B = A*dy;
            A = A*dx;
            for(j = 0; j < NODES_IN_ELEM; j++){
                Int[j*3  ] += A*N[j];
                Int[j*3+1] += B*N[j];
                Int[j*3+2] += C*N[j];
            }
        }
    }

    return 0;
}


/**
 * Computes the integral of the Green's function derivative H for one
 * element. The output is the vector \a Int, which contains the integrals of the
 * Green's function derivative times each of the element's shape
 * functions \a N over the whole element. This represents the partial
 * contribution of each of the nodes in the element.
 *
 * Works for quadratic interpolation in triangular elements (6-noded triangles).
 *
 * @param FLAG  : [ Input ] Use normal derivative (0) or gradient (!=0)
 * @param X     : [ Input ] Global coordinates of the nodes defining the element
 * @param Xeval : [ Input ] Coordinates of evaluation point
 * @param Int   : [ Output ] Integral of the Green's function times the shape functions
 * @param normal : [ Input ] Vecotr normal to this element's surface 
 */
int intH_tria6(const unsigned int FLAG,
               double X[6][3],
               double *Xeval,
               double *Int,
               double *normal)
{
    const unsigned int NODES_IN_ELEM = 6;
    unsigned int i, j;
    double A, B, C, dx, dy, dz, Jsq, nxdx, rsq, W;
    double L[2], N[6], XL[3];

    /* Initialize */
    if(!FLAG) for(i = 0; i < NODES_IN_ELEM; i++) Int[i] = 0.0;
    else for(i = 0; i < 18; i++) Int[i] = 0.0;

    /* Integrate using Gauss-Legendre quadrature */
    for(i = 0; i < TNGAUSS; i++){
        L[0] = TGauss[i][0];
        L[1] = TGauss[i][1];
        W    = TGauss[i][2];

        /* Shape functions N and Jacobian J for these (L1,L2) values */
        shape_tria6(L,N);       
        Jsq = X2L_tria6(X,XL,L,N);

        /* Auxiliar quantities */
        dx = Xeval[0] - XL[0];
        dy = Xeval[1] - XL[1];
        dz = Xeval[2] - XL[2];
        rsq = dx*dx + dy*dy + dz*dz;
        A = W*sqrt( Jsq/(rsq*rsq*rsq) );

        /* Add contribution for each node in this element */
        if(!FLAG){
            nxdx = dx*normal[0] + dy*normal[1] + dz*normal[2];
            A = A*nxdx;
            for(j = 0; j < NODES_IN_ELEM; j++) Int[j] -= A*N[j];
        }
        else{
            C = A*dz;
            B = A*dy;
            A = A*dx;
            for(j = 0; j < NODES_IN_ELEM; j++){
                Int[j*3  ] += A*N[j];
                Int[j*3+1] += B*N[j];
                Int[j*3+2] += C*N[j];
            }          
        }
    }

    return 0;
}


/**
 * Computes the integral of the Green's function G when a collocation point is
 * one of the nodes of the integration element and returns the values for all
 * shape functions in \a Int. Uses a regularization transformation that changes
 * the triangle into a degenerated square where the integration is done using
 * Gauss-Jacobi quadrature.
 *
 * Works for linear interpolation in triangular elements (3-noded triangles).
 *
 * @param SinNode : [ Input ] Singular Node number (relative to the element)
 * @param X     : [ Input ]  Global coordinates of the nodes defining the element
 * @param Xeval : [ Input ] Coordinates of evaluation point
 * @param Int   : [ Output ] Integral of the Green's function times the shape functions
 */
int intSingularG_tria3(unsigned int SinNode,
                       double X[3][3],
                       double *Xeval,
                       double *Int)
{
    unsigned int i, j;
    double A, dx, dy, dz, Jsq, U1, U2, W1, W2;
    double L[2], N[3], XL[3];
 
    /* Initialize */
    Int[0] = Int[1] = Int[2] = 0.0;

    /* Integrate using Gauss-Legendre and Gauss-Jacobi quadratures */
    for(i = 0; i < NGAUSS; i++){
        U1 = Gauss[i][0];
        W1 = Gauss[i][1];
    
        for(j = 0; j < NGAUSS; j++){
            U2 = GaussJacobi[j][0];
            W2 = GaussJacobi[j][1];

            /* Linear transformation to get the singularity at node 1 */
            if(SinNode == 1){
                L[0] = 0.5*(1.0 - U2);
                L[1] = 0.25*(1.0 + U1)*(1.0 + U2);
            }
            else if(SinNode == 2){
                L[0] = 0.25*(1.0 - U1)*(1.0 + U2);
                L[1] = 0.5*(1.0 - U2);
            }
            else{
                L[0] = 0.25*(1.0 + U1)*(1.0 + U2);
                L[1] = 0.25*(1.0 - U1)*(1.0 + U2);
            }

            /* Shape functions N and Jacobian J for these (L1,L2) values */
            shape_tria3(L,N);
            Jsq = X2L_tria3(X,XL,L,N);
            
            /* Contribution from point (i,j) */
            dx = Xeval[0] - XL[0];
            dy = Xeval[1] - XL[1];
            dz = Xeval[2] - XL[2];
            A = W1*W2*0.125*sqrt( Jsq/(dx*dx + dy*dy + dz*dz) );              
            Int[0] += A*N[0];
            Int[1] += A*N[1];
            Int[2] += A*N[2];
        }
    }

    return 0;
}


/**
 * Computes the integral of the Green's function G when a collocation point is
 * one of the nodes of the integration element and returns the values for all
 * shape functions in \a Int. Uses a regularization transformation that changes
 * the triangle into a degenerated square where the integration is done using
 * Gauss-Jacobi quadrature.
 *
 * Works for quadratic interpolation in triangular elements (6-noded triangles).
 *
 * @param SinNode : [ Input ] Singular Node number (relative to the element)
 * @param X     : [ Input ]  Global coordinates of the nodes defining the element
 * @param Xeval : [ Input ] Coordinates of evaluation point
 * @param Int   : [ Output ] Integral of the Green's function times the shape functions
 */
int intSingularG_tria6(unsigned int SinNode,
                       double X[6][3],
                       double *Xeval,
                       double *Int)
{
    const unsigned int NODES_IN_ELEM = 6;
    unsigned int i, j, k, T;
    double A, B, dx, dy, dz, Jsq, U1, U2, W1, W2;
    double L1[2], L2[2], N1[6], N2[6], XL[3];
 
    /* Initialize */
    for(i = 0; i < NODES_IN_ELEM; i++) Int[i] = 0.0;

    /* Choose number of subelements */
    if(SinNode%2 == 1){
        T = 1;
        B = 0.125;
    }
    else{
        T = 2;
        B = 0.0625;
    }

    /* Integrate using Gauss-Legendre and Gauss-Jacobi quadratures */
    for(i = 0; i < NGAUSS; i++){
        U1 = Gauss[i][0];
        W1 = Gauss[i][1];
    
        for(j = 0; j < NGAUSS; j++){
            U2 = GaussJacobi[j][0];
            W2 = GaussJacobi[j][1];

            /* Linear transformation to get the singularity at node 1 */
            if(SinNode == 1){
                L1[0] = 0.5*(1.0 - U2);
                L1[1] = 0.25*(1.0 + U1)*(1.0 + U2);
            }
            else if(SinNode == 2){
                L1[0] = 0.25*(1.0 - U2);
                L1[1] = 0.5 + 0.25*U1*(1.0 + U2);
                L2[0] = 0.5 - 0.25*U1*(1.0 + U2);
                L2[1] = 0.25*(1.0 - U2);
            }
            else if(SinNode == 3){
                L1[0] = 0.25*(1.0 - U1)*(1.0 + U2);
                L1[1] = 0.5*(1.0 - U2);
            }
            else if(SinNode == 4){
                L1[0] = 0.25*(1.0 + U1)*(1.0 + U2);
                L1[1] = 0.5 - 0.25*U1*(1.0 + U2);
                L2[0] = 0.25*(1.0 - U1)*(1.0 + U2);
                L2[1] = 0.25*(1.0 - U2);
            }
            else if(SinNode == 5){
                L1[0] = 0.25*(1.0 + U1)*(1.0 + U2);
                L1[1] = 0.25*(1.0 - U1)*(1.0 + U2);
            }
            else{
                L1[0] = 0.5 + 0.25*U1*(1.0 + U2);
                L1[1] = 0.25*(1.0 - U1)*(1.0 + U2);
                L2[0] = 0.25*(1 - U2);
                L2[1] = 0.25*(1.0 + U1)*(1.0 + U2);
            }

            /* Contribution from triangle 1 */
            shape_tria6(L1,N1);
            Jsq = X2L_tria6(X,XL,L1,N1);
            dx = Xeval[0] - XL[0];
            dy = Xeval[1] - XL[1];
            dz = Xeval[2] - XL[2];
            A = W1*W2*B*sqrt( Jsq/(dx*dx + dy*dy + dz*dz) );
            for(k = 0; k < NODES_IN_ELEM; k++) Int[k] += A*N1[k];

            /* Contribution from triangle 2 (only if necessary) */
            if(T == 2){
                shape_tria6(L2,N2);
                Jsq = X2L_tria6(X,XL,L2,N2);
                dx = Xeval[0] - XL[0];
                dy = Xeval[1] - XL[1];
                dz = Xeval[2] - XL[2];
                A = W1*W2*B*sqrt( Jsq/(dx*dx + dy*dy + dz*dz) );  
                for(k = 0; k < NODES_IN_ELEM; k++) Int[k] += A*N2[k];
            }
        }
    }

    return 0;
}


/**
 * Computes the integral of the Green's function derivative H for one
 * element when the collocation point is one of the nodes of the integration
 * element, and returns the values for all shape functions in vector \a Int.
 * Uses element subdivision and standard Gauss-Legendre quadrature integration.
 *
 * Works for linear interpolation in triangular elements (3-noded triangles).
 *
 * @param SinNode : [ Input ] Singular Node number (relative to the element)
 * @param FLAG  : [ Input ] Use normal derivative (0) or gradient (!=0)
 * @param X     : [ Input ] Global coordinates of the nodes defining the element
 * @param Xeval : [ Input ] Coordinates of evaluation point
 * @param Int   : [ Output ] Integral of the Green's function times the shape functions
 * @param normal : [ Input ] Vecotr normal to this element's surface 
 */
int intSingularH_tria3(const unsigned int FLAG,
                       unsigned int SinNode, 
                       double X[3][3],
                       double *Xeval,
                       double *Int,
                       double *normal)
{
    const unsigned int NODES_IN_ELEM = 3;
    unsigned int i, j, k, m;
    double A, B, C, dx, dy, dz, J, JLsq, nxdx, rsq, W, M1, M2, M3;
    double L[2], N[3], XL[3];
    double xs[3][3][2];
 
    /* Initialize */
    if(!FLAG) Int[0] = Int[1] = Int[2] = 0.0;
    else for(i = 0; i < 9; i++) Int[i] = 0.0;
    for(i = 0; i < 3; i++)
        for(j = 0; j < 3; j++)
            for(k = 0; k <2; k++) xs[i][j][k] = 0.0;

    /* Reorder so that singularity is at node 3 */
    if(SinNode == 1){
        for(i = 0; i < 3; i++) shift(X[2][i],X[0][i],X[1][i]);
    }
    else if(SinNode == 2){
        for(i = 0; i < 3; i++) shift(X[2][i],X[1][i],X[0][i]);
    }

    /* Setup 3 initial triangles */
    xs[0][0][0] = 1.0;  xs[0][0][1] = 0.0;
    xs[0][1][0] = 0.5;  xs[0][1][1] = 0.5;
    xs[0][2][0] = 0.5;  xs[0][2][1] = 0.0;

    xs[1][0][0] = 0.5;  xs[1][0][1] = 0.0;
    xs[1][1][0] = 0.5;  xs[1][1][1] = 0.5;
    xs[1][2][0] = 0.0;  xs[1][2][1] = 0.5;
    
    xs[2][0][0] = 0.5;  xs[2][0][1] = 0.5;
    xs[2][1][0] = 0.0;  xs[2][1][1] = 1.0;
    xs[2][2][0] = 0.0;  xs[2][2][1] = 0.5;
    
    /* Start subdivision loop */
    J = 1.0;
    for(m = 0; m < NSUBDIVISIONS; m++){
        J = J*0.25;
    
        /* Integrate over the 3 triangles furthest away from the singularity */
        for(i = 0; i < 3; i++){
            
            /* Integrate over each triangle using Gauss-Legendre quadrature */
            for(j = 0; j < TSNGAUSS; j++){
                M1 = TSGauss[j][0];
                M2 = TSGauss[j][1];
                M3 = 1.0 - M1 - M2;
                W = TSGauss[j][2];
                
                /**************************************************** 
                * Get L and N as a function of (M1,M2). This is the *
                * transformation between the unit triangle (M1,M2)  *
                * and the smaller subtriangle (L1,L2)               *
                ****************************************************/
                L[0] = xs[i][0][0]*M1 + xs[i][1][0]*M2 + xs[i][2][0]*M3;
                L[1] = xs[i][0][1]*M1 + xs[i][1][1]*M2 + xs[i][2][1]*M3;
                shape_tria3(L,N);
                JLsq = X2L_tria3(X,XL,L,N);

                /* Auxiliar quantities */
                dx = Xeval[0] - XL[0];
                dy = Xeval[1] - XL[1];
                dz = Xeval[2] - XL[2];
                rsq = dx*dx + dy*dy + dz*dz;
                A = J*W*sqrt( JLsq/(rsq*rsq*rsq) );

                if(!FLAG){
                    nxdx = dx*normal[0]+dy*normal[1]+dz*normal[2];
                    A = A*nxdx;
                    Int[0] -= A*N[0];
                    Int[1] -= A*N[1];
                    Int[2] -= A*N[2];
                }
                else{
                    C = A*dz;
                    B = A*dy;
                    A = A*dx;
                    for(k = 0; k < NODES_IN_ELEM; k++){
                        Int[k*3  ] += A*N[k];
                        Int[k*3+1] += B*N[k];
                        Int[k*3+2] += C*N[k];
                    }
                }
            }
        }
    
        /* Set triangles for new subdivision */
        for(i = 0; i < 3; i++) {
            for(j = 0; j < 3; j++){
                xs[i][j][0] = xs[i][j][0]*0.5;
                xs[i][j][1] = xs[i][j][1]*0.5;
            }
        }
    }

    /* Unscramble node contributions to return them in their right places */    
    if(FLAG == 0){
        if(SinNode == 1){
            shift(Int[1],Int[0],Int[2]);
        }
        else if(SinNode == 2){
            shift(Int[0],Int[1],Int[2]);
        }
    }
    else{
        if(SinNode == 1){
            for(i = 0; i < 3; i++) shift(Int[3+i],Int[i],Int[6+i]);
        }
        else if(SinNode == 2){
            for(i = 0; i < 3; i++) shift(Int[i],Int[3+i],Int[6+i]);
        }
    }

    return 0;
}


/**
 * Computes the integral of the Green's function derivative H for one
 * element when the collocation point is one of the nodes of the integration
 * element, and returns the values for all shape functions in vector \a Int.
 * Uses element subdivision and standard Gauss-Legendre quadrature integration.
 *
 * Works for quadratic interpolation in triangular elements (6-noded triangles).
 *
 * @param SinNode : [ Input ] Singular Node number (relative to the element)
 * @param FLAG  : [ Input ] Use normal derivative (0) or gradient (!=0)
 * @param X     : [ Input ] Global coordinates of the nodes defining the element
 * @param Xeval : [ Input ] Coordinates of evaluation point
 * @param Int   : [ Output ] Integral of the Green's function times the shape functions
 * @param normal : [ Input ] Vecotr normal to this element's surface 
 */
int intSingularH_tria6(const unsigned int FLAG,
                       unsigned int SinNode,
                       double X[6][3],
                       double *Xeval,
                       double *Int,
                       double *normal)
{
    const unsigned int NODES_IN_ELEM = 6;
    unsigned int i, j, k, m;
    double A, A1, A2, B, B1, B2, C, C1, C2, dx, dx1, dx2, dy, dy1, dy2, dz, 
            dz1, dz2, J, JLsq, JL1sq, JL2sq, dr, dr1, dr2, W, M1, M2, M3, U1, U2, U3;
    double L[2], L1[2], L2[2], N[6], N1[6], N2[6], XL[3], XL1[3], XL2[3];
    double xs[3][3][2];
 
    /* Initialize */
    if(FLAG == 0) for(i = 0; i < NODES_IN_ELEM; i++) Int[i] = 0.0;
    else for(i = 0; i < 18; i++) Int[i] = 0.0;

    /* Reorder so that singularity is at node 5 */
    if(SinNode == 1){
        for(i = 0; i < 3; i++){
            shift(X[4][i],X[0][i],X[2][i]);
            shift(X[5][i],X[1][i],X[3][i]);         
        }
    }
    else if(SinNode == 3){
        for(i = 0; i < 3; i++){
            shift(X[4][i],X[2][i],X[0][i]);
            shift(X[3][i],X[1][i],X[5][i]);
        }
    }
    
    /* Setup 3 initial triangles */
    xs[0][0][0] = 1.0;  xs[0][0][1] = 0.0;
    xs[0][1][0] = 0.5;  xs[0][1][1] = 0.5;
    xs[0][2][0] = 0.5;  xs[0][2][1] = 0.0;

    xs[1][0][0] = 0.5;  xs[1][0][1] = 0.0;
    xs[1][1][0] = 0.5;  xs[1][1][1] = 0.5;
    xs[1][2][0] = 0.0;  xs[1][2][1] = 0.5;
    
    xs[2][0][0] = 0.5;  xs[2][0][1] = 0.5;
    xs[2][1][0] = 0.0;  xs[2][1][1] = 1.0;
    xs[2][2][0] = 0.0;  xs[2][2][1] = 0.5;
    
    /* Start subdivision loop */
    J = 1.0;
    for(m = 0; m < NSUBDIVISIONS; m++){
        J = J*0.25;
    
        /* Integrate over the 3 triangles furthest away from the singularity */
        for(i = 0; i < 3; i++){
            
            /* Integrate over each triangle using Gauss-Legendre quadrature */
            for(j = 0; j < TSNGAUSS; j++){
                M1 = TSGauss[j][0];
                M2 = TSGauss[j][1];
                M3 = 1.0 - M1 - M2;
                W = TSGauss[j][2];

                /**************************************************** 
                 * Get L and N as a function of (M1,M2). This is the *
                 * transformation between the unit triangle (M1,M2)  *
                 * and the smaller subtriangle (L1,L2)               *
                 ****************************************************/
                if(SinNode%2 == 1){
                    L[0] = xs[i][0][0]*M1 + xs[i][1][0]*M2 + xs[i][2][0]*M3;
                    L[1] = xs[i][0][1]*M1 + xs[i][1][1]*M2 + xs[i][2][1]*M3;
                    shape_tria6(L,N);
                    JLsq = X2L_tria6(X,XL,L,N);
                    
                    /* Auxiliar quantities */
                    dx = Xeval[0] - XL[0];
                    dy = Xeval[1] - XL[1];
                    dz = Xeval[2] - XL[2];
                    dr = dx*dx + dy*dy + dz*dz;
                    A = J*W*sqrt( JLsq/(dr*dr*dr) );
                    
                    if(FLAG == 0){
                        A = A*(dx*normal[0] + dy*normal[1] + dz*normal[2]);
                        for(k = 0; k < NODES_IN_ELEM; k++) Int[k] -= A*N[k];
                    }
                    else{
                        C = A*dz;
                        B = A*dy;
                        A = A*dx;
                        for(k = 0; k < NODES_IN_ELEM; k++){
                            Int[k*3  ] += A*N[k];
                            Int[k*3+1] += B*N[k];
                            Int[k*3+2] += C*N[k];
                        }
                    }
                }
                else{
                    if(SinNode == 2){
                        U1 = 0.5*M3; U2 = M1 + 0.5*M3; U3 = 1.0 - U1 - U2;
                        L1[0] = xs[i][0][0]*U1 + xs[i][1][0]*U2 + xs[i][2][0]*U3;
                        L1[1] = xs[i][0][1]*U1 + xs[i][1][1]*U2 + xs[i][2][1]*U3;
                        U1 = M1 + 0.5*M3; U2 = M3; U3 = 1.0 - U1 - U2;
                        L2[0] = xs[i][0][0]*U1 + xs[i][1][0]*U2 + xs[i][2][0]*U3;
                        L2[1] = xs[i][0][1]*U1 + xs[i][1][1]*U2 + xs[i][2][1]*U3;
                    }
                    else if(SinNode == 4){
                        U1 = M1; U2 = 0.5*M3; U3 = 1.0 - U1 - U2;
                        L1[0] = xs[i][0][0]*U1 + xs[i][1][0]*U2 + xs[i][2][0]*U3;
                        L1[1] = xs[i][0][1]*U1 + xs[i][1][1]*U2 + xs[i][2][1]*U3;
                        U1 = M1; U2 = M2 + 0.5*M3; U3 = 1.0 - U1 - U2;
                        L2[0] = xs[i][0][0]*U1 + xs[i][1][0]*U2 + xs[i][2][0]*U3;
                        L2[1] = xs[i][0][1]*U1 + xs[i][1][1]*U2 + xs[i][2][1]*U3;
                    }
                    else{
                        U1 = 0.5*M3; U2 = M1; U3 = 1.0 - U1 - U2;
                        L1[0] = xs[i][0][0]*U1 + xs[i][1][0]*U2 + xs[i][2][0]*U3;
                        L1[1] = xs[i][0][1]*U1 + xs[i][1][1]*U2 + xs[i][2][1]*U3;
                        U1 = M1 + 0.5*M3; U2 = M2; U3 = 1.0 - U1 - U2;
                        L2[0] = xs[i][0][0]*U1 + xs[i][1][0]*U2 + xs[i][2][0]*U3;
                        L2[1] = xs[i][0][1]*U1 + xs[i][1][1]*U2 + xs[i][2][1]*U3;
                    }
                    shape_tria6(L1,N1);
                    shape_tria6(L2,N2);
                    JL1sq = X2L_tria6(X,XL1,L1,N1);
                    JL2sq = X2L_tria6(X,XL2,L2,N2);   

                    /* Auxiliar quantities */
                    dx1 = Xeval[0] - XL1[0];
                    dy1 = Xeval[1] - XL1[1];
                    dz1 = Xeval[2] - XL1[2];
                    dx2 = Xeval[0] - XL2[0];
                    dy2 = Xeval[1] - XL2[1];
                    dz2 = Xeval[2] - XL2[2];
                    dr1 = dx1*dx1 + dy1*dy1 + dz1*dz1;
                    dr2 = dx2*dx2 + dy2*dy2 + dz2*dz2;
                    A1 = 0.5*J*W*sqrt( JL1sq/(dr1*dr1*dr1) );
                    A2 = 0.5*J*W*sqrt( JL2sq/(dr2*dr2*dr2) );
                    
                    if(FLAG == 0){
                        A1 = A1*(dx1*normal[0] + dy1*normal[1] + dz1*normal[2]);
                        A2 = A2*(dx2*normal[0] + dy2*normal[1] + dz2*normal[2]);

                        for(k = 0; k < NODES_IN_ELEM; k++) Int[k] -= A1*N1[k] + A2*N2[k];
                    }
                    else{
                        C1 = A1*dz1;
                        B1 = A1*dy1;
                        A1 = A1*dx1;
                        C2 = A2*dz2;
                        B2 = A2*dy2;
                        A2 = A2*dx2;
                        for(k = 0; k < NODES_IN_ELEM; k++){
                            Int[k*3  ] += A1*N1[k] + A2*N2[k];
                            Int[k*3+1] += B1*N1[k] + B2*N2[k];
                            Int[k*3+2] += C1*N1[k] + B2*N2[k];
                        }
                    }
                }
            }
        }
   
        /* Set triangles for new subdivision */
        for(i = 0; i < 3; i++) {
            for(j = 0; j < 3; j++){
                xs[i][j][0] = xs[i][j][0]*0.5;
                xs[i][j][1] = xs[i][j][1]*0.5;
            }
        }
    }

    /* Unscramble node contributions to return them in their right places */
    if(FLAG == 0){
        if(SinNode == 1){
            shift(Int[2],Int[0],Int[4]);
            shift(Int[3],Int[1],Int[5]);            
        }
        else if(SinNode == 3){
            shift(Int[0],Int[2],Int[4]);
            shift(Int[5],Int[1],Int[3]);
        }
    }
    else{
        if(SinNode == 1){
            for(i = 0; i < 3; i++){
                shift(Int[6+i],Int[i],Int[12+i]);
                shift(Int[9+i],Int[3+i],Int[15+i]);         
            }
        }
        else if(SinNode == 3){
            for(i = 0; i < 3; i++){
                shift(Int[i],Int[6+i],Int[12+i]);
                shift(Int[15+i],Int[3+i],Int[9+i]);
            }
        }
    }

    return 0;
}

