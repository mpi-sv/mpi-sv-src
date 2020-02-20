/******************************************************************************
* File     : depSolver-mpi.c                                                  *
* Author   : Carlos Rosales Fernandez (carlos.rosales.fernandez(at)gmail.com) *
* Revision : 1.0 (2008-12-23)                                                 *
******************************************************************************/
/**
 * @brief Driver for depSolver-mpi.
 *
 * @file
 * Main function for Laplace AC 3D multidomain electrical problem.
 *
 * @param nOut Output Format. nOut == 1 indicates Standard, nOut == 2 is VTK.
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
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <mpi.h>
#include "assembly-mpi.h"
#include "errorHandler.h"
#include "gaussData.h"
#include "gmres-mpi.h"
#include "memHandler.h"
#include "postProcess.h"
#include "preProcess.h"

FILE *file_log;
char cElemType[6];
unsigned int nDim, nElems, nElemType, nForceElems, nForcePoints, nInternalPoints, nInterfaces,
             nMats, nNodes, nNodesAux, nNodesInElem, nCols, nColType, NX, NY, NZ;
int          nodeMin, nodeMax, localNodes, localSize;
int          pointMin, pointMax, localPoints;
int          forceMin, forceMax, localForcePoints;
int          forceElemMin, forceElemMax, localForceElems;
int          nprocs, proc;
const int    master = 0;
double       deltaX, deltaY, deltaZ;

int main(int argc, char *argv[])
{
    FILE *fIn, *fAux, *fp[3];
    time_t currentTime;
    struct tm *localTime;
    double t1,t2,t3,t4,t_tot,t1s,t2s,t3s,t4s,t_tots;
    double axis[3];
    int t1m, t2m, t3m, t4m, t_totm;
    int *procNodes, *procFirst, *procSize;
    int *procPoints, *procFirstPoint, *procCoords;
    int *forcePoints, *firstForcePoint, *forceSize;
    int *forceElems, *firstForceElem, *forceElemSize;
    int *potSize, *potFirstPoint, *fieldSize, *fieldFirstPoint;
    unsigned int **EF, **localEF;
    unsigned int *global2XF;
    unsigned int complete, partial;
    unsigned int dbleBufferSize, uintBufferSize, current;
    unsigned int ANALYSIS, i, j, nBuffer, nInit, preCond, size, internalPtsTemp;
    unsigned int nOut;
    char cFilename[30]="input.bem", cBuffer[33], cBCFile[33], cElemFile[33],
         cNodeFile[33], cInternalPointsFile[33], cSection[33], cTmp[33],
         solver[11], cForcePoints[33], cOutputType[4]="NAN";
    unsigned int *vBCType, *uintParam2, *elemsBuffer;
    unsigned int **vInterfaces, **mElems;
    double *vProbParam, *vB, *vBTemp;
    double localFcm[3], localXcm[3], globalFcm[3], globalXcm[3];
    double **globalXe, **localXe;
    double **localField, **localForce, **localPot, **localXinner, **localXF;
    double **globalField, **globalForce, **globalPot;
    double **vMatParam, **mBC, **mNodes, **Xinner, **XinnerTemp, **mA, **XF, **xCols;
    unsigned int uintParam[11];
    double       dbleParam[ 4];


    /* Start MPI environment */
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &proc);

    printf("P:%d / %d Running\n",proc+1,nprocs);

    /* Housekeeping arrays that allow global MPI exchanges */
    /* Mesh nodes (assembly) */
    procNodes  = (int *)calloc(nprocs,sizeof(int));
    procFirst  = (int *)calloc(nprocs,sizeof(int));
    procSize   = (int *)calloc(nprocs,sizeof(int));
    /* Internal nodes (post-process, potential and field) */
    procPoints = (int *)calloc(nprocs,sizeof(int));
    procCoords = (int *)calloc(nprocs,sizeof(int));
    procFirstPoint = (int *)calloc(nprocs,sizeof(int));
    /* Potential (post-process) */
    potSize       = (int *)calloc(nprocs,sizeof(int));
    potFirstPoint = (int *)calloc(nprocs,sizeof(int));
    /* Field (post-process) */
    fieldSize       = (int *)calloc(nprocs,sizeof(int));
    fieldFirstPoint = (int *)calloc(nprocs,sizeof(int));    
    /* Force Points (post-process) */
    forcePoints     = (int *)calloc(nprocs,sizeof(int));
    forceSize       = (int *)calloc(nprocs,sizeof(int));
    firstForcePoint = (int *)calloc(nprocs,sizeof(int));
    /* Force elements (MST only, post-process) */
    forceElems     = (int *)calloc(nprocs,sizeof(int));
    forceElemSize  = (int *)calloc(nprocs,sizeof(int));
    firstForceElem = (int *)calloc(nprocs,sizeof(int));

    if( proc == master ){
      /* Open and initialize log file */
      file_log = fopen("bem.log","a");
      fprintf(file_log,"********************************************");
      fprintf(file_log,"************************************\n");
      fprintf(file_log,"depSolver-mpi Version 1.0 (Parallel).\n\n");
      fprintf(file_log,"Copyright 2006, 2008 Carlos Rosales Fernandez and IHPC (A*STAR).\n");
      fprintf(file_log,"License: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n");
      fprintf(file_log,"This is free software: you are free to change and redistribute it.\n");
      fprintf(file_log,"There is NO WARRANTY, to the extent permitted by law.\n");
      fprintf(file_log,"********************************************");
      fprintf(file_log,"************************************\n\n");
      fprintf(file_log,"depSolver-mpi(): started @ ");
      currentTime = time(NULL);
      localTime = localtime(&currentTime);
      fputs(asctime(localTime),file_log);
      fclose(file_log);
      file_log = fopen("bem.log","a");
    }

    /* Initialize pointers to arrays that may not need to be allocated */
    vInterfaces = NULL;
    xCols       = NULL;
    XF          = NULL;
    Xinner      = NULL;
    XinnerTemp  = NULL;

    /* Initialize integers related to optional parameters */
    nInterfaces     = 0;
    nMats           = 0;
    nForcePoints    = 0;
    nInternalPoints = 0;
    nCols           = 0;

    /****************************************************************/
    /***     READ MAIN INPUT FILE, input.bem                      ***/
    /****************************************************************/

    if ( proc == master ){

        /* Clean and open input file */
        comFilter(cFilename);
        fIn=fopen(cFilename,"r");

        /* Read node section */
        fscanf(fIn,"%32s %u %32s",cSection,&nNodes,cNodeFile);

        /* Read element section */
        fscanf(fIn,"%32s %u %5s %32s",cSection,&nElems,cElemType,cElemFile);
        for(i = 0; i < 5; i++) cElemType[i] = tolower(cElemType[i]);

        /* Read materials section */
        fscanf(fIn,"%32s %u",cSection,&nMats);
        vMatParam = doubleMatrix(nMats,2,0);
        for(i = 0; i < nMats; i++)
            fscanf(fIn,"%u %le %le",&nBuffer,&vMatParam[i][0],&vMatParam[i][1]);

        /* Read interfaces section */
        fscanf(fIn,"%32s %u",cSection,&nInterfaces);
        if(nInterfaces > 0){
            vInterfaces = uintMatrix(nInterfaces,2,0);
            for(i = 0; i < nInterfaces; i++)
                fscanf(fIn,"%u %u %u",&nBuffer,&vInterfaces[i][0],&vInterfaces[i][1]);
        }

        /* Read problems section */
        fscanf(fIn,"%32s",cSection);
        vProbParam = doubleVector(1,1);     /* This could be a double scalar      */
        fscanf(fIn,"%le",&vProbParam[0]);
        fscanf(fIn,"%32s",cBCFile);

        /* Read analysis section */
        fscanf(fIn,"%32s %u %u",cSection,&preCond,&nInit);
        fscanf(fIn," %u ",&ANALYSIS);
        klee_make_symbolic(&ANALYSIS,sizeof(ANALYSIS),"ANALYSIS");
        klee_assume(ANALYSIS<4);
        printf("Hhaais %d",ANALYSIS);
        /* Multipolar approximations for sphere */
        if((ANALYSIS > 4) && (ANALYSIS < 10))
            fscanf(fIn,"%u %le %32s",&nForcePoints,&axis[0],cForcePoints);

        /* Dipolar approximation for ellipsoid */
        else if(ANALYSIS == 10 || ANALYSIS == 11)
            fscanf(fIn,"%u %le %le %le %32s",&nForcePoints,&axis[0],&axis[1],&axis[2],cForcePoints);
            
        if( ANALYSIS > 4 ){
            fAux = fopen(cForcePoints,"r");
            XF = doubleMatrix(nForcePoints,3,0);
            for(i = 0; i < nForcePoints; i++)
                fscanf(fAux,"%u %le %le %le",&nBuffer,&XF[i][0],&XF[i][1],&XF[i][2]);
            fclose(fAux);
        }

        /* Read internal points section (points where V and E are evaluated) */
        internalPtsTemp = 0;
        fscanf(fIn,"%32s %u",cSection,&nInternalPoints);
        if(nInternalPoints > 0){
            internalPtsTemp = nInternalPoints;
            fscanf(fIn,"%3s",cOutputType);
            for(i = 0; i < strlen(cOutputType); i++) cOutputType[i] = toupper(cOutputType[i]);
            nOut = 0;
            if( strncmp(cOutputType,"STD",3) == 0 ) nOut = 1;
            if( strncmp(cOutputType,"VTK",3) == 0 ) nOut = 2;
            if( nOut == 1 || nOut == 2 ){
                fscanf(fIn,"%32s",cInternalPointsFile);
                XinnerTemp = doubleMatrix(nInternalPoints,4,0);
            }
            else errorHandler("\n\nError: Unknown output type option.\n\n");
        }

        /* Read columns positions */
        fscanf(fIn,"%32s %u",cSection,&nCols);
        if( nCols > 0 ){
            fscanf(fIn,"%u",&nColType);
            xCols = doubleMatrix(nCols,3,0);
            for(i = 0; i < nCols; i++)
                fscanf(fIn,"%le %le %le",&xCols[i][0],&xCols[i][1],&xCols[i][2]);
        }
        fclose(fIn);

        /* Set element properties (nDim, nNodesInElem, nElemType) */
        nElemType = elemType();
        if( nElemType != 5 && nElemType != 6 )
            errorHandler("Error: Unknown element type option");

        /* Store MPI exchange data in buffer array */
        uintParam[ 0] = nNodes;
        uintParam[ 1] = nElems;
        uintParam[ 2] = nElemType;
        uintParam[ 3] = nNodesInElem;
        uintParam[ 4] = nMats;
        uintParam[ 5] = nInterfaces;
        uintParam[ 6] = ANALYSIS;
        uintParam[ 7] = nInternalPoints;
        uintParam[ 8] = nForcePoints;
        uintParam[ 9] = nOut;
        uintParam[10] = nCols;

        dbleParam[0] = vProbParam[0];
        dbleParam[1] = axis[0];
        dbleParam[2] = axis[1];
        dbleParam[3] = axis[2];
    }

    /* Broadcast values of parameters from the master to all other processors */
    MPI_Bcast(uintParam, 11, MPI_UNSIGNED, master, MPI_COMM_WORLD);
    MPI_Bcast(dbleParam,  4, MPI_DOUBLE,   master, MPI_COMM_WORLD);

    /* Store MPI exchange data in local variables */
    if( proc != master ){
        nNodes          = uintParam[ 0];
        nElems          = uintParam[ 1];
        nElemType       = uintParam[ 2];
        nNodesInElem    = uintParam[ 3];
        nMats           = uintParam[ 4];
        nInterfaces     = uintParam[ 5];
        ANALYSIS        = uintParam[ 6];
        nInternalPoints = uintParam[ 7];
        nForcePoints    = uintParam[ 8];
        nOut            = uintParam[ 9];
        nCols           = uintParam[10];

        vProbParam    = doubleVector(1,1);
        vProbParam[0] = dbleParam[0];
        axis[0]       = dbleParam[1];
        axis[1]       = dbleParam[2];
        axis[2]       = dbleParam[3];
    }

    /* Broadcast values of parameters from the master to all other processors */
    MPI_Bcast(&preCond, 1, MPI_UNSIGNED, master, MPI_COMM_WORLD);
    MPI_Bcast(&nInit,   1, MPI_UNSIGNED, master, MPI_COMM_WORLD);
    MPI_Bcast(&nForcePoints, 1, MPI_UNSIGNED, master, MPI_COMM_WORLD);

    if( nMats > 0 ){
        if( proc != master ) vMatParam = doubleMatrix(nMats,2,0);
        MPI_Bcast(vMatParam[0], nMats*2, MPI_DOUBLE, master, MPI_COMM_WORLD);
    }

    if( nInterfaces > 0 ){
        if( proc != master ) vInterfaces = uintMatrix(nInterfaces,2,0);
        MPI_Bcast(vInterfaces[0], nInterfaces*2, MPI_UNSIGNED, master, MPI_COMM_WORLD);
    }

    /* Broadcast column positions and radius if necessary */
    if( nCols > 0 ){
        MPI_Bcast(xCols[0], nCols*3, MPI_DOUBLE, master, MPI_COMM_WORLD);
    }

    /* Define the size of the problem */
    size = 2*nNodes;
    nDim = 3;

    /* Memory allocation for element connectivity, nodes cordinates and bcs */
    mNodes  = doubleMatrix(nNodes,nDim,0);
    mElems  = uintMatrix(nElems,nNodesInElem,0);
    mBC     = doubleMatrix(size,3,1);
    vBCType = uintVector(size,1);
    
    /* Coordinate translation array for force calculation using MST */
    global2XF = uintVector(nNodes,1);

    /****************************************************************/
    /*** READ (nodes, elements, bcs, post-processing) INPUT FILES ***/
    /****************************************************************/

    if( proc == master ){
        /* Clean and read node file */
        comFilter(cNodeFile);
        fAux = fopen(cNodeFile,"r");
        for(i = 0; i < nNodes; i++){
            fscanf(fAux,"%u",&nBuffer);         /* Discard index */
            for(j = 0; j < nDim; j++) {
                printf("------------------%s-----%d--%d--\n", cNodeFile, i, j);
                fscanf(fAux,"%le",&mNodes[i][j]);
            }
        }
        fclose(fAux);

        /* Clean and read element file */
        comFilter(cElemFile);
        fAux = fopen(cElemFile,"r");
        for(i = 0; i < nElems; i++){
            fscanf(fAux,"%u",&nBuffer);         /* Discard index */
            for(j = 0; j < nNodesInElem; j++) fscanf(fAux,"%u",&mElems[i][j]);
        }
        fclose(fAux);

        /* Clean and read boundary conditions file (bc stored as integer value) */
        comFilter(cBCFile);
        fAux = fopen(cBCFile,"r");
        for(j = 0; j < 2*nNodes; j++){
            fscanf(fAux,"%u",&nBuffer);     /* Discard index */
            fscanf(fAux,"%32s",cBuffer);
            if(strncmp(cBuffer,"C",1) == 0 || strncmp(cBuffer,"c",1) == 0){
                strncpy(cTmp,&cBuffer[1],strlen(cBuffer)-1);
                nBuffer = atoi(cTmp) + 5;
            }
            else nBuffer = atoi(cBuffer);
            vBCType[j] = nBuffer;

            /* Decide how many values to read depending on bc type */
            if((nBuffer > 2) && (nBuffer < 6))
                fscanf(fAux,"%le %le %le",&mBC[j][0],&mBC[j][1],&mBC[j][2]);
            else if(nBuffer == 0 || nBuffer == 6)
                fscanf(fAux,"%le %le",&mBC[j][0],&mBC[j][1]);
            else fscanf(fAux,"%le",&mBC[j][0]);
        }
        fclose(fAux);

        /* Clean and read internal points file */
        if(nInternalPoints > 0){
            comFilter(cInternalPointsFile);
            fAux = fopen(cInternalPointsFile,"r");
            if( strncmp(cOutputType,"VTK",3) == 0 ){
                fscanf(fAux,"%u %u %u",&NX,&NY,&NZ);
                fscanf(fAux,"%le %le %le",&deltaX,&deltaY,&deltaZ);
            }
            for(i = 0; i < nInternalPoints; i++){
                fscanf(fAux,"%u",&nBuffer);         /* Discard index */
                for(j = 0; j < nDim; j++) fscanf(fAux,"%le",&XinnerTemp[i][j]);
            }
            fclose(fAux);
        }

        /* Remove temporary files from current directory */
        //system("rm -f ./*.tmp");

        /* Produce warning for specially problematic inputs */
        if ( nOut == 2 && nCols > 0)
            warningHandler("The output options nCol > 0 and VTK may produce unexpected results");
    }

    /* MPI exchange of nodes */
    MPI_Bcast(mNodes[0], nNodes*3, MPI_DOUBLE, master, MPI_COMM_WORLD);

    /* MPI exchange of elements (connectivity) */
    MPI_Bcast(mElems[0], nElems*nNodesInElem, MPI_UNSIGNED, master, MPI_COMM_WORLD);

    /* MPI exchange of BCs */
    MPI_Bcast(mBC[0], size*3, MPI_DOUBLE, master, MPI_COMM_WORLD);
    MPI_Bcast(vBCType, size, MPI_UNSIGNED, master, MPI_COMM_WORLD);

    /* Record program status at this stage in file log */
    if( proc == master ){
        fprintf(file_log,"\nUsing %u nodes and %u %s ",nNodes,nElems,cElemType);
        fprintf(file_log,"elements to solve AC electrostatic problem");
        fprintf(file_log,"\nNumber of interfaces: %u\n",nInterfaces);
        fprintf(file_log,"Number of materials: %u\n",nMats);
        fprintf(file_log,"Number of processors: %d\n",nprocs);
        fclose(file_log);
        file_log = fopen("bem.log","a");
        t1 = (double)clock()/CLOCKS_PER_SEC;
    }

    /****************************************************************/
    /***     INPUT DATA OBTAINED, SOLUTION OF PROBLEM FOLLOWS     ***/
    /****************************************************************/

    complete = ( nNodes - 1 ) / nprocs;
    partial  = ( nNodes - 1 ) - complete*nprocs;

    if( (proc+1) <= partial ){
        nodeMin = proc*( complete + 1);
        nodeMax = ( proc + 1 )*( complete + 1 ) - 1;
    }
    else{
        nodeMin = proc*complete + partial;
        nodeMax = ( proc + 1 )*complete + partial - 1;
    }
    if( (proc+1)%nprocs == 0 ) nodeMax = nodeMax + 1;
    localNodes = nodeMax - nodeMin + 1;
    localSize  = 2*localNodes;

    MPI_Allgather(&localNodes, 1, MPI_INT,
                   procNodes,  1, MPI_INT, 
                   MPI_COMM_WORLD);
                   
    for(i = 0; i < nprocs; i++)
        procSize[i] = 2*procNodes[i];

    procFirst[0] = 0;
    for(i = 1; i < nprocs; i++)
        procFirst[i] = procFirst[i-1] + procSize[i-1];

    /* Memory allocation for coefficient matrix and right hand side vector */
    mA = doubleMatrix(localNodes,size,1);
    vB     = doubleVector(size,1);
    vBTemp = doubleVector(localSize,1);

    /* Assemble coefficient matrix */
    if( proc == master ){
        fprintf(file_log,"\ndepSolver(): performing matrix assembly ...");
        fclose(file_log);
        file_log = fopen("bem.log","a");
    }

    if(nElemType == 5){
        electricFormA_tria3(mNodes,mElems,vBCType,vInterfaces,
                            vMatParam,vProbParam,mA,mBC,vBTemp);
    }
    else{
        electricFormA_tria6(mNodes,mElems,vBCType,vInterfaces,
                            vMatParam,vProbParam,mA,mBC,vBTemp);
    }

    /* Make sure all CPUs have the complete B array */
    MPI_Allgatherv(vBTemp, localSize, MPI_DOUBLE, vB, procSize, 
                   procFirst, MPI_DOUBLE, MPI_COMM_WORLD);

    /* Solve linear system */
    if( proc == master ){
        t2 = (double)clock()/CLOCKS_PER_SEC - t1;
        fprintf(file_log,"\ndepSolver(): solving linear system with GMRES ...");
        fclose(file_log);
        file_log = fopen("bem.log","a");
    }

    solverGMRES_el(preCond,nInit,mA,vB,procSize,procFirst,procNodes);

    t3 = (float)clock()/CLOCKS_PER_SEC-t2-t1;


    /**************************************************************/
    /***      SOLUTION OBTAINED, POST-PROCESSING FOLLOWS        ***/
    /**************************************************************/

    /* Potential, electric field and force at required points */
    if( proc == master) {
        fprintf(file_log,"\ndepSolver(): performing postProcessing tasks ...");
        fclose(file_log);
        file_log = fopen("bem.log","a");
        if( nInternalPoints > 0 ){
            Xinner = doubleMatrix(nInternalPoints,3,0);
            postProcessSetup(cOutputType, xCols, XinnerTemp, Xinner);
            freeDoubleMatrix(XinnerTemp,internalPtsTemp);
        }
    }
    else { // added by zhenbang to avoid out of bound error happened in slave nodes
        if ( nInternalPoints > 0) {
            Xinner = doubleMatrix(nInternalPoints, 3, 0);
        }
    }//end
    
    /* Scatter internal points amongst processors */
    if( nInternalPoints != 0 ){
        complete = ( nInternalPoints - 1 ) / nprocs;
        partial  = ( nInternalPoints - 1 ) - complete*nprocs;

        if( (proc+1) <= partial ){
            pointMin = proc*( complete + 1);
            pointMax = ( proc + 1 )*( complete + 1 ) - 1;
        }
        else{
            pointMin = proc*complete + partial;
            pointMax = ( proc + 1 )*complete + partial - 1;
        }
        if( (proc+1)%nprocs == 0 ) pointMax = pointMax + 1;
        localPoints = pointMax - pointMin + 1;

        MPI_Allgather(&localPoints, 1, MPI_INT,
                       procPoints,  1, MPI_INT, 
                       MPI_COMM_WORLD);

        for(i = 0; i < nprocs; i++){
            procCoords[i] = 3*procPoints[i];
            potSize[i]    = 2*procPoints[i];
            fieldSize[i]  = 6*procPoints[i];
        }
        procFirstPoint[0]  = 0;
        potFirstPoint[0]   = 0;
        fieldFirstPoint[0] = 0;
        for(i = 1; i < nprocs; i++){
            procFirstPoint[i]  = procFirstPoint[i-1]  + procCoords[i-1];
            potFirstPoint[i]   = potFirstPoint[i-1]   + potSize[i-1];
            fieldFirstPoint[i] = fieldFirstPoint[i-1] + fieldSize[i-1];
        }

        localXinner = doubleMatrix(localPoints,3,0);
        MPI_Scatterv(Xinner[0], procCoords, procFirstPoint, MPI_DOUBLE,
                     localXinner[0], localPoints*3, MPI_DOUBLE, 
                     master, MPI_COMM_WORLD);
        
        localPot   = doubleMatrix(localPoints,2,0);
        localField = doubleMatrix(localPoints,6,0);
        
        globalPot   = doubleMatrix(nInternalPoints,2,0);
        globalField = doubleMatrix(nInternalPoints,6,0);
    }

    /* Maxwell's Stress Tensor calculation, we must find the relevant nodes   */
    /* and elements (BCType == 6) and distribute them amongst all processors. */
    if( ANALYSIS == 3 || ANALYSIS == 4 ){

        /* Find out how many points require a force calculation */
        for( i = 0; i < nNodes; i++ )
            if( vBCType[i] == 6 ) nForcePoints++;

        /* Allocate memory and fill up force points coordinates */
        XF = doubleMatrix(nForcePoints,3,0);
        current = 0;
        for( i = 0; i < nNodes; i++ ){
            if( vBCType[i] == 6 ){
                XF[current][0] = mNodes[i][0];
                XF[current][1] = mNodes[i][1];
                XF[current][2] = mNodes[i][2];
                global2XF[i]   = current;
                current++;
            }
        }

        /* Find out how many elements require a force calculation */
        nForceElems = 0;
        for( i = 0; i < nElems; i++ ){
            if( vBCType[mElems[i][0]-1] == 6 ) nForceElems++;
        }
        globalXe = doubleMatrix(nForceElems,nNodesInElem,1);

        /* Allocate memory and fill up force points coordinates */
        if( proc == master ){
            EF       = uintMatrix(nForceElems,nNodesInElem,0);
            current  = 0;
            for( i = 0; i < nElems; i++ ){
                if( vBCType[mElems[i][0]-1] == 6 ){
                    for(j = 0; j < nNodesInElem; j++) 
                        EF[current][j] = mElems[i][j];
                    current++;
                }
            }
        }  
    }

    /* Scatter force points amongst processors */
    if( nForcePoints != 0 ){   
    
        complete = ( nForcePoints - 1 ) / nprocs;
        partial  = ( nForcePoints - 1 ) - complete*nprocs;

        if( (proc+1) <= partial ){
            forceMin = proc*( complete + 1);
            forceMax = ( proc + 1 )*( complete + 1 ) - 1;
        }
        else{
            forceMin = proc*complete + partial;
            forceMax = ( proc + 1 )*complete + partial - 1;
        }
        if( (proc+1)%nprocs == 0 ) forceMax = forceMax + 1;
        localForcePoints = forceMax - forceMin + 1;

        MPI_Allgather(&localForcePoints, 1, MPI_INT, 
                      forcePoints,       1, MPI_INT, 
                      MPI_COMM_WORLD);
                       
        for(i = 0; i < nprocs; i++) forceSize[i] = 3*forcePoints[i];
        
        firstForcePoint[0] = 0;
        for(i = 1; i < nprocs; i++)
            firstForcePoint[i] = firstForcePoint[i-1] + forceSize[i-1];
    
        localXF = doubleMatrix(localForcePoints,3,0);
        MPI_Scatterv(XF[0], forceSize, firstForcePoint, MPI_DOUBLE,
                     localXF[0], localForcePoints*3, MPI_DOUBLE, 
                     master, MPI_COMM_WORLD);
                     
        /* Scatter elements if using Maxwell's Stress Tensor calculation */
        if( ANALYSIS == 3 || ANALYSIS == 4 ){
            complete = ( nForceElems - 1 ) / nprocs;
            partial  = ( nForceElems - 1 ) - complete*nprocs;

            if( (proc+1) <= partial ){
                forceElemMin = proc*( complete + 1);
                forceElemMax = ( proc + 1 )*( complete + 1 ) - 1;
            }
            else{
                forceElemMin = proc*complete + partial;
                forceElemMax = ( proc + 1 )*complete + partial - 1;
            }
            if( (proc+1)%nprocs == 0 ) forceElemMax = forceElemMax + 1;
            localForceElems = forceElemMax - forceElemMin + 1;

            MPI_Allgather(&localForceElems, 1, MPI_INT, 
                           forceElems,      1, MPI_INT, 
                           MPI_COMM_WORLD);

            for(i = 0; i < nprocs; i++) forceElemSize[i] = nNodesInElem*forceElems[i];
            firstForceElem[0] = 0;
            for(i = 1; i < nprocs; i++)
                firstForceElem[i] = firstForceElem[i-1] + forceElemSize[i-1];

            localEF = uintMatrix(localForceElems,nNodesInElem,0);
            localXe = doubleMatrix(localForceElems,nNodesInElem,1);
            MPI_Scatterv(EF[0], forceElemSize, firstForceElem, MPI_UNSIGNED,
                         localEF[0], localForceElems*nNodesInElem, MPI_UNSIGNED, 
                         master, MPI_COMM_WORLD);
                         
            localForce  = doubleMatrix(localForceElems,3,1);
            globalForce = doubleMatrix(nForceElems,3,1);
        }
        else{
            localForce  = doubleMatrix(localForcePoints,3,1);
            globalForce = doubleMatrix(nForcePoints,3,1);
        }
    }
    
    /* Perform distributed post-process calculations */
    if( nElemType == 5){
            postProcess_tria3(ANALYSIS, axis, localXF, localXe, localXinner,
                              mNodes, localEF, mElems, global2XF, firstForcePoint,
                              forceSize, vMatParam, vProbParam, vBCType, vB,
                              localPot, localField, localForce, localFcm,
                              localXcm);
    }
    else{
            postProcess_tria6(ANALYSIS, axis, localXF, localXe, localXinner, 
                              mNodes, localEF, mElems, global2XF, firstForcePoint,
                              forceSize, vMatParam, vProbParam, vBCType, vB,
                              localPot, localField, localForce, localFcm,
                              localXcm);
    }
    
    if( nInternalPoints > 0 ){ 
        /* Gather post-process data and dump to files from master */
        MPI_Gatherv( localPot[0], localPoints*2, MPI_DOUBLE,
                    globalPot[0], potSize, potFirstPoint,
                    MPI_DOUBLE, master, MPI_COMM_WORLD);


        MPI_Gatherv( localField[0], localPoints*6, MPI_DOUBLE,
                    globalField[0], fieldSize, fieldFirstPoint,
                    MPI_DOUBLE, master, MPI_COMM_WORLD);
    }

    /* Gather all distributed force data in global arrays */
    if( nForcePoints > 0 ){
    
        if( ANALYSIS == 3 || ANALYSIS == 4){
    
            /* Gather force at the center of the elements (Force) */
            MPI_Gatherv( localForce[0], localForceElems*3, MPI_DOUBLE,
                        globalForce[0], forceElemSize, firstForceElem, MPI_DOUBLE, 
                        master, MPI_COMM_WORLD);
                        
            /* Gather points at the center of the elements (Xe) */
            MPI_Gatherv( localXe[0], localForceElems*3, MPI_DOUBLE,
                        globalXe[0], forceElemSize, firstForceElem, MPI_DOUBLE, 
                        master, MPI_COMM_WORLD);
                
            /* Add up total force at the center of mass (Fcm) */            
            MPI_Reduce(localFcm, globalFcm, 3, MPI_DOUBLE, MPI_SUM, 
                       master, MPI_COMM_WORLD);
               
            /* Add up position of the center of mass (Xcm) */            
            MPI_Reduce(localXcm, globalXcm, 3, MPI_DOUBLE, MPI_SUM, 
                      master, MPI_COMM_WORLD);
                   
            globalXcm[0] = globalXcm[0]/nForcePoints;
            globalXcm[1] = globalXcm[1]/nForcePoints;
            globalXcm[2] = globalXcm[2]/nForcePoints;
        }
        else{
   
            MPI_Gatherv( localForce[0], localForcePoints*3, MPI_DOUBLE,
                        globalForce[0], forceSize, firstForcePoint, MPI_DOUBLE, 
                        master, MPI_COMM_WORLD);
        }
    }

    /* Save solution and output data to file */                
    if( proc == master ){
        saveData(ANALYSIS, cOutputType, Xinner, XF, globalXe, globalPot, 
                 globalField, globalForce, globalFcm, globalXcm);

        fAux = fopen("solution.dat","w");
        for(i = 0; i < nNodes; i++){
            fprintf(fAux,"%le\t%le\t%le\t",mNodes[i][0],mNodes[i][1],mNodes[i][2]);
            fprintf(fAux,"%le\t%le\n",vB[2*i],vB[2*i + 1]);
        }
        fclose(fAux);
    }
    t4 = (double)clock()/CLOCKS_PER_SEC - t3 - t2 - t1;


    /*******************************************************************/
    /***  POST-PROCESSING FINISHED, ANALYSIS OF PERFORMANCE FOLLOWS  ***/
    /*******************************************************************/
    
    if( proc == master ){
        fprintf(file_log,"\n\n*** depSolver(): performance analysis ***");
        if(t1 > 60){
            t1m = (int)(t1/60.0);
            t1s = t1-t1m*60.0;
            fprintf(file_log,"\nTIME READING INPUT: \t\t%u m %2.1f s",t1m,t1s);
        }
        else fprintf(file_log,"\nTIME READING INPUT: \t\t%2.1f seconds",t1);

        if(t2 > 60){
            t2m = (int)(t2/60.0);
            t2s = t2-t2m*60;
            fprintf(file_log,"\nTIME IN ASSEMBLY: \t\t%u m %2.1f s",t2m,t2s);
        }
        else fprintf(file_log,"\nTIME IN ASSEMBLY: \t\t%2.1f seconds",t2);

        if(t3 > 60){
            t3m = (int)(t3/60.0);
            t3s = t3-t3m*60.0;
            fprintf(file_log,"\nTIME IN SOLVER: \t\t%u m %2.1f s",t3m,t3s);
        }
        else  fprintf(file_log,"\nTIME IN SOLVER: \t\t%2.1f seconds",t3);

        if(t4 > 60){
            t4m = (int)(t4/60.0);
            t4s = t4-t4m*60.0;
            fprintf(file_log,"\nTIME IN POST-PROCESSING: \t%u m %2.1f s",t4m,t4s);
        }
        else fprintf(file_log,"\nTIME IN POST-PROCESSING: \t%2.1f seconds",t4);

        t_tot = t1+t2+t3+t4;
        if(t_tot > 60){
            t_totm = (int)(t_tot/60.0);
            t_tots = t_tot-t_totm*60.0;
            fprintf(file_log,"\nTOTAL EXECUTION TIME: \t%u m %2.1f s",t_totm,t_tots);
        }
        else fprintf(file_log,"\nTOTAL EXECUTION TIME: \t\t%2.1f seconds",t_tot);
        fclose(file_log);
        file_log = fopen("bem.log","a");
    }

    /************************************************************************/
    /***  PERFORMANCE ANALYSIS FINISHED, CLOSE ALL FILES AND FREE MEMORY  ***/
    /************************************************************************/

    free(vB);
    free(vBCType);
    free(vProbParam);
    freeUintMatrix(mElems,nElems);
    freeDoubleMatrix(vMatParam,nMats);
    freeDoubleMatrix(mBC,size);
    freeDoubleMatrix(mNodes,nNodes);
    freeDoubleMatrix(mA,localNodes);

    /* Free only if previously allocated */
    if( ANALYSIS > 4 ) freeDoubleMatrix(XF,nForcePoints);
    if( nInterfaces > 0 ) freeUintMatrix(vInterfaces,nInterfaces);
    if( nCols > 0 ) freeDoubleMatrix(xCols,nCols);


    /* Free MPI related buffers */
    free(procSize);
    free(procFirst);
    free(procNodes);
    free(procPoints);
    free(procFirstPoint);
    free(vBTemp);

    /* Program has finished */
    if(proc == master){
        fprintf(file_log,"\n\ndepSolver(): finished @ ");
        currentTime = time(NULL);
        localTime   = localtime(&currentTime);
        fputs(asctime(localTime),file_log);
        fprintf(file_log,"\n\n********************************************");
        fprintf(file_log,"************************************\n\n");
        fclose(file_log);
    }

    MPI_Finalize();

    return 0;
}

