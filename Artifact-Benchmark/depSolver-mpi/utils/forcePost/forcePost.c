/******************************************************************************
* File      : forcePost.c                                                     *
* Author    : Carlos Rosales Fernandez (carlos@ihpc.a-star.edu.sg)            *
* Date      : 01-09-2006                                                      *
* Revision  : 1.0                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
* Takes the file "force.dat" and the number of points where the force is      *
* calculated and produces the output X.dat, Y.dat, FX.dat, FY.dat and         *
* FZ.dat to be used with matlab. The files Fi.dat contain the DEP force in    *
* the ith direction. This is for use with quiver in matlab.                   *
* Also produces XX.dat, YY.dat, FXX.dat and XXI.dat, YYI.dat to be used with  *
* contourf and surf in matlab. The files XXI.dat and YYI.dat contain a number *
* nxi and nyi of points between the minimum and the maximum values of X and Y.*
* step is used to control how many of the points are included in the files for*
* the arrow data. Using step = 1 gives all data, using step = 2 gives 1 point *
* of every 4 (divides in both x and y).                                       *
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
#include <math.h>

void errorHandler(char errorText[]);
double **doubleMatrix(unsigned int ROWS, unsigned int COLS, unsigned int INIT);
void freeDoubleMatrix(double **M, unsigned int ROWS);

int main(int argc, char *argv[])
{
FILE *fp,*fX, *fXX, *fY, *fYY, *fFX, *fFY, *fFZ, *fFXX, *fFYY, *fXXI, *fYYI;
char cBuffer[32];
unsigned int i, j, NX, NY, nxi, nyi, size, step, sumx, sumy, sumz;
double buffer, dx, dy, dxi, dyi, XI, YI, Fx, Fy, Fz, Xmax, Xmin, Ymax, Ymin;
double *X, *Y, **F;

/* MAKE SURE INPUT IS CORRECT */
if(argc == 2 && strcmp(argv[1],"-h") == 0){
    printf("\n\nSYNTAX\nforcePost nx ny nxi nyi\n\n");
    printf("DESCRIPTION\n");
    printf("Takes the file 'force.dat' and the number of points where the ");
    printf("force is\ncalculated and produces the output X.dat, Y.dat, FX.dat");
    printf(", FY.dat and\nFZ.dat to be used with matlab. The files Fi.dat ");
    printf("contain the DEP force in\nthe ith direction. This is for use with");
    printf(" quiver in matlab.\n\n");
    printf("Also produces XX.dat, YY.dat, FXX.dat and XXI.dat, YYI.dat to be ");
    printf("used with\ncontourf and surf in matlab. The files XXI.dat and ");
    printf("YYI.dat contain a number\nnxi and nyi of points between the ");
    printf("minimum and the maximum values of X and Y.\n\n");
    printf("step is used to control how many of the points are included in ");
    printf("the files for\nthe arrow data. Using step = 1 gives all data, ");
    printf("using step = 2 gives 1 point\nof every 4 (divides in both x and y)\n\n");
    exit(0);
}
else if(argc != 6){
    printf("\n\nError: Incorrect syntax.\n\nTry: forcePost nx ny nxi nyi\n");
    errorHandler("or type 'forcePost -h' for help");
}
else{
    NX = atoi(argv[1]);
    NY = atoi(argv[2]);
    nxi = atoi(argv[3]);
    nyi = atoi(argv[4]);
    step = atoi(argv[5]);
}

/* ALLOCATE SPACE FOR E DATA */
size = NX*NY;
F = doubleMatrix(size,6,0);

/* OPEN INPUT DATA FILE, CLEAR FIRST LINE AND READ */
if((fp = fopen("force.dat","r")) == NULL)
    errorHandler("Error: Unable to open input file 'force.dat'");
for(i =0; i < 6; i++) fscanf(fp,"%s",&cBuffer);
for(i = 0; i < size; i++) fscanf(fp,"%le    %le %le %le %le %le",&F[i][0],&F[i][1],&F[i][2],&F[i][3],&F[i][4],&F[i][5]);

/* FIND OUT CONSTANT COLUMN */
sumx = sumy = sumz = 1;
for(i = 1; i < size; i++){
    if(F[i][0] == F[i-1][0]) sumx++;
    if(F[i][1] == F[i-1][1]) sumy++;
    if(F[i][2] == F[i-1][2]) sumz++;
}

/* OPEN OUTPUT FILES */
if((fX = fopen("X.dat","w")) == NULL)
    errorHandler("Error: Unable to open output file 'X.dat'");
if((fY = fopen("Y.dat","w")) == NULL)
    errorHandler("Error: Unable to open output file 'Y.dat'");
if((fXX = fopen("XX.dat","w")) == NULL)
    errorHandler("Error: Unable to open output file 'XX.dat'");
if((fYY = fopen("YY.dat","w")) == NULL)
    errorHandler("Error: Unable to open output file 'YY.dat'");
if((fXXI = fopen("XXI.dat","w")) == NULL)
    errorHandler("Error: Unable to open output file 'XXI.dat'");
if((fYYI = fopen("YYI.dat","w")) == NULL)
    errorHandler("Error: Unable to open output file 'YYI.dat'");
if((fFX = fopen("FX.dat","w")) == NULL)
    errorHandler("Error: Unable to open output file 'FX.dat'");
if((fFY = fopen("FY.dat","w")) == NULL)
    errorHandler("Error: Unable to open output file 'FY.dat'");
if((fFZ = fopen("FZ.dat","w")) == NULL)
    errorHandler("Error: Unable to open output file 'FZ.dat'");
if((fFXX = fopen("FXX.dat","w")) == NULL)
    errorHandler("Error: Unable to open output file 'FXX.dat'");
if((fFYY = fopen("FYY.dat","w")) == NULL)
    errorHandler("Error: Unable to open output file 'FYY.dat'");

/* CHOOSE Xmax/Xmin AND Ymax/Ymin */
if(sumx == size){
    Xmax = F[0][1]; Xmin = F[0][1];
    Ymax = F[0][2]; Ymin = F[0][2];
    for(i = 0; i < size; i++){
        if(F[i][1] > Xmax) Xmax = F[i][1];
        else if(F[i][1] < Xmin) Xmin = F[i][1];
        if(F[i][2] > Ymax) Ymax = F[i][2];
        else if(F[i][2] < Ymin) Ymin = F[i][2];
    }
}
else if(sumy == size){
    Xmax = F[0][0]; Xmin = F[0][0];
    Ymax = F[0][2]; Ymin = F[0][2];
    for(i = 0; i < size; i++){
        if(F[i][0] > Xmax) Xmax = F[i][0];
        else if(F[i][0] < Xmin) Xmin = F[i][0];
        if(F[i][2] > Ymax) Ymax = F[i][2];
        else if(F[i][2] < Ymin) Ymin = F[i][2];
    }
}
else if(sumz == size){
    Xmax = F[0][0]; Xmin = F[0][0];
    Ymax = F[0][1]; Ymin = F[0][1];
    for(i = 0; i < size; i++){
        if(F[i][0] > Xmax) Xmax = F[i][0];
        else if(F[i][0] < Xmin) Xmin = F[i][0];
        if(F[i][1] > Ymax) Ymax = F[i][1];
        else if(F[i][1] < Ymin) Ymin = F[i][1];
    }
}
else errorHandler("Error: No constant plane in input file.");

dx = (Xmax - Xmin)/(NX - 1);
dy = (Ymax - Ymin)/(NY - 1);
for(i = 0; i < NX; i++) fprintf(fXX,"%le\n",Xmin+i*dx);
for(i = 0; i < NY; i++) fprintf(fYY,"%le\n",Ymin+i*dy);

dxi = (Xmax-Xmin)/(nxi - 1);
dyi = (Ymax-Ymin)/(nyi - 1);
for(i = 0; i < nxi; i++) fprintf(fXXI,"%le\n",Xmin+i*dxi);
for(i = 0; i < nyi; i++) fprintf(fYYI,"%le\n",Ymin+i*dyi);  

for(i = 0; i < NX; i = i+step){
    for(j = 0; j < NY; j = j+step){
        fprintf(fX,"%le\n",F[i*NY + j][0]);
        fprintf(fY,"%le\n",F[i*NY + j][1]);
        fprintf(fFX,"%le\n",F[i*NY + j][3]);
        fprintf(fFY,"%le\n",F[i*NY + j][4]);
        fprintf(fFZ,"%le\n",F[i*NY + j][5]);
    }
}

for(i = 0; i < NX; i++){
    for(j = 0; j < NY; j++){
        fprintf(fFXX,"%le   ",F[i*NY + j][3]);
        fprintf(fFYY,"%le   ",F[i*NY + j][4]);
    }
    fprintf(fFXX,"\n");
    fprintf(fFYY,"\n");
}


/* CLOSE ALL FILES AND FREE MEMORY */
fclose(fp);
fclose(fX);
fclose(fXX);
fclose(fXXI);
fclose(fY);
fclose(fYY);
fclose(fYYI);
fclose(fFX);
fclose(fFY);
fclose(fFZ);
fclose(fFXX);
fclose(fFYY);
freeDoubleMatrix(F,size);

return 0;
}
