// -*- c++ -*-
/// @file Ltl.C
/*
 * This file is based on code originally written by Mauno R�nkk�,
 * Copyright � 1998 Mauno R�nkk� <mronkko@abo.fi>.
 *
 * Modifications by Heikki Tauriainen <Heikki.Tauriainen@hut.fi>
 * and Marko M�kel� <Marko.Makela@hut.fi>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifdef __GNUC__
# pragma implementation
#endif // __GNUC__
#include "ltl/Ltl.h"
#include "ltl/LtlGraph.h"
#include <assert.h>

Ltl::Store Ltl::m_store;

class Ltl&
Ltl::Store::insert (class Ltl& ltl)
{
  std::pair<std::set<class Ltl*,struct ltless>::iterator,bool> p =
    m_set.insert (&ltl);
  if (!p.second)
    delete &ltl;
  else {
    // new formula: add it to the translation table
    unsigned num = m_set.size ();
    if (!(num & (num - 1))) {
      const class Ltl** table = new const class Ltl*[num << 1];
      memcpy (table, m_table, (num - 1) * sizeof *m_table);
      delete[] m_table;
      m_table = table;
    }
    m_table[ltl.m_num = num - 1] = &ltl;
  }
  return **p.first;
}

bool
Ltl::operator< (const class Ltl& other) const
{
  if (getKind () < other.getKind ()) return true;
  if (other.getKind () < getKind ()) return false;
  switch (getKind ()) {
  case Atom:
    return static_cast<const class LtlAtom&>(*this) <
      static_cast<const class LtlAtom&>(other);
  case Constant:
    return static_cast<const class LtlConstant&>(*this) <
      static_cast<const class LtlConstant&>(other);
  case Junct:
    return static_cast<const class LtlJunct&>(*this) <
      static_cast<const class LtlJunct&>(other);
  case Iff:
    return static_cast<const class LtlIff&>(*this) <
      static_cast<const class LtlIff&>(other);
  case Future:
    return static_cast<const class LtlFuture&>(*this) <
      static_cast<const class LtlFuture&>(other);
  case Until:
    return static_cast<const class LtlUntil&>(*this) <
      static_cast<const class LtlUntil&>(other);
  }
  assert (false);
  return false;
}

void
LtlAtom::expand (class LtlGraphNode& node,
		 std::stack<class LtlGraphNode*>& to_expand) const
{
  if (!node.m_old[negClone ().m_num])
    node.m_atomic.assign_true (m_num);
  else {
    to_expand.pop ();
    delete &node;
  }
}

void
LtlConstant::expand (class LtlGraphNode& node,
		     std::stack<class LtlGraphNode*>& to_expand) const
{
  if (!m_true) {
    to_expand.pop ();
    delete &node;
  }
}

void
LtlJunct::expand (class LtlGraphNode& node,
		  std::stack<class LtlGraphNode*>& to_expand) const
{
  if (m_con) {
    if (!node.m_old[m_left.m_num])
      node.m_new.assign_true (m_left.m_num);
    if (!node.m_old[m_right.m_num])
      node.m_new.assign_true (m_right.m_num);
  }
  else {
    class BitVector new_set (node.m_new);
    if (!node.m_old[m_left.m_num])
      new_set.assign_true (m_left.m_num);
    if (!node.m_old[m_right.m_num])
      node.m_new.assign_true (m_right.m_num);
    to_expand.push (new class LtlGraphNode (node.m_incoming, new_set,
					    node.m_old, node.m_atomic,
					    node.m_next));
  }
}

void
LtlIff::expand (class LtlGraphNode& node,
		std::stack<class LtlGraphNode*>& to_expand) const
{
  class BitVector new_set (node.m_new);
  class Ltl& neg_left = m_left.negClone ();
  class Ltl& neg_right = m_right.negClone ();
  if (!node.m_old[m_left.m_num])
    node.m_new.assign_true (m_left.m_num);
  if (!new_set[neg_left.m_num])
    new_set.assign_true (neg_left.m_num);
  if (m_iff) {
    if (!node.m_old[m_right.m_num])
      node.m_new.assign_true (m_right.m_num);
    if (!new_set[neg_right.m_num])
      new_set.assign_true (neg_right.m_num);
  }
  else {
    if (!node.m_old[neg_right.m_num])
      node.m_new.assign_true (neg_right.m_num);
    if (!new_set[m_right.m_num])
      new_set.assign_true (m_right.m_num);
  }
  to_expand.push (new class LtlGraphNode (node.m_incoming, new_set,
					  node.m_old, node.m_atomic,
					  node.m_next));
}

void
LtlUntil::expand (class LtlGraphNode& node,
		  std::stack<class LtlGraphNode*>& to_expand) const
{
  if (m_release) {
    if (!node.m_old[m_right.m_num])
      node.m_new.assign_true (m_right.m_num);
    class BitVector new_set (node.m_new);
    if (!node.m_old[m_left.m_num])
      node.m_new.assign_true (m_left.m_num);
    class BitVector next_set (node.m_next);
    next_set.assign_true (m_num);
    to_expand.push (new class LtlGraphNode (node.m_incoming, new_set,
					    node.m_old, node.m_atomic,
					    next_set));
  }
  else {
    class BitVector new_set (node.m_new);
    if (!node.m_old[m_left.m_num])
      new_set.assign_true (m_left.m_num);
    if (!node.m_old[m_right.m_num])
      node.m_new.assign_true (m_right.m_num);
    class BitVector next_set (node.m_next);
    next_set.assign_true (m_num);
    to_expand.push (new class LtlGraphNode (node.m_incoming, new_set,
					    node.m_old, node.m_atomic,
					    next_set));
  }
}

void
LtlFuture::expand (class LtlGraphNode& node,
		   std::stack<class LtlGraphNode*>& to_expand) const
{
  switch (m_op) {
  case next:
    node.m_next.assign_true (m_formula.m_num);
    return;
  case finally:
    {
      class BitVector new_set (node.m_new);
      if (!node.m_old[m_formula.m_num])
	new_set.assign_true (m_formula.m_num);
      to_expand.push (new class LtlGraphNode (node.m_incoming,
					      new_set,
					      node.m_old,
					      node.m_atomic,
					      node.m_next));
    }
    break;
  case globally:
    if (!node.m_old[m_formula.m_num])
      node.m_new.assign_true (m_formula.m_num);
    break;
#ifndef NDEBUG
  default:
    assert (false);
#endif // NDEBUG
  }
  node.m_next.assign_true (m_num);
}
