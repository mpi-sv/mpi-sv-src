/******************************************************************************
* File      : potential.c                                                     *
* Author   : Carlos Rosales Fernandez (carlos.rosales.fernandez(at)gmail.com) *
* Revision : 1.0 (2008-12-23)                                                 *
******************************************************************************/
/**
 * @brief Potential calculation functions
 *
 * @file
 * Definition of the two functions required to calculate the electrical
 * potential in depSolver: potential_tria3 , potential_tria6.
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
#include "kernelIntegrals.h"
#include "normals.h"
#include "potential.h"

extern unsigned int nElems, nNodes;

/**
 * Calculates the electric potential at the required point \a Xin and returns
 *  its real and imaginary parts in array \a mPot:
 *
 * \a mPot[0] = Re(V) \n
 * \a mPot[1] = Im(V)
 *
 * Works for linear interpolation in triangular elements (3-noded triangles).
 *
 * @param Xin     : [ Input ] Coordinates of the potential evaluation point
 * @param mNodes  : [ Input ] Coordinates of all nodes in the computational domain
 * @param mElems  : [ Input ] Connectivity of all nodes in the computational domain
 * @param vB      : [ Input ] Solution vector for the electrostatic problem
 * @param mPot    : [ Output ] Electrical potential
 */
int potential_tria3(double *Xin,
                    double **mNodes,
                    unsigned int **mElems,
                    double *vB,
                    double *mPot)
{
    const unsigned int ELEMS = nElems, NODES_IN_ELEM = 3;
    unsigned int currentNode, i, j, test, SinNode; 
    double A, dx, dy, dz;
    double normal[3], SubP[3];
    double X[3][3];

    /* Initialize */
    A = 1.0/(pi4*eps0);
    mPot[0] = mPot[1] = 0.0;
 
    for(i = 0; i < ELEMS; i++){

        /* Coordinates of nodes of this element */
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
            dx = X[j][0] - Xin[0];
            dy = X[j][1] - Xin[1];
            dz = X[j][2] - Xin[2];
            if(dx == 0 && dy == 0.0 && dz == 0.0 && test == 0){
                test = 1;
                SinNode = j+1;
                getNormal_tria3(mElems[i][j],mNodes,mElems,normal);
            }
        }

        /* Contribution from all j nodes in element i */
        if(test == 1) intSingularG_tria3(SinNode,X,Xin,SubP);
        else intG_tria3(X,Xin,SubP);

        /* Add contribution to potential from element i */
        for(j = 0; j < NODES_IN_ELEM; j++)          {
            currentNode = 2*( mElems[i][j] - 1 );
            mPot[0] += A*SubP[j]*vB[currentNode    ];
            mPot[1] += A*SubP[j]*vB[currentNode + 1];
        }
    }

    return 0;
}


/**
 * Calculates the electric potential at the required point \a Xin and returns
 *  its real and imaginary parts in array \a mPot:
 *
 * \a mPot[0] = Re(V) \n
 * \a mPot[1] = Im(V)
 *
 * Works for quadratic interpolation in triangular elements (6-noded triangles).
 *
 * @param Xin     : [ Input ] Coordinates of the potential evaluation point
 * @param mNodes  : [ Input ] Coordinates of all nodes in the computational domain
 * @param mElems  : [ Input ] Connectivity of all nodes in the computational domain
 * @param vB      : [ Input ] Solution vector for the electrostatic problem
 * @param mPot    : [ Output ] Electrical potential
 */
int potential_tria6(double *Xin,
                    double **mNodes,
                    unsigned int **mElems, 
                    double *vB,
                    double *mPot)
{
    const unsigned int ELEMS = nElems, NODES_IN_ELEM = 6;
    unsigned int currentNode, i, j, test, SinNode; 
    double A, dx, dy, dz;
    double normal[3], SubP[6];
    double X[6][3];

    /* Initialize */
    A = 1.0/(pi4*eps0);
    mPot[0] = mPot[1] = 0.0;
 
    for(i = 0; i < ELEMS; i++){
        
        /* Coordinates of nodes of this element */
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
            dx = X[j][0] - Xin[0];
            dy = X[j][1] - Xin[1];
            dz = X[j][2] - Xin[2];
            if(dx == 0 && dy == 0.0 && dz == 0.0 && test == 0){
                test = 1;
                SinNode = j+1;
                getNormal_tria6(mElems[i][j],mNodes,mElems,normal);
            }
        }

        /* Contribution from all j nodes in element i */
        if(test == 1) intSingularG_tria6(SinNode,X,Xin,SubP);
        else intG_tria6(X,Xin,SubP);

        /* Add contribution to potential from element i */
        for(j = 0; j < NODES_IN_ELEM; j++){
            currentNode = 2*( mElems[i][j] - 1 );
            mPot[0] += A*SubP[j]*vB[currentNode    ];
            mPot[1] += A*SubP[j]*vB[currentNode + 1];
        }
    }

    return 0;
}

