/******************************************************************************
* File     : postProcess.h                                                    *
* Author   : Carlos Rosales Fernandez (carlos.rosales.fernandez(at)gmail.com) *
* Revision : 1.0 (2008-12-23)                                                 *
******************************************************************************/
/**
 * @brief Prototype declarations of post-processing functions
 *
 * @file
 * Prototype declarations of post-processing functions
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

int saveData(unsigned int ANALYSIS,
             char *cOutputType,
             double **Xinner,
             double **XF,
             double **globalXe,
             double **globalPot,
             double **globalField,
             double **globalForce,
             double *globalFcm,
             double *globalXcm);

int postProcessSetup(char *cOutputType,
                     double **xCols,
                     double **XinnerTemp,
                     double **Xinner);

int multipoleSetup(unsigned int ANALYSIS,
                   double *axis,
                   unsigned int *nShape,
                   unsigned int *nOrder,
                   double *R);

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
                      double *localXcm);

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
                      double *localXcm);

