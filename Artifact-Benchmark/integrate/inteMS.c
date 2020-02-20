/* FEVS: A Functional Equivalence Verification Suite for High-Performance
 * Scientific Computing
 *
 * Copyright (C) 2010, Stephen F. Siegel, Timothy K. Zirkel,
 * University of Delaware
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */


#include<stdlib.h>
#include<stdio.h>
#include<assert.h>
#include<math.h>
#include<float.h>
#include<mpi.h>

#define WORK_TAG 1
#define TERMINATE_TAG 2
#define RESULT_TAG 3

double epsilon = DBL_MIN*10;
double pi = M_PI;

double f1(double x) { return sin(x); }

double f2(double x) { return cos(x); }

/* Sequential function to recursively estimate integral of f from a to
 * b.  fa=f(a), fb=f(b), area=given estimated integral of f from a to
 * b.  The interval [a,b] is divided in half and the areas of the two
 * pieces are estimated using Simpson's rule.  If the sum of those two
 * areas is within tolerance of given area, convergence has been
 * achieved and the result returned is the sum of the two areas.
 * Otherwise the function is called recursively on the two
 * subintervals and the sum of the results returned.
 */
double integrate_seq(double a, double b, double fa, double fb, double area,
		     double tolerance, double f(double),int *count) {
  double delta = b - a;
  double c = a+delta/2;
  double fc = f(c);
  double leftArea = (fa+fc)*delta/4;
  double rightArea = (fc+fb)*delta/4;

  (*count)++;
  if (tolerance < epsilon) {
    printf("Tolerance may not be possible to obtain.\n");
    return leftArea+rightArea;
  }
  if (fabs(leftArea+rightArea-area)<=tolerance) {
    return leftArea+rightArea;
  }
  return integrate_seq(a, c, fa, fc, leftArea, tolerance/2, f,count) +
    integrate_seq(c, b, fc, fb, rightArea, tolerance/2, f,count);
}

/* Sequential algorithm to estimate integral of f from a to b within
 * given tolerance. */
double integral_seq(double a, double b, double tolerance, double f(double),int *count) {
  //count = 0;
  return integrate_seq(a, b, f(a), f(b), (f(a)+f(b))*(b-a)/2, tolerance, f,count);
}

/* Worker function: called by procs of non-0 rank only.  Accepts
 * task from manager.   Each task is represented by a single integer
 * i, corresponding to the i-th subinterval of [a,b].  The task is
 * to compute the integral of f(x) over that subinterval within
 * tolerance of subtolerance.   At the end, a Bcast is used to
 * get final answer from manager. */
double worker(double a, double f(double),int *count,double subintervalLength,double subtolerance) {
  double left, right, result, answer;
  int task;
  MPI_Status status;

   while (1) {
    MPI_Recv(&task, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if (status.MPI_TAG == WORK_TAG) {
    left = a+task*subintervalLength;
    right = left+subintervalLength;
    result=integral_seq(left, right, subtolerance, f,count);
    MPI_Send(&result, 1, MPI_DOUBLE, 0, RESULT_TAG, MPI_COMM_WORLD);
    } else {
     break;
     }
    }
    fflush(stdout);
  return 0.0;
}

/* manager: this function called by proc 0 only.  Distributes tasks
 * to workers and accumulates results.   Each task is represented
 * by an integer i.  The integer i represents a subinterval
 * of the interval [a,b].   Returns the final result.  */
double manager(double f(double),int numTasks) {
  int i, nprocs,task = 0;
  MPI_Status status;
  double result, answer = 0.0;
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  for (i=1; i<nprocs; i++) {
    MPI_Send(&task, 1, MPI_INT, i, WORK_TAG, MPI_COMM_WORLD);
    task++;
  }
  int flag = 0;
  while (task < numTasks) {
//[svs] -- task allotment statically
     MPI_Recv(&result, 1, MPI_DOUBLE, MPI_ANY_SOURCE, RESULT_TAG, MPI_COMM_WORLD, &status); 
     MPI_Send(&task, 1, MPI_INT, status.MPI_SOURCE, WORK_TAG, MPI_COMM_WORLD);
     task++;
     answer += result;
  }
  
  for (i=1; i<nprocs; i++) {
    MPI_Recv(&result, 1, MPI_DOUBLE, i, RESULT_TAG,
	     MPI_COMM_WORLD, &status);
    MPI_Send(NULL, 0, MPI_INT, status.MPI_SOURCE, TERMINATE_TAG, MPI_COMM_WORLD); 
    answer += result;
  }
  return answer;
}

/* called in collective fashion */
double integral(double a, double b, double tolerance, double f(double),int *count,int rank) {
  int nprocs;
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  int numWorkers = nprocs - 1;
  int numTasks = nprocs+2;
double subintervalLength, subtolerance;

  subintervalLength = (b-a)/numTasks;
  subtolerance = tolerance/numTasks;
  if (rank == 0) {
    return manager(f,numTasks);
  } else {
    return worker(a,f,count,subintervalLength,subtolerance);
  }
}

int main (int argc, char **argv) {

int count = 0;
int  rank;

  double result;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  result = integral(0, pi, 1, f1,&count,rank);
  // if (rank == 0) { printf("%4.20lf\n", result); }
  /* result = integral(0, pi, .0000000001, f2); */
  /* if (rank == 0) { printf("%4.20lf\n", result); } */
  MPI_Finalize();

  return 0;
}
