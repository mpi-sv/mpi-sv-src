/******************************************************************************
* File      : fieldPost.c                                                     *
* Author    : Carlos Rosales Fernandez (carlos@ihpc.a-star.edu.sg)            *
* Date      : 01-09-2006                                                      *
* Revision  : 1.0                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
* Takes the file "field.dat" and the number of points where the field is      *
* calculated and produces the output X.dat, Y.dat and E.dat to be used with   *
* matlab. The file E.dat contains |E|. It also writes the interpolation       *
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
#include <math.h>

void errorHandler(char errorText[]);
double **doubleMatrix(unsigned int ROWS, unsigned int COLS, unsigned int INIT);
void freeDoubleMatrix(double **M, unsigned int ROWS);

int main(int argc, char *argv[])
{
FILE *fp,*fX,*fY,*fE,*fXI,*fYI;
char cBuffer[32];
unsigned int i, j, NX, NY, nxi, nyi, size, sumx, sumy, sumz;
double buffer, dx, dy, dxi, dyi, XI, YI, E2, Ex, Ey, Ez, ExIm, EyIm, EzIm;
double Xmax, Xmin, Ymax, Ymin;
double *X, *Y, **E;

/* MAKE SURE INPUT IS CORRECT */
if(argc == 2 && strcmp(argv[1],"-h") == 0){
    printf("\n\nSYNTAX\nfieldPost nx ny nxi nyi\n\n");
    printf("DESCRIPTION\n");
    printf("Takes the file 'field.dat' and the number of points where the ");
    printf("field is\ncalculated and produces the output X.dat, Y.dat and ");
    printf("E.dat to be used with\nmatlab. The file E.dat contains |E|. It ");
    printf("also writes the interpolation vectors XI and YI that contain a ");
    printf("number nInterp of points between the\nminimum and the maximum ");
    printf("values of X and Y.\nThe parser will eliminate the column with ");
    printf("repeated values from the output.\n\n");
    exit(0);
}
else if(argc != 5){
    printf("\n\nError: Incorrect syntax\n\nTry: fieldPost nx ny nxi nyi\n");
    errorHandler("or type 'fieldPost -h' for help.\n");
}
else{
    NX = atoi(argv[1]);
    NY = atoi(argv[2]);
    nxi = atoi(argv[3]);
    nyi = atoi(argv[4]);
}

/* ALLOCATE SPACE FOR E DATA */
size = NX*NY;
E = doubleMatrix(size,9,0);

/* OPEN INPUT DATA FILE, CLEAR FIRST LINE AND READ */
if((fp = fopen("field.dat","r")) == NULL)
    errorHandler("Error: Unable to open input file 'field.dat'.");
for(i =0; i < 9; i++) fscanf(fp,"%s",&cBuffer);
for(i = 0; i < size; i++){
    fscanf(fp,"%le %le %le %le ",&E[i][0],&E[i][1],&E[i][2],&E[i][3]);
    fscanf(fp,"%le %le %le %le %le",&E[i][4],&E[i][5],&E[i][6],&E[i][7],&E[i][8]);
}

/* FIND OUT CONSTANT COLUMN */
sumx = sumy = sumz = 1;
for(i = 1; i < size; i++){
    if(E[i][0] == E[i-1][0]) sumx++;
    if(E[i][1] == E[i-1][1]) sumy++;
    if(E[i][2] == E[i-1][2]) sumz++;
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
if((fE = fopen("E.dat","w")) == NULL)
    errorHandler("Error: Unable to open output file 'E.dat'");

/* CHOOSE Xmax/Xmin AND Ymax/Ymin */
if(sumx == size){
    Xmax = E[0][1]; Xmin = E[0][1];
    Ymax = E[0][2]; Ymin = E[0][2];
    for(i = 0; i < size; i++){
        if(E[i][1] > Xmax) Xmax = E[i][1];
        else if(E[i][1] < Xmin) Xmin = E[i][1];
        if(E[i][2] > Ymax) Ymax = E[i][2];
        else if(E[i][2] < Ymin) Ymin = E[i][2];
    }
}
else if(sumy == size){
    Xmax = E[0][0]; Xmin = E[0][0];
    Ymax = E[0][2]; Ymin = E[0][2];
    for(i = 0; i < size; i++){
        if(E[i][0] > Xmax) Xmax = E[i][0];
        else if(E[i][0] < Xmin) Xmin = E[i][0];
        if(E[i][2] > Ymax) Ymax = E[i][2];
        else if(E[i][2] < Ymin) Ymin = E[i][2];
    }
}
else if(sumz == size){
    Xmax = E[0][0]; Xmin = E[0][0];
    Ymax = E[0][1]; Ymin = E[0][1];
    for(i = 0; i < size; i++){
        if(E[i][0] > Xmax) Xmax = E[i][0];
        else if(E[i][0] < Xmin) Xmin = E[i][0];
        if(E[i][1] > Ymax) Ymax = E[i][1];
        else if(E[i][1] < Ymin) Ymin = E[i][1];
    }
}
else errorHandler("Error: No constant plane in input file.");

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
        Ex = E[i*NY + j][3];
        Ey = E[i*NY + j][5];
        Ez = E[i*NY + j][7];
        ExIm = E[i*NY + j][4];
        EyIm = E[i*NY + j][6];
        EzIm = E[i*NY + j][8];
        E2 = sqrt(Ex*Ex + Ey*Ey + Ez*Ez + ExIm*ExIm + EyIm*EyIm + EzIm*EzIm);
        fprintf(fE,"%le ",E2);
    }
    fprintf(fE,"\n");
}

/* CLOSE ALL FILES AND FREE MEMORY */
fclose(fp);
fclose(fX);
fclose(fXI);
fclose(fY);
fclose(fYI);
fclose(fE);
freeDoubleMatrix(E,size);

return 1;
}
