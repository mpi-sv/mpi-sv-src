/******************************************************************************
* File     : preProcess.c                                                     *
* Author   : Carlos Rosales Fernandez (carlos.rosales.fernandez(at)gmail.com) *
* Revision : 1.0 (2008-12-23)                                                 *
******************************************************************************/
/**
 * @brief Input setup functions
 *
 * @file
 * This file contains the definition of two functions required by depSolver
 * during the initial setup of the problem: comFilter and elemType.
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
#include "preProcess.h"

#define LINELENGTH 320

extern char cElemType[];
extern unsigned int nDim, nNodes, nNodesAux, nElems, nNodesInElem; 

/**
 * Receives the name of the input file and filters out all C++ style comments,
 * then creates uncommented file filename.tmp. Input file is not changed.
 * The line length is hard-coded to 320.
 */
void comFilter(char *FileName)
{
    FILE *fIn, *fOut;
    char cTest[LINELENGTH];
    int  i;

    if((fIn = fopen(FileName,"r")) != NULL){   
        if((fOut = fopen(strcat(FileName,".tmp"),"w")) != NULL){
            while(fgets(cTest,LINELENGTH,fIn) != NULL){
                for(i = 0; i < strlen(cTest); i++)
                    if((cTest[i] == '/') && cTest[i+1] == '/'){
                        if(i ==0) cTest[i]='\0';
                        else{
                            cTest[i]='\n';
                            cTest[i+1]='\0';
                        }
                    break;
                    }
                if(strlen(cTest) > 0) fprintf(fOut,"%s",cTest);
            }
        }
        else{
            printf("ERROR: Can't create output file %s.\n", FileName);
            exit(1);
        }
    }
    else{
        printf("ERROR: Can't open input file %s.\n", FileName);
        exit(1);
    }

    fclose(fIn);
    fclose(fOut);
}

/**
 * Returns an index between 1 and 9 indicating the element type. If 0 is
 * returned the element type is invalid. It also provides the number of nodes
 * in each element, \a nNodesInElem, and the dimension of the problem, \a nDim.
 * Recognised element types are line1, line2, line3, tria1, tria3, tria6,
 * quad1, quad4 and quad8.
 */
int elemType()
{
    /* Transform to lowercase for comparison */
    cElemType[0] = tolower(cElemType[0]);
    cElemType[1] = tolower(cElemType[1]);
    cElemType[2] = tolower(cElemType[2]);
    cElemType[3] = tolower(cElemType[3]);
    cElemType[4] = tolower(cElemType[4]);
    cElemType[5] = tolower(cElemType[5]);

    /* Find element characteristic from input */
    if(strcmp(cElemType,"line1") == 0){
        nDim = 2;
        nNodesInElem = 1;
        return 1;
    }
    else if(strcmp(cElemType,"line2") == 0){
        nDim = 2;
        nNodesInElem = 2;
        return 2;
    }
    else if(strcmp(cElemType,"line3") == 0){
        nDim = 2;
        nNodesInElem = 3;
        return 3;
    }
    else if(strcmp(cElemType,"tria1") == 0){
        nDim = 3;
        /* Using 3 nodes for the virtual element containing the node */
        nNodesInElem = 3;
        return 4;
    }
    else if(strcmp(cElemType,"tria3") == 0){
        nDim = 3;
        nNodesInElem = 3;
        return 5;
    }
    else if(strcmp(cElemType,"tria6") == 0){
        nDim = 3;
        nNodesInElem = 6;
        return 6;
    }
    else if(strcmp(cElemType,"quad1") == 0){
        nDim = 3;
        nNodesInElem = 1;
        return 7;
    }
    else if(strcmp(cElemType,"quad4") == 0){
        nDim = 3;
        nNodesInElem = 4;
        return 8;
    }
    else if(strcmp(cElemType,"quad8") == 0){
        nDim = 3;
        nNodesInElem = 8;
        return 9;
    }

    /* This indicates an invalid element type */
    else return 0;
}

