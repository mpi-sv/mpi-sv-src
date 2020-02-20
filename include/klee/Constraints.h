//===-- Constraints.h -------------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef KLEE_CONSTRAINTS_H
#define KLEE_CONSTRAINTS_H
#include <stdio.h>
#include "klee/util/Ref.h"
#include "klee/Expr.h"

#include <vector>
#include <queue>
#include <algorithm>
#include <glog/logging.h>
// FIXME: Currently we use ConstraintManager for two things: to pass
// sets of constraints around, and to optimize constraints. We should
// move the first usage into a separate data structure
// (ConstraintSet?) which ConstraintManager could embed if it likes.
namespace klee {

class ExprVisitor;
  
class ConstraintManager {
public:
  typedef std::vector< ref<Expr> > constraints_ty;
  typedef constraints_ty::iterator iterator;
  typedef constraints_ty::const_iterator const_iterator;

  ConstraintManager() {}

  // create from constraints with no optimization
  explicit
  ConstraintManager(const std::vector< ref<Expr> > &_constraints) :
    constraints(_constraints) {}

  ConstraintManager(const ConstraintManager &cs)
    : constraints(cs.constraints) {}

  typedef std::vector< ref<Expr> >::const_iterator constraint_iterator;

  // given a constraint which is known to be valid, attempt to 
  // simplify the existing constraint set
  void simplifyForValidConstraint(ref<Expr> e);

  ref<Expr> simplifyExpr(ref<Expr> e) const;

  void addConstraint(ref<Expr> e);

  /*
   * added yhb
   * used to indicate whether a path condition is contained in the current path condition*/
  bool containsCM(ConstraintManager cs){
	  std::vector< ref<Expr> > ::iterator it=cs.constraints.begin();
	  while(it!=cs.constraints.end()){
		  if(find(constraints.begin(), constraints.end(), *it)!=constraints.end()){
			  it++;
		  }
		  else{
			  return false;
		  }
	  }
	  return true;
  }

  /*
   * added yhb
   * used to dump the PC*/
  bool dumpPC(){
	  std::vector< ref<Expr> > ::iterator it=constraints.begin();
	  while(it!=constraints.end()){
		    //     LOG(INFO)<<"con is:"<<*it;
	     it++;
	  }
	  return true;
  }

  bool empty() const {
    return constraints.empty();
  }
  ref<Expr> back() const {
    return constraints.back();
  }
  constraint_iterator begin() const {
    return constraints.begin();
  }
  constraint_iterator end() const {
    return constraints.end();
  }
  size_t size() const {
    return constraints.size();
  }

  bool operator==(const ConstraintManager &other) const {
    return constraints == other.constraints;
  }

private:
  std::vector< ref<Expr> > constraints;

  // returns true iff the constraints were modified
  bool rewriteConstraints(ExprVisitor &visitor);

  void addConstraintInternal(ref<Expr> e);
};

}

#endif /* KLEE_CONSTRAINTS_H */
