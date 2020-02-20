/******************************************************************************
* File      : potPost.c                                                       *
* Author    : Carlos Rosales Fernandez (carlos@ihpc.a-star.edu.sg)            *
* Date      : 01-09-2006                                                      *
* Revision  : 1.0                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
* Takes the file "potential.dat" and the number of points where the field is  *
* calculated and produces the output X.dat, Y.dat and V.dat to be used with   *
* matlab. The file V.dat contains Re[V]. It also writes the interpolation     *
* vectors XI and YI that contain a number nInterp of points between the       *
* minimum and the maximum values of X and Y.                                  *
* The parser will eliminate the column with repeated values from the output.  *
******************************************************************************/

/*******************************************************************************
* Copyright 2006, 2008 Carlos Rosales Fernandez and IHPC (A*STAR).             *
*                                                                              *
* This file is part of depSolver.                                              *
*                                                                              *
* depSolver is free software: you can redistribute it and/or modify it under   *
* the terms of the GNU GPL version 3 or (at your option) any later version.    *
*
* depSolver is distributed in the hope that it will be useful, but WITHOUT ANY *
* WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS    *
* FOR A PARTICULAR PURPOSE. See the GNU General Public License for details.    *
*                                                                              *
* You should have received a copy of the GNU General Public License along with *
* depSolver in the file COPYING.txt. If not, see <http://www.gnu.org/licenses/>*
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void errorHandler(char errorText[]);
double **doubleMatrix(unsigned int ROWS, unsigned int COLS, unsigned int INIT);
void freeDoubleMatrix(double **M, unsigned int ROWS);

int main(int argc, char *argv[])
{
    FILE *fp,*fX,*fY,*fV, *fXI,*fYI;
    char cBuffer[32];
    unsigned int i, j, NX, NY, nxi, nyi, size, sumx, sumy, sumz;
    double dx, dy, dxi, dyi, XI, YI, VR, VI, Xmax, Xmin, Ymax, Ymin;
    double *X, *Y, **V;

    /* MAKE SURE INPUT IS CORRECT */
    if(argc == 2 && strcmp(argv[1],"-h") == 0){
        printf("\n\nSYNTAX\npotPost nx ny nxi nyi\n\n");
        printf("DESCRIPTION\n");
        printf("Takes the file 'potential.dat' and the number of points where");
        printf(" the field is\ncalculated and produces the output X.dat, ");
        printf("Y.dat and V.dat to be used with\nmatlab. The file V.dat ");
        printf("contains Re[V]. It also writes the interpolation\nvectors XI ");
        printf("and YI that contain a number nInterp of points between the\n");
        printf("minimum and the maximum values of X and Y.\nThe parser will e");
        printf("liminate the column with repeated values from the output.\n\n");
        exit(0);
    }
    else if(argc != 5){
        printf("\n\nError: Incorrect syntax.\n\nTry: potPost nx ny nxi nyi\n");
        errorHandler("or type 'potPost -h' for help.\n");
    }
    else{
        NX = atoi(argv[1]);
        NY = atoi(argv[2]);
        nxi = atoi(argv[3]);
        nyi = atoi(argv[4]);
    }
    
    /* ALLOCATE SPACE FOR E DATA */
    size = NX*NY;
    V = doubleMatrix(size,5,0);


    /* OPEN INPUT DATA FILE, CLEAR FIRST LINE AND READ */
    if((fp = fopen("potential.dat","r")) == NULL)
        errorHandler("Error: Unable to open input file 'potential.dat'");
    for(i =0; i < 5; i++) fscanf(fp,"%s",&cBuffer);
    for(i = 0; i < size; i++){
        fscanf(fp,"%le %le %le ",&V[i][0],&V[i][1],&V[i][2]);
        fscanf(fp,"%le %le",&V[i][3],&V[i][4]);
    }
    
    /* FIND OUT CONSTANT COLUMN */
    sumx = sumy = sumz = 1;
    for(i = 1; i < size; i++){
        if(V[i][0] == V[i-1][0]) sumx++;
        if(V[i][1] == V[i-1][1]) sumy++;
        if(V[i][2] == V[i-1][2]) sumz++;
    }
    
    /* OPEN OUTPUT FILES */
    if((fX = fopen("X.dat","w")) == NULL)
        errorHandler("Error: Unable to open output file 'X.dat'");
    if((fY = fopen("Y.dat","w")) == NULL)
        errorHandler("Error: Unable to open output file 'Y.dat'");
    if((fXI = fopen("XI.dat","w")) == NULL)
        errorHandler("Error: Unable to open output file 'XI.dat'");
    if((fYI = fopen("YI.dat","w")) == NULL)
        errorHandler("Error: Unable to open output file 'YI.dat'");
    if((fV = fopen("V.dat","w")) == NULL)
        errorHandler("Error: Unable to open output file 'V.dat'");

    /* CHOOSE Xmax/Xmin AND Ymax/Ymin */
    if(sumx == size){
        Xmax = V[0][1]; Xmin = V[0][1];
        Ymax = V[0][2]; Ymin = V[0][2];
        for(i = 0; i < size; i++){
            if(V[i][1] > Xmax) Xmax = V[i][1];
            else if(V[i][1] < Xmin) Xmin = V[i][1];
            if(V[i][2] > Ymax) Ymax = V[i][2];
            else if(V[i][2] < Ymin) Ymin = V[i][2];
        }
    }
    else if(sumy == size){
        Xmax = V[0][0]; Xmin = V[0][0];
        Ymax = V[0][2]; Ymin = V[0][2];
        for(i = 0; i < size; i++){
            if(V[i][0] > Xmax) Xmax = V[i][0];
            else if(V[i][0] < Xmin) Xmin = V[i][0];
            if(V[i][2] > Ymax) Ymax = V[i][2];
            else if(V[i][2] < Ymin) Ymin = V[i][2];
        }
    }
    else if(sumz == size){
        Xmax = V[0][0]; Xmin = V[0][0];
        Ymax = V[0][1]; Ymin = V[0][1];
        for(i = 0; i < size; i++){
            if(V[i][0] > Xmax) Xmax = V[i][0];
            else if(V[i][0] < Xmin) Xmin = V[i][0];
            if(V[i][1] > Ymax) Ymax = V[i][1];
            else if(V[i][1] < Ymin) Ymin = V[i][1];
        }
    }
    else errorHandler("Error: No constant plane in input file!!");
    
dx = (Xmax - Xmin)/(NX - 1);
dy = (Ymax - Ymin)/(NY - 1);
for(i = 0; i < NX; i++) fprintf(fX,"%le\n",Xmin+i*dx);
for(i = 0; i < NY; i++) fprintf(fY,"%le\n",Ymin+i*dy);

dxi = (Xmax-Xmin)/(nxi - 1);
dyi = (Ymax-Ymin)/(nyi - 1);
for(i = 0; i < nxi; i++) fprintf(fXI,"%le\n",Xmin+i*dxi);
for(i = 0; i < nyi; i++) fprintf(fYI,"%le\n",Ymin+i*dyi);   

for(i = 0; i < NX; i++){
    for(j = 0; j < NY; j++){
        fprintf(fV,"%le ",V[i*NY + j][3]);
    }
    fprintf(fV,"\n");
}

/* CLOSE ALL FILES AND FREE MEMORY */
fclose(fp);
fclose(fX);
fclose(fXI);
fclose(fY);
fclose(fYI);
fclose(fV);
freeDoubleMatrix(V,size);

return 0;
}
