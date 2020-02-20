/*
 *   Heat conduction demo program
 *
 *  solves the heat equation on a 2D grid.
 *
 *  Initial version March 2009
 *  Matthias.Lieber@tu-dresden.de
 *  Tobias.Hilbrich@zih.tu-dresden.de
 *
 *  Header for MPI version.
 *
 *  "Adapted" for the SC11 debugging tutorial by:
 *  David Lecomber <david@allinea.com>
 *  Ganesh Gopalakrishnan <ganesh@cs.utah.edu>
 *  Bronis R. de Supinski <bronis@llnl.gov>
 *  Wei-Fan <wfchiang@cs.utah.edu>
 *  Anh Vo <avo@cs.utah.edu>
 *  Tobias Hilbrich <tobias.hilbrich@tu-dresden.de>
 */

#include <math.h>
//#include <tgmath.h>//uclibc's libm
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

/* This type represents the grid and its description */
typedef struct
{
    /* current theta array */
    double **theta;
    /* new theta array */
    double **thetanew;
    /* domain size (number of grid cells) in x and y */
    int xsize;
    int ysize;
    /* size of a grid cell */
    double dx;
    double dy;
    /* "heat equation constant" */
    double k;
} heatGrid;

/* This type defines everything thats needed for MPI */
typedef struct {
    /* Own rank, used to only let master do output*/
    int rank;
    /* Comm for a cartesian distribution of the grid*/
    MPI_Comm cart;
    /* Neighbors in communicator*/
    int up,down,left,right;
    /* Start of a processes sub-grid (x, y)*/
    int start_x;
    int start_y;
    /* Number of cells in x or y direction for this process*/
    int num_cells_x;
    int num_cells_y;
    /* Datatype used to transfer a data column*/
    MPI_Datatype columntype;
} dataMPI;

void heatAllocate(heatGrid *grid, int xsize, int ysize);
void heatDeallocate(heatGrid *grid);
void heatInitialize(heatGrid* grid);
double heatInitFunc(double x);
void heatPrint(heatGrid* grid);
void heatTimestep(heatGrid* grid, dataMPI* mympi, double dt, double* dthetamax);
void heatBoundary(heatGrid* grid, dataMPI* mympi);
void heatOutput (heatGrid* grid, int filenumber);
void heatTotalEnergy(heatGrid* grid, double *energy);
void heatMPISetup (int* pargc, char*** pargv, heatGrid *grid, dataMPI* configMPI);
void heatMPIFree (dataMPI* configMPI);
void heatMPIGather (heatGrid *grid, dataMPI* configMPI);

double fmax(double a ,double b)
{
return a>b?a:b;
}

double fmin(double a, double b)
{
return a<b?a:b;
}
