// -*- c++ -*-
/// @file LtlGraph.h
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

#ifndef LtlGraph_h_
# define LtlGraph_h_
# ifdef __GNUC__
#  pragma interface
# endif // __GNUC__

# include "Ltl.h"
# include "BitVector.h"
# include <map>

/**
 * A node in a graph representing an LTL formula.
 *
 * The graph itself is a set of nodes of this kind.
 * The LtlGraphNode class is merely a front end for the set representation.
 */
class LtlGraphNode
{
public:
  /// From which nodes can one come to this node.
  std::set<unsigned> m_incoming;
  /// A set of formulae to be processed.
  class BitVector m_new;
  /// A set of formulae that must hold, when this node is entered.
  class BitVector m_old;
  /// A set of atomic propositions that must hold, when this node is entered.
  class BitVector m_atomic;
  /// A set of formulae that must hold in immediate next nodes.
  class BitVector m_next;

  /// Default constructor
  LtlGraphNode () :
    m_incoming (), m_new (), m_old (), m_atomic (), m_next () {}

  /// Constructs a specific node.
  LtlGraphNode (const std::set<unsigned>& incoming,
		const class BitVector& neww,
		const class BitVector& old,
		const class BitVector& atomic,
		const class BitVector& next) :
    m_incoming (incoming), m_new (neww), m_old (old),
    m_atomic (atomic), m_next (next) {}
  /// Copy constructor
  LtlGraphNode (const class LtlGraphNode& node) :
    m_incoming (node.m_incoming), m_new (node.m_new),
    m_old (node.m_old), m_atomic (node.m_atomic), m_next (node.m_next) {}
private:
  /// Assignment operator
  class LtlGraphNode& operator= (const class LtlGraphNode& node);
public:
  /// Destructor
  ~LtlGraphNode () {}
};

/**
 * Graph representation of a generalized B�chi automaton
 * corresponding to an LTL formula
 *
 * A LtlGraph is automatically constructed from a given
 * Ltl formula. All relevant information is fully stored to the
 * graph, so that the formula itself may well be destoyed right after
 * the conversion. The Ltl operators/operands provide themselves
 * the knowledge of how to be expanded into a LtlGraph.
 */
class LtlGraph
{
public:
  /*@{*/
  /// Map from state numbers to graph nodes
  typedef std::map<unsigned,class LtlGraphNode> Map;
  /// Iterator to the map
  typedef Map::iterator iterator;
  /// Constant iterator to the map
  typedef Map::const_iterator const_iterator;
  /*@}*/
public:
  /// Next available state number
  unsigned m_next;

  /// Map from state numbers to graph nodes (0=initial state)
  std::map<unsigned,class LtlGraphNode> m_nodes;

public:
  ///
  LtlGraph(){}
  /// Copy constructor
  LtlGraph (const class LtlGraph& old);
  /// Assignment operator
  class LtlGraph& operator= (const class LtlGraph& other);
public:
  /// Constructs a graph from a given LTL formula.
  LtlGraph (const class Ltl& formula);

  /// Destructor
  ~LtlGraph () {}

  /** @name Accessors to the graph nodes */
  /*@{*/
  const_iterator begin () const { return m_nodes.begin (); }
  const_iterator end () const { return m_nodes.end (); }
  /*@}*/
};

class Transition {

public:
    int source;
    int target;
    std::set<const LtlAtom*> transition_atoms;

public:
    Transition() {}

};

/// a wrapper class
/// epsilon is the transition with empty transition_atoms
class Automata : public LtlGraph {

public:
    unsigned int initial_state;
    std::set<unsigned int> states;
    std::set<unsigned int> finalStates;
    std::set<Transition *> transitions;

public:
    void getTransition(unsigned int state, std::set<Transition *> &result);
    void transite(unsigned int state, unsigned int atom, std::set<unsigned int> &result);
    void dump();

public:
    Automata(const class Ltl& ltl);

    void getTransition(std::set<Transition *> &result, unsigned int state);
};

#endif // LtlGraph_h_
