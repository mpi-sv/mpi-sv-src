/******************************************************************************
* File     : assembly-mpi.c                                                   *
* Author   : Carlos Rosales Fernandez (carlos.rosales.fernandez(at)gmail.com) *
* Revision : 1.0 (2008-12-23)                                                 *
******************************************************************************/
/**
 * @brief Definition of matrix assembly functions.
 *
 * @file
 * Definition of the two functions that perform the matrix assembly for
 * depSolver-mpi: electricFormA_tria3, electricFormA_tria6.
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
#include "assembly-mpi.h"
#include "constants.h"
#include "kernelIntegrals.h"
#include "memHandler.h"
#include "normals.h"

extern unsigned int nElems, nInterfaces, nMats, nNodes;
extern int         proc;
extern int         nodeMin, nodeMax, localNodes, localSize;


/**
 * Forms the coefficient matrix \a mA and the constant vector \a vB for the
 * electrostatic problem described by the linear system \a mA .\a X = \a vB.
 *
 * The coefficient matrix is assembled from the element contribution
 * submatrices,\a SubA. Uses symmetry to reduce storage from (2nNodes x 2nNodes)
 * to (nNodes x 2nNodes) by not duplicating the imaginary terms in \a mA.
 *
 * Works for linear interpolation in triangular elements (3-noded triangles).
 *
 * @param mNodes  : [ Input ] Coordinates of all nodes in the computational domain
 * @param mElems  : [ Input ] Connectivity of all nodes in the computational domain
 * @param vBCType : [ Input ] Boundary condition type for each node in the computational domain
 * @param vInterfaces : [ Input ] Material IDs for each dielectric interface in the simulation
 * @param vMatParam : [ Input ] Electric properties of dielectric materials in the simulation
 * @param vProbParam : [ Input ] Frequency of the applied electric field
 * @param mA : [ Output ] Coefficient matrix for the electrostatic problem
 * @param mBC : [ Input ] Boundary conditions for each node in the computational domain
 * @param vB : [ Output ] Constant vector for the electrostatic problem
 */
int electricFormA_tria3(double **mNodes,
                        unsigned int **mElems,
                        unsigned int *vBCType,
                        unsigned int **vInterfaces,
                        double **vMatParam,
                        double *vProbParam,
                        double **mA,
                        double **mBC,
                        double *vBTemp)
{
    const unsigned int FLAG = 0, NODES_IN_ELEM = 3;
    unsigned int i, interfaceID, j, k, mat1, mat2;
    unsigned int col, currentNode, row, SinNode;  
    double C, D, EpsDif, EpsTot, SigDif, SigTot, W;
    double Xeval[3], SubA[3], normal[3];
    double X[3][3];
    double *A, *B;

    /* Initialize pointers to arrays that may not need to be allocated */
    A = NULL;
    B = NULL;

    /* Auxiliar quantities for eqs at conductors */
    C = 1.0/(pi4*eps0);
    interfaceID = 0;

    /* Auxiliar quantities for eqs at dielectric interfaces */
    if(nInterfaces > 0){
        W = vProbParam[0]*pi2*eps0;
        A = doubleVector(nInterfaces,0);
        B = doubleVector(nInterfaces,0);
 
        for(i = 0; i < nInterfaces; i++){
            mat1 = vInterfaces[i][0] - 1;
            mat2 = vInterfaces[i][1] - 1;
            SigTot = vMatParam[mat1][0] + vMatParam[mat2][0];
            SigDif = vMatParam[mat1][0] - vMatParam[mat2][0];
            EpsTot = W*(vMatParam[mat1][1] + vMatParam[mat2][1]);
            EpsDif = W*(vMatParam[mat1][1] - vMatParam[mat2][1]);
            D = C/(SigTot*SigTot + EpsTot*EpsTot);
            A[i] = D*(SigTot*SigDif + EpsTot*EpsDif);
            B[i] = D*(SigDif*EpsTot - SigTot*EpsDif);
        }
    }

    /* Find contributions to each node and assemble coefficient matrix */
    for(i = nodeMin; i <= nodeMax; i++){
    
        col = i - nodeMin;
        Xeval[0] = mNodes[i][0];
        Xeval[1] = mNodes[i][1];
        Xeval[2] = mNodes[i][2];

        /* Operations necessary only for dielectric interfaces */
        if(vBCType[i] == 0 || vBCType[i] == 6){
            
            /* Get interface and normal at evaluation point */
            interfaceID = (unsigned int)mBC[i][1] - 1;
            getNormal_tria3(i+1,mNodes,mElems,normal);

            /* Add diagonal terms due to jump across interface */
            mA[col][2*i] -= 0.5/eps0;
        }

        /* Loop through elements doing integrals for each pair of nodes */
        for(j = 0; j < nElems; j++){
                
            /* Test for singular case */
            SinNode = 0;
            for(k = 0; k < NODES_IN_ELEM; k++){
                currentNode = mElems[j][k] - 1;
                X[k][0] = mNodes[currentNode][0];
                X[k][1] = mNodes[currentNode][1];
                X[k][2] = mNodes[currentNode][2];
                if(currentNode == i) SinNode = k + 1;
            }

            /* Potential given */
            if(vBCType[i] == 1){
                if(SinNode == 0) intG_tria3(X,Xeval,SubA);
                else intSingularG_tria3(SinNode,X,Xeval,SubA);
            
                /* Assign contributions to the right places in the matrix */
                for(k = 0; k < NODES_IN_ELEM; k++){
		    row = 2*( mElems[j][k] - 1 );
                    mA[col][row] += C*SubA[k];
                }
            }
        
            /* Dielectric interface */
            else{
                if(SinNode == 0) intH_tria3(FLAG,X,Xeval,SubA,normal);
                else intSingularH_tria3(FLAG,SinNode,X,Xeval,SubA,normal);
            
                /* Assign contributions to the right places in the matrix */
                for(k = 0; k < NODES_IN_ELEM; k++){
                    row = 2*( mElems[j][k] - 1 );
                    mA[col][row    ] += A[interfaceID]*SubA[k];
                    mA[col][row + 1] += B[interfaceID]*SubA[k];
                }
            }
        }

        /* Assemble right hand side vector */
        vBTemp[2*col    ] = mBC[i         ][0];
        vBTemp[2*col + 1] = mBC[i + nNodes][0];
    }

    /* Free only when necessary */
    if(nInterfaces > 0){
        free(A);
        free(B);
    }

    return 0;
}


/**
 * Forms the coefficient matrix \a mA and the constant vector \a vB for the
 * electrostatic problem described by the linear system \a mA .\a X = \a vB.
 *
 * The coefficient matrix is assembled from the element contribution
 * submatrices,\a SubA. Uses symmetry to reduce storage from (2nNodes x 2nNodes)
 * to (nNodes x 2nNodes) by not duplicating the imaginary terms in \a mA.
 *
 * Works for quadratic interpolation in triangular elements (6-noded triangles).
 *
 * @param mNodes  : [ Input ] Coordinates of all nodes in the computational domain
 * @param mElems  : [ Input ] Connectivity of all nodes in the computational domain
 * @param vBCType : [ Input ] Boundary condition type for each node in the computational domain
 * @param vInterfaces : [ Input ] Material IDs for each dielectric interface in the simulation
 * @param vMatParam : [ Input ] Electric properties of dielectric materials in the simulation
 * @param vProbParam : [ Input ] Frequency of the applied electric field
 * @param mA : [ Output ] Coefficient matrix for the electrostatic problem
 * @param mBC : [ Input ] Boundary conditions for each node in the computational domain
 * @param vB : [ Output ] Constant vector for the electrostatic problem
 */
int electricFormA_tria6(double **mNodes,
                        unsigned int **mElems,
                        unsigned int *vBCType,
                        unsigned int **vInterfaces,
                        double **vMatParam,
                        double *vProbParam,
                        double **mA,
                        double **mBC,
                        double *vBTemp)
{
    const unsigned int FLAG = 0, NODES_IN_ELEM = 6;
    unsigned int i, interfaceID, j, k, mat1, mat2;
    unsigned int col, currentNode, row, SinNode;  
    double C, D, EpsDif, EpsTot, SigDif, SigTot, W;
    double Xeval[3], SubA[6], normal[3];
    double X[6][3];
    double *A, *B;

    /* Initialize pointers to arrays that may not need to be allocated */
    A = NULL;
    B = NULL;

    /* Auxiliar quantities for eqs at conductors */
    C = 1.0/(pi4*eps0);
    interfaceID = 0;

    /* Auxiliar quantities for eqs at dielectric interfaces */
    if(nInterfaces > 0){
        W = vProbParam[0]*pi2*eps0;
        A = doubleVector(nInterfaces,0);
        B = doubleVector(nInterfaces,0);

        for(i = 0; i < nInterfaces; i++){
            mat1 = vInterfaces[i][0] - 1;
            mat2 = vInterfaces[i][1] - 1;
            SigTot = vMatParam[mat1][0] + vMatParam[mat2][0];
            SigDif = vMatParam[mat1][0] - vMatParam[mat2][0];
            EpsTot = W*(vMatParam[mat1][1] + vMatParam[mat2][1]);
            EpsDif = W*(vMatParam[mat1][1] - vMatParam[mat2][1]);
            D = C/(SigTot*SigTot + EpsTot*EpsTot);
            A[i] = D*(SigTot*SigDif + EpsTot*EpsDif);
            B[i] = D*(SigDif*EpsTot - SigTot*EpsDif);
        }
    }

    /* Find contributions to each node and assemble coefficient matrix */
    for(i = nodeMin; i <= nodeMax; i++){
    
        col = i - nodeMin;
        Xeval[0] = mNodes[i][0];
        Xeval[1] = mNodes[i][1];
        Xeval[2] = mNodes[i][2];

        /* Operations necessary only for dielectric interfaces */
        if(vBCType[i] == 0 || vBCType[i] == 6){

            /* Get interface and normal at evaluation point */
            interfaceID = (unsigned int)mBC[i][1] - 1;
            getNormal_tria6(i+1,mNodes,mElems,normal);

            /* Add diagonal terms due to jump across interface */
            mA[col][2*i] -= 0.5/eps0;
        }  

        /* Loop through elements doing integrals for each pair of nodes */
        for(j = 0; j < nElems; j++){
            
            /* Test for singular case */
            SinNode = 0;
            for(k = 0; k < NODES_IN_ELEM; k++){
                currentNode = mElems[j][k] - 1;
                X[k][0] = mNodes[currentNode][0];
                X[k][1] = mNodes[currentNode][1];
                X[k][2] = mNodes[currentNode][2];
                if(currentNode == i) SinNode = k + 1;
            }

            /* Potential given */
            if(vBCType[i] == 1){
                if(SinNode == 0) intG_tria6(X,Xeval,SubA);
                else intSingularG_tria6(SinNode,X,Xeval,SubA);

                /* Assign contributions to the right places in the matrix */
                for(k = 0; k < NODES_IN_ELEM; k++){
                    row = 2*( mElems[j][k] - 1 );
                    mA[col][row] += C*SubA[k];
                }
            }
        
            /* Dielectric interface */
            else{
                if(SinNode == 0) intH_tria6(FLAG,X,Xeval,SubA,normal);
                else intSingularH_tria6(FLAG,SinNode,X,Xeval,SubA,normal);

                /* Assign contributions to the right places in the matrix */
                for(k = 0; k < NODES_IN_ELEM; k++){
                    row = 2*( mElems[j][k] - 1 );
                    mA[col][row    ] += A[interfaceID]*SubA[k];
                    mA[col][row + 1] += B[interfaceID]*SubA[k];
                }
            }
        }

        /* Assemble right hand side vector */
        vBTemp[2*col    ] = mBC[i         ][0];
        vBTemp[2*col + 1] = mBC[i + nNodes][0];
    }

    /* Free only when necessary */
    if(nInterfaces > 0){
        free(A);
        free(B);
    }

    return 0;
}

