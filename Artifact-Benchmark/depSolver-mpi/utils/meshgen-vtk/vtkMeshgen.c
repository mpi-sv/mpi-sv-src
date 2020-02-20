/******************************************************************************
* File      : meshgen.c                                                       *
* Author    : Carlos Rosales Fernandez (carlos@ihpc.a-star.edu.sg)            *
* Date      : 01-09-2006                                                      *
* Revision  : 1.0                                                             *
*******************************************************************************
* Generates a regular 3d cartesian grid with limits [(xmin,xmax),(ymin,ymax), *
* (zmin,zmax)] and saves the coordinates to the file "mesh.dat". The first    *
* two lines of the file contain the total number of points and the interval   *
* between two consecutive points for each of the three spatial directions.    *
*                                                                             *
* Example:    meshgen-vtk 10 10 20 -50 50 -10 10 -5 5                         *
* Produces a mesh in the volume [(-50,50):(-10,10):(-5,5)] with a total of    *
* 2000 points (10x10x20). Line numbers are always saved to the ouput file.    *
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
#include <ctype.h>

void errorHandler(char errorText[]);

int main(int argc, char *argv[])
{
    FILE *fout;
    int  i, j, k, nx, ny, nz, count;
    double dx, dy, dz, r, x, xmin, xmax, y, ymin, ymax, z, zmin, zmax;

    /* Check for input error */
    if(argc == 2 && strcmp(argv[1],"-h") == 0){
        printf("\n\nSYNTAX:\nmeshgen-vtk nx ny nz xmin xmax ymin ymax ");
        printf("zmin zmax\n\n");
        printf("DESCRIPTION\n");
        printf("Generates a regular 3d cartesian grid with limits ");
        printf("[(xmin,xmax),(ymin,ymax),\n(zmin,zmax)] and saves the ");
        printf("coordinates to the file 'mesh.dat'. The first\ntwo lines ");
        printf("of the file contain the total number of points and the ");
        printf("interval\nbetween two consecutive points for each of the ");
        printf("three spatial directions.\nLine numbers are always saved ");
        printf("to the output file.\n\n");
        printf("EXAMPLE\n");
        printf("meshgen-vtk 10 10 20 -50 50 -10 10 -5 5\n\n");
        printf("Produces a mesh in the volume [(-50,50):(-10,10):(-5,5)] ");
        printf("with a total\nof 2000 points (10x10x20).\n\n");
        exit(0);
    }
    else if(argc != 10){
        printf("\n\nError: Incorrect syntax.\n\n");
        printf("Try: meshgen-vtk nx ny nz xmin xmax ymin ymax zmin zmax\n");
        errorHandler("or type 'meshgen-vtk -h' for help.\n");
    }
    else{
        nx = atoi(argv[1]);
        ny = atoi(argv[2]);
        nz = atoi(argv[3]);
        xmin = atof(argv[4]);
        xmax = atof(argv[5]);
        ymin = atof(argv[6]);
        ymax = atof(argv[7]);
        zmin = atof(argv[8]);
        zmax = atof(argv[9]);
    }
    if( (fout = fopen("mesh.bem","w")) == NULL )
        errorHandler("Error: Can't create output file mesh.dat");

    /* Initialize */
    if( nx == 1 ) dx = 0.0;
    else dx = (xmax - xmin)/( (double)nx - 1.0 );
    if( ny == 1 ) dy = 0.0;
    else dy = (ymax - ymin)/( (double)ny - 1.0 );
    if( nz == 1 ) dz = 0.0;
    else dz = (zmax - zmin)/( (double)nz - 1.0 );

    fprintf(fout,"%d\t%d\t%d\n",nx,ny,nz);
    fprintf(fout,"%le\t%le\t%le\n",dx,dy,dz);
    count = 1;
    for(k = 0; k < nz; k++){
        z = zmin + k*dz;
        for(j = 0; j < ny; j++){
            y = ymin + j*dy;
            for(i = 0; i < nx; i++){
                x = xmin + i*dx;
                fprintf(fout,"%d\t%le\t%le\t%le\n",count,x,y,z);
                count++;
            }
        }
     }
    fclose(fout);

    return 0;
}

 
