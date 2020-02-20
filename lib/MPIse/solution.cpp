/*
 * solution.cpp
 *
 *  Created on: 2017-10-1
 *      Author: yhb
 */

#include <iostream>
#include <fstream>
#include <list>
#include <map>
#include <vector>
#include <string>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include "mpise/generate.h"
#include "mpise/analysis.h"
//----added
#include "mpise/commlog.h"
#include "klee/ExecutionState.h"

using namespace llvm;
using namespace klee;
using namespace std;

void f(int *r, int* p, int n, int m, list<int *> *result)
{

  if(m==1){
    int *pp=new int[m];
    int count=0;
    for(;r!=p;++r)
      {pp[count++]=*r;}
    //printf("**%d**\n",n);
     pp[count++]=n;
    result->push_front(pp);
  }
  else
    for(*p=0;*p<=n;++*p)
      f(r,p+1,n-*p,m-1,result);
}

void g(int n, int m, list<int *> *result)
{
  int r[m];
  f(r,r,n,m,result);
}
