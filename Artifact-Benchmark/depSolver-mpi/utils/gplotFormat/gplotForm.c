/******************************************************************************
* File      : gplotForm.c                                                     *
* Author    : Carlos Rosales Fernandez (carlos@ihpc.a-star.edu.sg)            *
* Date      : 01-09-2006                                                      *
* Revision  : 1.0                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
* Inserts a blank row every nrows for all ncols in filename for all N rows in *
* the file. Input file is not altered in any way. Output file is by default   *
* called "temp.gnu".                                                          *              
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
    FILE *fIn, *fOut, *fTmp;
    char cBuffer[32];
    unsigned int count, i, j, k, N, nrows, ncols;
    double data;

    if(argc == 2 && strcmp(argv[1],"-h") == 0){
        printf("\n\nSYNTAX\ngplotForm filename N nrows ncols.\n");
        printf("\nDESCRIPTION\n");
        printf("Inserts a blank row every nrows for all ncols in filename");
        printf(" for all N rows in\nthe file. Input file is not altered in ");
        printf("any way. Output file is by default\ncalled 'temp.gnu'.\n\n");
        printf("EXAMPLE\n");
        printf("gplotForm potential.dat 2400 20 5\n\n");
        printf("Prepares a potential data file with a total of 2400 lines for");
        printf(" gnuplot by\ninserting a blank space every 20 rows.\n\n");
        exit(0);
    }
    else if(argc != 5){
        printf("\n\nError: Incorrect syntax.\n\nTry: gplotForm filename N nrows ncols.\n");
        errorHandler("or type 'gplotForm -h' for help.\n");
    }

    /* Initialize */
    fIn = fopen(argv[1],"r");
    N = atoi(argv[2]);
    nrows = atoi(argv[3]);
    ncols = atoi(argv[4]);
    for(i = 0; i < ncols; i++) fscanf(fIn,"%s",cBuffer);

	fOut = fopen("temp.gnu","w");
    count = 0;
	for(j = 0; j < N; j++){
		for(k = 0; k < ncols; k++){
    		fscanf(fIn,"%le",&data);
			fprintf(fOut,"%le\t",data);
		}
		fprintf(fOut,"\n");
		count++;
		if(count == nrows){
		  count = 0;
		  fprintf(fOut,"\n");
		}	  
	}
    fclose(fOut);
    fclose(fIn);

return 0;
}

