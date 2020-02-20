/******************************************************************************
* File     : constants.h                                                      *
* Author   : Carlos Rosales Fernandez (carlos.rosales.fernandez(at)gmail.com) *
* Revision : 1.0 (2008-12-23)                                                 *
******************************************************************************/
/**
 * @brief Definition of all global constants.
 *
 * @file
 * Definitions of all global constants for use with dep-electric code and
 * pre-processor macros.
 *
 * @def eps0 : Electric permittivity of vacuum (F/m)
 * @def mu0  : Magnetic permittivity of vacuum (N/A^2)
 * @param qe   : Elementary (electron) charge (C)
 * @param pi   : The constant Pi
 * @param pi2  :  2*Pi
 * @param pio2 : Pi/2
 * @param pio4 : Pi/4
 * @param NGAUSS   : [ Constant ] Number of points in standard line quadrature
 * @param TNGAUSS  : Number of points in standard triangle cubature
 * @param TSNGAUSS : Number of points in singular triangle cubature
 * @param NSUBDIVISIONS : Number of subdivisions for strong singularity integral
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

#define eps0    8.854187817E-12    /* Electric permittivity of vacuum (F/m)   */
#define mu0     1.256637061E-06    /* Magnetic permittivity of vacuum (N/A^2) */
#define qe      1.602176531E-19         /* Elementary (electron) charge (C)   */
#define pi      3.141592653589793E+00   /* Pi   */
#define pi2     6.283185307179586E+00   /* 2*Pi */
#define pi4     1.256637061435917E+01   /* 4*Pi */
#define pio2    1.570796326794896E+00   /* Pi/2 */
#define pio4    7.853981633974483E-01   /* Pi/4 */

/* Dimensions of the Gauss quadrature matrices */
#define NGAUSS  8
#define TNGAUSS 7
#define TSNGAUSS    7
#define NSUBDIVISIONS   8

/* Macros used to reorder nodes in strongly singular integrals */
#define shift(a,b,c){double d = (a); (a) = (b); (b) = (c); (c) = d;}
#define swap(a,b){double c = (a); (a) = (b); (b) = c;}

