/******************************************************************************
* File      : stdMeshgen.c                                                       *
* Author    : Carlos Rosales Fernandez (carlos@ihpc.a-star.edu.sg)            *
* Date      : 01-09-2006                                                      *
* Revision  : 1.0                                                             *
*******************************************************************************
* Generates a regular 2d mesh with limits [(xmin,xmax),(ymin,ymax)] in the    *
* plane constantPlane = r, where constantPlane takes the values x, y or z.    *
* Line numbers are always saved to the output file, called "mesh.dat".        *
*                                                                             *
* It can be called sucesively in order to produce a single file with all the  *
* necessary points, as long as "firstLine" contains the correct number of the *
* first element of the plane (1 in the first call, 442 in the second if       *
* nx = ny = 21, et...).                                                             *
*                                                                             *
* Example:    mesh 1 z 100 20 -50 50 -10 10 0.0                               *
* Produces a mesh in the plane z = 0.0 in the range x = (-50,50),             *
* y = (-10,10), with 100 points in the x direction and 20 in the y            *
* direction. The line number is saved to the ouput file starting with 1.      *
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
    char constantPlane[2];
    int a, count, i, j, nx, ny, nz, n;
    double dx, dy, r, x, xmin, xmax, y, ymin, ymax, z;

    /* Check for input error */
    if(argc == 2 && strcmp(argv[1],"-h") == 0){
        printf("\n\nSYNTAX\nmeshgen-std firstLine constantPlane ");
        printf("nx ny xmin xmax ymin ymax r\n");
        printf("\nDESCRIPTION\n");
        printf("Generates a regular 2d mesh with limits [(xmin,xmax),");
        printf("(ymin,ymax)] in the\nplane constantPlane = r, where ");
        printf("constantPlane takes the values x, y or z.\nLine numbers are ");
        printf("always saved to the output file, called 'mesh.dat'.\n\n");
        printf("meshgen-std can be called succesively in order to ");
        printf("produce a single file\nwith all the necessary points, ");
        printf("as long as 'firstLine' contains the correct\nnumber of the first ");
        printf("element of the plane (1 in the first call, 442 in the\nsecond ");
        printf("if n = 21, et...).\n\n");
        printf("EXAMPLE\n");
        printf("meshgen-std 1 z 100 20 -50 50 -10 10 0.0\n\n");
        printf("Produces a mesh in the plane z = 0.0 in the range x = (-50,50),\n");
        printf("y = (-10,10), with 100 points in the x direction and 20 in the y\n");
        printf("direction.\n\n");
        exit(0);
    }
    else if(argc != 10){
        printf("\n\nError: Incorrect syntaxis.\n\nTry: meshgen-std ");
        printf("firstLine constantPlane nx ny xmin xmax ymin ymax r\n");
        errorHandler("or type 'meshgen-std -h' for help.\n");
    }
    else{
        a = atoi(argv[1]);
        strcpy(constantPlane,argv[2]);
        nx = atoi(argv[3]);
        ny = atoi(argv[4]);
        xmin = atof(argv[5]);
        xmax= atof(argv[6]);
        ymin = atof(argv[7]);
        ymax= atof(argv[8]);
        r = atof(argv[9]);
    }
    if((fout = fopen("mesh.dat","a")) == NULL)
        errorHandler("Error: Can't open output file mesh.dat");

    /* Initialize */
    constantPlane[0] = toupper(constantPlane[0]);
    dx = (xmax - xmin)/(nx - 1);
    dy = (ymax - ymin)/(ny - 1);

    count = a;
    for(i = 0; i < nx; i++){
        x = xmin + i*dx;
        for(j = 0; j < ny; j++){
            y = ymin + j*dy;
            if(strcmp(constantPlane,"X") == 0)
                fprintf(fout,"%d %le %le %le\n",count,r,x,y);
            else if(strcmp(constantPlane,"Y") == 0)
                fprintf(fout,"%d    %le %le %le\n",count,x,r,y);
            else
                fprintf(fout,"%d   %le %le %le\n",count,x,y,r);
            count++;
        }
    }
    fclose(fout);

    return 0;
}

 
