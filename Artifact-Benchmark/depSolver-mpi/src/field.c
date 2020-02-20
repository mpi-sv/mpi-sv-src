/******************************************************************************
* File     : field.c                                                          *
* Author   : Carlos Rosales Fernandez (carlos.rosales.fernandez(at)gmail.com) *
* Revision : 1.0 (2008-12-23)                                                 *
******************************************************************************/
/**
 * @brief Electric field calculation
 *
 * @file
 * Functions necessary to calculate the electric field in depSolver: \n
 * field_tria3 and field_tria6.
 *
 * Both functions receive the evaluation point coordinates in array \a Xin and
 * return the complex electric field array \a mE.
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
#include "field.h"
#include "kernelIntegrals.h"
#include "normals.h"

extern unsigned int nElems, nNodes;

/**
 * Calculates the Electric Field at the required point \a Xin and returns its
 * real and imaginary parts in \a mE :
 *
 * \a mE[0] = Re(Ex), \a mE[2] = Re(Ey), \a mE[4] = Re(Ez), \n
 * \a mE[1] = Im(Ex), \a mE[3] = Im(Ey), \a mE[5] = Im(Ez).
 *
 * Works for linear interpolation in triangular elements (3-noded triangles).
 *
 * @param Xin    : [ Input ]  Coordinates of evaluation point
 * @param mNodes : [ Input ]  Coordinates of all nodes in the computational domain
 * @param mElems : [ Input ]  Connectivity of all nodes in the computational domain
 * @param vB     : [ Input ]  Electrostatic problem solution vector
 * @param mE     : [ Output ] Electric field
 */
int field_tria3(double *Xin,
                double **mNodes,
                unsigned int **mElems,
                double *vB,
                double *mE)
{
    const unsigned int ELEMS = nElems, DIM = 3, FLAG = 1, NODES_IN_ELEM = 3;
    unsigned int i, j, k, currentNode, SinNode, test;
    double A, B, dx, dy, dz;
    double normal[3], SubE[9];
    double X[3][3];

    /* Initialize */
    B = 0.5/eps0;
    A = B/pi2;
    for(i = 0; i < 6; i++) mE[i] = 0.0;

    for(i = 0; i < ELEMS; i++){

        /* Get coordinates of nodes in this element */
        for(j = 0; j < NODES_IN_ELEM; j++){
            currentNode = mElems[i][j] - 1;
            X[j][0] = mNodes[currentNode][0];
            X[j][1] = mNodes[currentNode][1];
            X[j][2] = mNodes[currentNode][2];
        }

        /* Test for singular case */
        test = 0;
        SinNode = 0;
        for(j = 0; j < NODES_IN_ELEM; j++){
            dx = Xin[0] - X[j][0];
            dy = Xin[1] - X[j][1];
            dz = Xin[2] - X[j][2];
            if(dx == 0 && dy == 0.0 && dz == 0.0 && test == 0){
                test = 1;
                SinNode = j+1;
                getNormal_tria3(mElems[i][j],mNodes,mElems,normal);
            }
        }

        /* Integrate to get contributions from all j nodes in element i */
        if(test == 1) intSingularH_tria3(FLAG,SinNode,X,Xin,SubE,normal);
        else intH_tria3(FLAG,X,Xin,SubE,normal);

        /* Add contribution to field from element i */
        for(j = 0; j < NODES_IN_ELEM; j++){
            currentNode = 2*( mElems[i][j] - 1 );
            for(k = 0; k < DIM; k++){
                mE[k*2  ] += A*SubE[j*DIM+k]*vB[currentNode    ];  /* Re(E) */
                mE[k*2+1] += A*SubE[j*DIM+k]*vB[currentNode + 1];  /* Im(E) */
            }
            if(test == 1){
                for(k = 0; k < DIM; k++){
                    mE[k*2  ] += B*vB[currentNode    ]*normal[k];
                    mE[k*2+1] += B*vB[currentNode + 1]*normal[k];
                }
                test = 2;
            }
        }
    }

    return 0;
}

/**
 * Calculates the Electric Field at the required point \a Xin and returns its
 * real and imaginary parts in \a mE :
 *
 * \a mE[0] = Re(Ex), \a mE[2] = Re(Ey), \a mE[4] = Re(Ez), \n
 * \a mE[1] = Im(Ex), \a mE[3] = Im(Ey), \a mE[5] = Im(Ez).
 *
 * Works for quadratic interpolation in triangular elements (6-noded triangles).
 *
 * @param Xin    : [ Input ]  Coordinates of evaluation point
 * @param mNodes : [ Input ]  Coordinates of all nodes in the computational domain
 * @param mElems : [ Input ]  Connectivity of all nodes in the computational domain
 * @param vB     : [ Input ]  Electrostatic problem solution vector
 * @param mE     : [ Output ] Electric field
 */
int field_tria6(double *Xin,
                double **mNodes,
                unsigned int **mElems,
                double *vB,
                double *mE)
{
    const unsigned int ELEMS = nElems, DIM = 3, FLAG = 1, NODES_IN_ELEM = 6;
    unsigned int i, j, k, currentNode, SinNode, test;
    double A, B, dx, dy, dz;
    double normal[3], SubE[18];
    double X[6][3];

    /* Initialize */
    B = 0.5/eps0;
    A = B/pi2;
    for(i = 0; i < 6; i++) mE[i] = 0.0;

    for(i = 0; i < ELEMS; i++){
        
        /* Get coordinates of nodes in this element */
        for(j = 0; j < NODES_IN_ELEM; j++){
            currentNode = mElems[i][j] - 1;
            X[j][0] = mNodes[currentNode][0];
            X[j][1] = mNodes[currentNode][1];
            X[j][2] = mNodes[currentNode][2];
        }

        /* Test for singular case */
        test = 0;
        SinNode = 0;
        for(j = 0; j < NODES_IN_ELEM; j++){
            dx = Xin[0] - X[j][0];
            dy = Xin[1] - X[j][1];
            dz = Xin[2] - X[j][2];
            if(dx == 0 && dy == 0.0 && dz == 0.0 && test == 0){
                test = 1;
                SinNode = j+1;
                getNormal_tria6(mElems[i][j],mNodes,mElems,normal);
            }
        }

        /* Integrate to get contributions from all j nodes in element i */
        if(test == 1) intSingularH_tria6(FLAG,SinNode,X,Xin,SubE,normal);
        else intH_tria6(FLAG,X,Xin,SubE,normal);

        /* Add contribution to field from element i */
        for(j = 0; j < NODES_IN_ELEM; j++){
            currentNode = 2*( mElems[i][j] - 1 );
            for(k = 0; k < DIM; k++){
                mE[k*2  ] += A*SubE[j*DIM+k]*vB[currentNode    ];  /* Re(E) */
                mE[k*2+1] += A*SubE[j*DIM+k]*vB[currentNode + 1];  /* Im(E) */
            }
            if(test == 1){
                for(k = 0; k < DIM; k++){
                    mE[k*2  ] += B*vB[currentNode    ]*normal[k];
                    mE[k*2+1] += B*vB[currentNode + 1]*normal[k];
                }
                test = 2;
            }
        }
    }

    return 0;
}

