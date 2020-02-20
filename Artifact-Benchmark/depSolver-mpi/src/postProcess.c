/******************************************************************************
* File     : postProcess.c                                                    *
* Author   : Carlos Rosales Fernandez (carlos.rosales.fernandez(at)gmail.com) *
* Revision : 1.0 (2008-12-23)                                                 *
******************************************************************************/
/**
 * @brief Post-processing functions
 *
 * @file
 * This file contains the definitions of the seven post-processing
 * driver functions: headerSetup, postProcessSetup, multipoleSetup,
 * postProcessCol_tria3, postProcessCol_tria6, postProcess_tria3,
 * postProcess_tria6
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
#include <string.h>
#include <math.h>
#include "constants.h"
#include "memHandler.h"
#include "field.h"
#include "force.h"
#include "postProcess.h"
#include "potential.h"

extern FILE *file_log;
extern unsigned int nElems, nInternalPoints, nForceElems, nForcePoints, 
                   nCols, nColType, NX, NY, NZ;
extern int pointMin, pointMax, localPoints, localForcePoints, proc;
extern const int master;
extern double deltaX, deltaY, deltaZ;

/**
 * @brief Save post-processed data to file
 *
 * @file
 * This file saves the post-processed data corresponding to potential and
 * electric field to file from the master only.
 */
int saveData(unsigned int ANALYSIS,
             char *cOutputType,
             double **Xinner,
             double **XF,
             double **globalXe,
             double **globalPot,
             double **globalField,
             double **globalForce,
             double *globalFcm,
             double *globalXcm)
{
    FILE *fp, *fp1, *fp2;
    unsigned int i;

    if( strcmp(cOutputType,"STD") == 0 ){
        switch(ANALYSIS){

            case(0):
            fp1 = fopen("potential.dat","w");
            fprintf(fp1,"#x\t\ty\t\tz\t\tRe[Pot(x,y,z)]\tIm[Pot(x,y,z)]\n");
            for(i = 0; i < nInternalPoints; i++){
                fprintf(fp1,"%e\t%e\t%e\t",Xinner[i][0],Xinner[i][1],Xinner[i][2]);
                fprintf(fp1,"%e\t%e\n",globalPot[i][0],globalPot[i][1]);
            }
            fclose(fp1);
            break;

            case(1):
            fp2 = fopen("field.dat","w");
            fprintf(fp2,"#x\t\ty\t\tz\t\tRe[Ex(x,y,z)]\tIm[Ex(x,y,z)]\tRe[Ey(x,y,z)]");
            fprintf(fp2,"\tIm[Ey(x,y,z)]\tRe[Ez(x,y,z)]\tIm[Ez(x,y,z)]\n");
            for(i = 0; i < nInternalPoints; i++){
                fprintf(fp2,"%e\t%e\t%e\t",Xinner[i][0],Xinner[i][1],Xinner[i][2]);
                fprintf(fp2,"%e\t%e\t%e\t",globalField[i][0],globalField[i][1],globalField[i][2]);
                fprintf(fp2,"%e\t%e\t%e\n",globalField[i][3],globalField[i][4],globalField[i][5]);
            }
            fclose(fp2);
            break;

            default:
            fp1 = fopen("potential.dat","w");
            fp2 = fopen("field.dat","w");
            fprintf(fp1,"#x\t\ty\t\tz\t\tRe[Pot(x,y,z)]\tIm[Pot(x,y,z)]\n");
            fprintf(fp2,"#x\t\ty\t\tz\t\tRe[Ex(x,y,z)] \tIm[Ex(x,y,z)]\tRe[Ey(x,y,z)]");
            fprintf(fp2,"\tIm[Ey(x,y,z)] \tRe[Ez(x,y,z)]\tIm[Ez(x,y,z)]\n");
            for(i = 0; i < nInternalPoints; i++){
                fprintf(fp1,"%e\t%e\t%e\t",Xinner[i][0],Xinner[i][1],Xinner[i][2]);
                fprintf(fp1,"%e\t%e\n",globalPot[i][0],globalPot[i][1]);
                fprintf(fp2,"%e\t%e\t%e\t",Xinner[i][0],Xinner[i][1],Xinner[i][2]);
                fprintf(fp2,"%e\t%e\t%e\t",globalField[i][0],globalField[i][1],globalField[i][2]);
                fprintf(fp2,"%e\t%e\t%e\n",globalField[i][3],globalField[i][4],globalField[i][5]);
            }
            fclose(fp1);
            fclose(fp2);
            break;
        }
    }
    else if( strcmp(cOutputType,"VTK") == 0 ){
        fp = fopen("results.vtk","w");
        fprintf(fp,"# vtk DataFile Version 2.0\n");
        fprintf(fp,"depSolver v2.0\n");
        fprintf(fp,"ASCII\n");
        fprintf(fp,"\n");
        fprintf(fp,"DATASET STRUCTURED_POINTS\n");
        fprintf(fp,"DIMENSIONS %d %d %d\n",NX,NY,NZ);
        fprintf(fp,"ORIGIN %e %e %e\n",Xinner[0][0],Xinner[0][1],Xinner[0][2]);
        fprintf(fp,"SPACING %e %e %e\n",deltaX,deltaY,deltaZ);
        fprintf(fp,"\n");
        fprintf(fp,"POINT_DATA %d\n",NX*NY*NZ);
        fprintf(fp,"\n");
        
        /* Electric potential (real part) at the required points */
        fprintf(fp,"SCALARS Re[V] double\n");
        fprintf(fp,"LOOKUP_TABLE default\n");
        for(i = 0; i < nInternalPoints; i++)
            fprintf(fp,"%e\n",globalPot[i][0]);

        /* Electric potential (imaginary part) at the required points */
        fprintf(fp,"\n");
        fprintf(fp,"SCALARS Im[V] double\n");
        fprintf(fp,"LOOKUP_TABLE default\n");
        for(i = 0; i < nInternalPoints; i++)
           fprintf(fp,"%e\n",globalPot[i][1]);

        /* Electric field (real part) at the required points */
        fprintf(fp,"\n");
        fprintf(fp,"VECTORS Re[E] double\n");
        for(i = 0; i < nInternalPoints; i++){
            fprintf(fp,"%e\t%e\t%e\n",globalField[i][0],globalField[i][2],globalField[i][4]);
        }

        /* Electric field (imaginary part) at the required points */
        fprintf(fp,"\n");
        fprintf(fp,"VECTORS Im[E] double\n");
        for(i = 0; i < nInternalPoints; i++){
            fprintf(fp,"%e\t%e\t%e\n",globalField[i][1],globalField[i][3],globalField[i][5]);
        }
        fclose(fp);
        
    }
    
    /* Save dep force data */
    if( ANALYSIS == 3 || ANALYSIS == 4 ){
        fp = fopen("force-mst.dat","w");
        fprintf(fp,"#Xcm\t\tYcm\t\tZcm\t\tFXcm\t\tFYcm\t\tFZcm\n");
        fprintf(fp,"%e\t%e\t%e\t",globalXcm[0],globalXcm[1],globalXcm[2]);
        fprintf(fp,"%e\t%e\t%e\n",globalFcm[0],globalFcm[1],globalFcm[2]);
        fprintf(fp,"#Xce\t\tYce\t\tZce\t\tFXce\t\tFYce\t\tFZce\n");
        for(i = 0; i < nForceElems; i++){
            fprintf(fp,"%e\t%e\t%e\t",globalXe[i][0],globalXe[i][1],globalXe[i][2]);
            fprintf(fp,"%e\t%e\t%e\n",globalForce[i][0],globalForce[i][1],globalForce[i][2]);
        }
        fclose(fp);
    }
    else if( ANALYSIS > 4 ){
        fp = fopen("force-mp.dat","w");
        fprintf(fp,"#X\t\tY\t\tZ\t\tFX\t\tFY\t\tFZ\n");
        for(i = 0; i < nForcePoints; i++){
            fprintf(fp,"%e\t%e\t%e\t",XF[i][0],XF[i][1],XF[i][2]);
            fprintf(fp,"%e\t%e\t%e\n",globalForce[i][0],globalForce[i][1],globalForce[i][2]);
        }
        fclose(fp);
    }

    return 0;
}

/**
 * Filter input data to eliminate points inside columns from the post-processing
 * list of points if necessary (STD output only).
 *
 * @param cOutputType : [ Input ] Output format type
 * @param xCols       : [ Input ] Coordinates and radius of columns
 * @param XinerTemp   : [ Input ] Original internal nodes from input files
 * @param Xinner      : [ Output ] Internal nodes where post-processing is to be done
 */
int postProcessSetup(char *cOutputType,
                     double **xCols,
                     double **XinnerTemp,
                     double **Xinner)
{
    unsigned int i, j, nInside, count;
    double dx, dy, x, y, Rsq;

    /* Filter data to avoid calculation inside columns */
    if( nInternalPoints > 0 ){
        if( nCols > 0 && strcmp(cOutputType,"STD") == 0 ){

            nInside = 0;
            for(i = 0; i < nInternalPoints; i++){
                x = XinnerTemp[i][0];
                y = XinnerTemp[i][1];

                count = 0;
                for( j = 0; j < nCols; j++ ){
                    dx  = fabs( x - xCols[j][0] );
                    dy  = fabs( y - xCols[j][1] );
                    Rsq = xCols[j][2]*xCols[j][2];

                    if( nColType == 1 ){
                        if( ( dx*dx +dy*dy ) > Rsq ) count++;
                    }
                    else{
                        if( (dx > xCols[j][2]) && (dy > xCols[j][2]) ) count++;
                    }
                }
                if(count == nCols){
                    XinnerTemp[i][3] = 1.0;
                    nInside++;
                }
                else XinnerTemp[i][3] = -1.0;
            }

            count = 0;
            for( i = 0; i< nInternalPoints; i ++){
                if( XinnerTemp[i][3] > 0.0 ){
                    Xinner[count][0] = XinnerTemp[i][0];
                    Xinner[count][1] = XinnerTemp[i][1];
                    Xinner[count][2] = XinnerTemp[i][2];
                    count++;
                }
            }

            /* Redefine the number of evaluation points */
            nInternalPoints = nInside;
        }
        else{
            for( i = 0; i< nInternalPoints; i ++){
                Xinner[i][0] = XinnerTemp[i][0];
                Xinner[i][1] = XinnerTemp[i][1];
                Xinner[i][2] = XinnerTemp[i][2];
            }
        }
    }

    return 0;
}

/**
 * Set up the type of multipolar approximation (order and shape) required.
 *
 * @param ANALYSIS : [ Input ] Type of post-processing to be done
 * @param axis     : [ Input ] Semi-axis of ellipsoidal particle
 * @param nShape   : [ Output ] Sphere (0) or ellipsoid (1)
 * @param nOrder   : [ Output ] Multipolar approximation order
 * @param R        : [ Output ] Radius of spherical particle
 */
int multipoleSetup(unsigned int ANALYSIS,
                   double *axis,
                   unsigned int *nShape,
                   unsigned int *nOrder,
                   double *R)
{
    /* Default to sphere with dipolar approximation */
    (*nOrder) = 1;
    (*nShape) = 0;
    (*R)      = axis[0];

    switch(ANALYSIS){
        case 5:         /* Dipolar Approximation (Order 1) */
            (*nOrder) = 1;
            break;
        case 6:         /* Quadrupolar Approximation (Order 2) */
            (*nOrder) = 2;
            break;
        case 7:         /* Octupolar Approximation (Order 3) */
            (*nOrder) = 3;
            break;
        case 8:         /* Multipolar Approx Order 4 */
            (*nOrder) = 4;
            break;
        case 9:         /* Multipolar Approx Order 5 */
            (*nOrder) = 5;
            break;
        case 10:        /* Dipolar Approx for Homogeneous Ellipsoid */
            (*nOrder) = 1;
            (*nShape) = 1;
            break;
        case 11:        /* Dipolar Approx for Single-shelled Oblate Ellipsoid */
            (*nOrder) = 1;
            (*nShape) = 1;
            break;
    }

    return 0;
}

/**
 * Calculates potential and electric field at the required points, and stores
 * them in files 'potential.dat' and 'field.dat' using the standard format.
 *
 * It also calculates the force on the dielectric interface if required and
 * stores it in file 'force-mst.dat' or 'force-mp.dat' for MST and multipolar
 * approximations respectively.
 *
 * Works for linear interpolation in triangular elements (3-noded triangles).
 *
 * @param ANALYSIS    : [ Input ] Type of post-processing to be done
 * @param cOutputType : [ Input ] Output format type
 * @param axis        : [ Input ] Semi-axis of ellipsoidal particle
 * @param XF          : [ Input ] Points where the force should be calculated
 * @param Xinner      : [ Input ] Nodes where potential and/or field should be calculated
 * @param mNodes  : [ Input ] Coordinates of all nodes in the computational domain
 * @param mElems  : [ Input ] Connectivity of all nodes in the computational domain
 * @param vMatParam  : [ Input ] Electric properties of dielectric materials
 * @param vProbParam : [ Input ] Frequency of the applied electric field
 * @param vBCType : [ Input ] Boundary condition type for each node in the domain
 * @param vB      : [ Input ] Solution vector for the electrostatic problem
 */
int postProcess_tria3(unsigned int ANALYSIS,
                      double *axis,
                      double **localXF,
                      double **localXe,
                      double **localXinner,
                      double **mNodes,
                      unsigned int **localEF,
                      unsigned int **mElems,
                      unsigned int *global2XF,
                      int *firstForcePoint,
                      int *forceSize,
                      double **vMatParam,
                      double *vProbParam,
                      unsigned int *vBCType,
                      double *vB,
                      double **localPot,
                      double **localField,
                      double **localForce,
                      double *localFcm,
                      double *localXcm)
{
    FILE *fp;
    unsigned int i, nOrder, nShape;
    double Eps, R;
    double Fcm[3], mPot[2], mE[6], Xin[3], Xcm[3], Xeval[3];

    /* Electric field and potential values at the required points */
    if( localPoints > 0 ){
        mPot[0] = mPot[1] = 0.0;
        for(i = 0; i < 6; i++) mE[i] = 0.0;

        for(i = 0; i < localPoints; i++){
        
            Xin[0] = localXinner[i][0];
            Xin[1] = localXinner[i][1];
            Xin[2] = localXinner[i][2];

            if(ANALYSIS == 0){
                potential_tria3(Xin,mNodes,mElems,vB,mPot);
                localPot[i][0] = mPot[0];
                localPot[i][1] = mPot[1];
            }
            else if(ANALYSIS == 1){
                field_tria3(Xin,mNodes,mElems,vB,mE);
                localField[i][0] = mE[0];
                localField[i][1] = mE[1];
                localField[i][2] = mE[2];
                localField[i][3] = mE[3];
                localField[i][4] = mE[4];
                localField[i][5] = mE[5];
            }
            else if(ANALYSIS != 4){
                potential_tria3(Xin, mNodes, mElems, vB, mPot);
                field_tria3(Xin, mNodes, mElems, vB, mE);
                
                localPot[i][0] = mPot[0];
                localPot[i][1] = mPot[1];
                
                localField[i][0] = mE[0];
                localField[i][1] = mE[1];
                localField[i][2] = mE[2];
                localField[i][3] = mE[3];
                localField[i][4] = mE[4];
                localField[i][5] = mE[5];
            }
        }
    }

    /* Calculate Fdep at dielectric interfaces if required */
    if(ANALYSIS == 3 || ANALYSIS == 4){

        Eps = vMatParam[0][1]*eps0;
        forceMST_tria3(mNodes, mElems, localXF, localXe, localEF,
                       global2XF, firstForcePoint, forceSize, vB,
                       Eps, localForce, localFcm, localXcm);

    }

    /* Calculate Fdep at particle centre using multipolar method if required */
    else if(ANALYSIS > 4){

        multipoleSetup(ANALYSIS, axis, &nShape, &nOrder, &R);

        if(nShape == 0){
            forceMultipole_tria3(nOrder, R, localXF, mNodes, mElems, vB,
                                 vMatParam, vProbParam, localForce);
        }
        else{
            forceEllipsoid_tria3(ANALYSIS, nOrder, axis, localXF,
                                 mNodes, mElems, vB, vMatParam,
                                 vProbParam, localForce);
        }
    }

    return 0;
}


/**
 * Calculates potential and electric field at the required points, and stores
 * them in files 'potential.dat' and 'field.dat' using the standard format.
 *
 * It also calculates the force on the dielectric interface if required and
 * stores it in file 'force-mst.dat' or 'force-mp.dat' for MST and multipolar
 * approximations respectively.
 *
 * Works for quadratic interpolation in triangular elements (6-noded triangles).
 *
 * @param ANALYSIS    : [ Input ] Type of post-processing to be done
 * @param cOutputType : [ Input ] Output format type
 * @param axis        : [ Input ] Semi-axis of ellipsoidal particle
 * @param XF          : [ Input ] Points where the force should be calculated
 * @param Xinner      : [ Input ] Nodes where potential and/or field should be calculated
 * @param mNodes  : [ Input ] Coordinates of all nodes in the computational domain
 * @param mElems  : [ Input ] Connectivity of all nodes in the computational domain
 * @param vMatParam  : [ Input ] Electric properties of dielectric materials
 * @param vProbParam : [ Input ] Frequency of the applied electric field
 * @param vBCType : [ Input ] Boundary condition type for each node in the domain
 * @param vB      : [ Input ] Solution vector for the electrostatic problem
 */
int postProcess_tria6(unsigned int ANALYSIS,
                      double *axis,
                      double **localXF,
                      double **localXe,
                      double **localXinner,
                      double **mNodes,
                      unsigned int **localEF,
                      unsigned int **mElems,
                      unsigned int *global2XF,
                      int *firstForcePoint,
                      int *forceSize,
                      double **vMatParam,
                      double *vProbParam,
                      unsigned int *vBCType,
                      double *vB,
                      double **localPot,
                      double **localField,
                      double **localForce,
                      double *localFcm,
                      double *localXcm)
{
    FILE *fp;
    unsigned int i, nOrder, nShape;
    double Eps, R;
    double Fcm[3], mPot[2], mE[6], Xin[3], Xcm[3], Xeval[3];

    /* Calculate electric field and potential */
    if( localPoints > 0 ){
        mPot[0] = mPot[1] = 0.0;
        for(i = 0; i < 6; i++) mE[i] = 0.0;

        for(i = 0; i < localPoints; i++){
        
            Xin[0] = localXinner[i][0];
            Xin[1] = localXinner[i][1];
            Xin[2] = localXinner[i][2];

            if(ANALYSIS == 0){
                potential_tria6(Xin,mNodes,mElems,vB,mPot);
                localPot[i][0] = mPot[0];
                localPot[i][1] = mPot[1];
            }
            else if(ANALYSIS == 1){
                field_tria6(Xin,mNodes,mElems,vB,mE);
                localField[i][0] = mE[0];
                localField[i][1] = mE[1];
                localField[i][2] = mE[2];
                localField[i][3] = mE[3];
                localField[i][4] = mE[4];
                localField[i][5] = mE[5];
            }
            else{
                potential_tria6(Xin,mNodes,mElems,vB,mPot);
                field_tria6(Xin,mNodes,mElems,vB,mE);
                
                localPot[i][0] = mPot[0];
                localPot[i][1] = mPot[1];
                
                localField[i][0] = mE[0];
                localField[i][1] = mE[1];
                localField[i][2] = mE[2];
                localField[i][3] = mE[3];
                localField[i][4] = mE[4];
                localField[i][5] = mE[5];
            }
        }
    }

    /* Calculate Fdep at dielectric interfaces */
    if(ANALYSIS == 3 || ANALYSIS == 4){

        Eps = vMatParam[0][1]*eps0;
        forceMST_tria6(mNodes, mElems, localXF, localXe, localEF,
                       global2XF, firstForcePoint, forceSize, vB,
                       Eps, localForce, localFcm, localXcm);

    }

    /* Calculate Fdep at particle centre using multipolar method */
    else if(ANALYSIS > 4){
        multipoleSetup(ANALYSIS, axis, &nShape, &nOrder, &R);

        if(nShape == 0){
            forceMultipole_tria6(nOrder, R, localXF, mNodes, mElems, vB,
                                 vMatParam, vProbParam, localForce);
        }
        else{
            forceEllipsoid_tria6(ANALYSIS, nOrder, axis, localXF,
                                 mNodes, mElems, vB, vMatParam,
                                 vProbParam, localForce);
        }
    }

    return 0;
}


