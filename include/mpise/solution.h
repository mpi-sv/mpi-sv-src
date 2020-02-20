/*
 * solution.h
 *
 *  Created on: 2017-10-2
 *      Author: yhb
 */

#ifndef SOLUTION_H_
#define SOLUTION_H_


#endif /* SOLUTION_H_ */

/* the recursion function, invoked by function g*/
void f(int *r, int* p, int n, int m, list<int *> *result);

/*
 *  compute the solutions for assigning n tasks to m slaves
 *    the results are stored in list result
 * */
void g(int n, int m, list<int *> *result);
