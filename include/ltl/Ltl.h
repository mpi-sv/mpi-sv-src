// -*- c++ -*-
/// @file Ltl.h
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

#ifndef Ltl_h_
# define Ltl_h_
# ifdef __GNUC__
#  pragma interface
# endif // __GNUC__

# ifdef DEBUG
#  include <stdio.h>
# endif // DEBUG
# include <set>
# include <stack>

/** Abstract base class for LTL formulae */
class Ltl
{
public:
  /// LTL formula comparator
  struct ltless {
    /// Lexicographic order of LTL formulae
    /// @param l1	pointer to first formula to compare
    /// @param l2	pointer to second formula to compare
    /// @return		true if l1 is before l2
    bool operator() (const class Ltl* l1,
		     const class Ltl* l2) const {
      return *l1 < *l2;
    }
  };
private:
  /// Set of instantiated LTL formulae
  class Store {
  private:
    /// Set of LTL formulae
    std::set<class Ltl*,struct ltless> m_set;
    /// Table of LTL formulae, indexed by Ltl::m_num
    const class Ltl** m_table;
    /// Copy constructor
    Store (const class Store&);
    /// Assignment operator
    class Store& operator= (const class Store&);
  public:
    /// Constructor
    Store () : m_set (), m_table (0) {}
    /// Destructor
    ~Store () {
      delete[] m_table;
      for (std::set<class Ltl*,struct ltless>::iterator i = m_set.begin ();
	   i != m_set.end (); i++) {
	delete *i;
      }
    }
    /// Insert a formula to the set
    class Ltl& insert (class Ltl& ltl);
    /// Translate a formula index to a formula reference
    /// @param f	formula index to be translated
    /// @return		the corresponding formula
    const class Ltl& fetch (unsigned f) const { return *m_table[f]; }
  };
  static class Store m_store;
  friend class Store;
public:
  /// Number of the LTL formula
  unsigned m_num;
  /// Constructor
  Ltl () : m_num (0) {}
private:
  /// Copy constructor
  Ltl (const class Ltl& old);
  /// Assignment operator
  class Ltl& operator= (const class Ltl& old);
protected:
  /// Destructor
  virtual ~Ltl () {}
  /// Reference registrator
  /// @param ltl	formula to be registered
  /// @return		an equivalent formula
  static class Ltl& insert (class Ltl& ltl) { return m_store.insert (ltl); }
public:
  /// Translate a formula index to a formula reference
  /// @param f		formula index to be translated
  /// @return		the corresponding formula
  static const class Ltl& fetch (unsigned f) { return m_store.fetch (f); }

  /// Returns a clone of the negation of this Ltl formula.
  virtual class Ltl& negClone () const = 0;

  /// Formula kinds
  enum Kind { Atom, Constant, Junct, Iff, Future, Until };
  /// Determine the kind of the formula
  virtual enum Kind getKind () const = 0;
  /// Less-than comparison
  bool operator< (const class Ltl& other) const;

  /// Implements operator/operand specific details of the expansion algorithm.
  /// @param node	the node representing the formula
  /// @param to_expand	stack of nodes to be expanded
  virtual void expand (class LtlGraphNode& node,
		       std::stack<class LtlGraphNode*>& to_expand) const = 0;

# ifdef DEBUG
  /// Stream output
  virtual void display (FILE* out) const = 0;
# endif // DEBUG
};

/** LTL atom */
class LtlAtom : public Ltl
{
private:
  /// The proposition number
  unsigned m_value;
  /// Flag: is the proposition negated?
  bool m_negated;

public:
  /// Constructor
  /// @param value	proposition number
  /// @param negated	flag: is the proposition negated?
  /// @return		an LTL atom
  static class Ltl& construct (unsigned value, bool negated) {
    return insert (*new class LtlAtom (value, negated));
  }
  /// Returns a clone of the negation of this Ltl formula.
  class Ltl& negClone () const {
    return insert (*new class LtlAtom (m_value, !m_negated));
  }
private:
  /// Constructor
  LtlAtom (unsigned value, bool negated = false) :
    Ltl (), m_value (value), m_negated (negated) {}
  /// Copy constructor
  LtlAtom (const class LtlAtom& old);
  /// Assignment operator
  class LtlAtom& operator= (const class LtlAtom& old);
protected:
  /// Destructor
  ~LtlAtom () {}
public:

  /// Determine the kind of the formula
  enum Kind getKind () const { return Atom; }
  /// Less-than comparison
  bool operator< (const class LtlAtom& other) const {
    return m_value < other.m_value ||
      (m_value == other.m_value && m_negated < other.m_negated);
  }

  /// Determine whether the atom is negated
  bool isNegated () const { return m_negated; }
  /// Determine the value of the proposition
  unsigned getValue () const { return m_value; }

  /// Implements operator/operand specific details of the expansion algorithm.
  /// @param node	the node representing the formula
  /// @param to_expand	stack of nodes to be expanded
  void expand (class LtlGraphNode& node,
	       std::stack<class LtlGraphNode*>& to_expand) const;

# ifdef DEBUG
  /// Stream output
  void display (FILE* out) const {
    if (m_negated) fputs ("! ", out);
    fprintf (out, "p%u", m_value);
  }
# endif // DEBUG
};

/** LTL constants 'true' and 'false' */
class LtlConstant : public Ltl
{
private:
  /// The truth value of the constant
  bool m_true;

public:
  /// Constructor
  /// @param true_	the constant 'true' or 'false'
  /// @return		an LTL constant
  static class Ltl& construct (bool true_) {
    return insert (*new class LtlConstant (true_));
  }
  /// Returns a clone of the negation of this Ltl formula.
  class Ltl& negClone () const {
    return insert (*new class LtlConstant (!m_true));
  }
private:
  /// Constructor
  LtlConstant (bool true_) : Ltl (), m_true (true_) {}
  /// Copy constructor
  LtlConstant (const class LtlConstant& old);
  /// Assignment operator
  class LtlConstant& operator= (const class LtlConstant& old);
protected:
  /// Destructor
  ~LtlConstant () {}
public:

  /// Determine the kind of the formula
  enum Kind getKind () const { return Constant; }
  /// Less-than comparison
  bool operator< (const class LtlConstant& other) const {
    return m_true < other.m_true;
  }

  /// Convert to truth value
  operator bool () const { return m_true; }

  /// Implements operator/operand specific details of the expansion algorithm.
  /// @param node	the node representing the formula
  /// @param to_expand	stack of nodes to be expanded
  void expand (class LtlGraphNode& node,
	       std::stack<class LtlGraphNode*>& to_expand) const;

# ifdef DEBUG
  /// Stream output
  void display (FILE* out) const { putc (m_true ? 't' : 'f', out); }
# endif // DEBUG
};

/** LTL conjunction and disjunction */
class LtlJunct : public Ltl
{
private:
  /// Flag: conjunction instead of disjunction?
  bool m_con;
  /// The left-hand-side sub-formula
  class Ltl& m_left;
  /// The right-hand-side sub-formula
  class Ltl& m_right;

public:
  /// Optimizing constructor
  /// @param con	flag: conjunction instead of disjunction
  /// @param l		the left-hand-side formula
  /// @param r		the right-hand-side formula
  /// @return		an equivalent formula
  static class Ltl& construct (bool con,
			       class Ltl& l,
			       class Ltl& r) {
    if (l.getKind () == Ltl::Constant)
      return bool (static_cast<class LtlConstant&>(l)) == con ? r : l;
    if (r.getKind () == Ltl::Constant)
      return bool (static_cast<class LtlConstant&>(r)) == con ? l : r;
    if (&l == &r)
      return l;
    return &l < &r
      ? insert (*new class LtlJunct (con, l, r))
      : insert (*new class LtlJunct (con, r, l));
  }
  /// Returns a clone of the negation of this Ltl formula.
  class Ltl& negClone () const {
    return insert (*new class LtlJunct (!m_con,
					m_left.negClone (),
					m_right.negClone ()));
  }

private:
  /// Constructor
  /// @param con	flag: conjunction instead of disjunction
  /// @param l		the left-hand-side formula
  /// @param r		the right-hand-side formula
  LtlJunct (bool con,
	    class Ltl& l,
	    class Ltl& r) :
    m_con (con), m_left (l), m_right (r) {}
  /// Copy constructor
  LtlJunct (const class LtlJunct& old);
  /// Assignment operator
  class LtlJunct& operator= (const class LtlJunct& old);
protected:
  /// Destructor
  ~LtlJunct () {}
public:
    const class Ltl& getLeft () const { return m_left; }

    const class Ltl& getRight () const { return m_right; }

    bool getCon() {return m_con;}
  /// Determine the kind of the formula
  enum Kind getKind () const { return Junct; }
  /// Less-than comparison
  bool operator< (const class LtlJunct& other) const {
    if (m_con < other.m_con) return true;
    if (other.m_con < m_con) return false;
    if (&m_left < &other.m_left) return true;
    if (&other.m_left < &m_left) return false;
    return &m_right < &other.m_right;
  }

  /// Implements operator/operand specific details of the expansion algorithm.
  /// @param node	the node representing the formula
  /// @param to_expand	stack of nodes to be expanded
  void expand (class LtlGraphNode& node,
	       std::stack<class LtlGraphNode*>& to_expand) const;

# ifdef DEBUG
  /// Stream output
  void display (FILE* out) const {
    fputs (m_con ? "& " : "| ", out);
    m_left.display (out);
    putc (' ', out);
    m_right.display (out);
  }
# endif // DEBUG
};

/** LTL equivalence or exclusive disjunction */
class LtlIff : public Ltl
{
private:
  /// Flag: equivalence instead of exclusive disjunction?
  bool m_iff;
  /// The left-hand-side sub-formula
  class Ltl& m_left;
  /// The right-hand-side sub-formula
  class Ltl& m_right;

public:
  /// Optimizing constructor
  /// @param iff	flag: equivalence instead of exclusive disjunction
  /// @param l		the left-hand-side formula
  /// @param r		the right-hand-side formula
  /// @return		an equivalent formula
  static class Ltl& construct (bool iff,
			       class Ltl& l,
			       class Ltl& r) {
    if (l.getKind () == Ltl::Constant)
      return bool (static_cast<class LtlConstant&>(l)) == iff
	? r
	: r.negClone ();
    if (r.getKind () == Ltl::Constant)
      return bool (static_cast<class LtlConstant&>(r)) == iff
	? l
	: l.negClone ();
    if (&l == &r)
      return LtlConstant::construct (iff);
    return &l < &r
      ? insert (*new class LtlIff (iff, l, r))
      : insert (*new class LtlIff (iff, r, l));
  }
  /// Returns a clone of the negation of this Ltl formula.
  class Ltl& negClone () const {
    return insert (*new class LtlIff (!m_iff,
				      m_left,
				      m_right));
  }

private:
  /// Constructor
  /// @param iff	flag: equivalence instead of exclusive disjunction
  /// @param l		the left-hand-side formula
  /// @param r		the right-hand-side formula
  LtlIff (bool iff,
	  class Ltl& l,
	  class Ltl& r) :
    m_iff (iff), m_left (l), m_right (r) {}
  /// Copy constructor
  LtlIff (const class LtlIff& old);
  /// Assignment operator
  class LtlIff& operator= (const class LtlIff& old);
protected:
  /// Destructor
  ~LtlIff () {}
public:
    const class Ltl& getLeft () const { return m_left; }

    const class Ltl& getRight () const { return m_right; }

    bool getIff() {return m_iff;}

  /// Determine the kind of the formula
  enum Kind getKind () const {return Iff; }
  /// Less-than comparison
  bool operator< (const class LtlIff& other) const {
    if (m_iff < other.m_iff) return true;
    if (other.m_iff < m_iff) return false;
    if (&m_left < &other.m_left) return true;
    if (&other.m_left < &m_left) return false;
    return &m_right < &other.m_right;
  }

  /// Implements operator/operand specific details of the expansion algorithm.
  /// @param node	the node representing the formula
  /// @param to_expand	stack of nodes to be expanded
  void expand (class LtlGraphNode& node,
	       std::stack<class LtlGraphNode*>& to_expand) const;

# ifdef DEBUG
  /// Stream output
  void display (FILE* out) const {
    fputs (m_iff ? "e " : "^ ", out);
    m_left.display (out);
    putc (' ', out);
    m_right.display (out);
  }
# endif // DEBUG
};

/** LTL next-time, globally and finally formulae */
class LtlFuture : public Ltl
{
public:
  /// Operator kinds
  enum Op { next, globally, finally };
private:
  /// The operator
  enum Op m_op;
  /// The formula being quantified
  class Ltl& m_formula;

public:
  /// Constructor
  /// @param op		the operator
  /// @param f		the formula
  /// @return		an LTL future formula
  static class Ltl& construct (enum Op op,
			       class Ltl& f) {
    return insert (*new class LtlFuture (op, f));
  }
  /// Returns a clone of the negation of this formula.
  class Ltl& negClone () const {
    switch (m_op) {
    case next:
      break;
    case globally:
      return insert (*new class LtlFuture (finally, m_formula.negClone ()));
    case finally:
      return insert (*new class LtlFuture (globally, m_formula.negClone ()));
    }
    return insert (*new class LtlFuture (m_op, m_formula.negClone ()));
  }

private:
  /// Constructor
  /// @param op		the operator
  /// @param f		the formula
  /// @param r		the right-hand-side formula
  LtlFuture (enum Op op,
	     class Ltl& f) :
    m_op (op), m_formula (f) {}
  /// Copy constructor
  LtlFuture (const class LtlFuture& old);
  /// Assignment operator
  class LtlFuture& operator= (const class LtlFuture& old);
protected:
  /// Destructor
  ~LtlFuture () {}
public:

  /// Determine the kind of the formula
  enum Kind getKind () const { return Future; }
  /// Less-than comparison
  bool operator< (const class LtlFuture& other) const {
    if (m_op < other.m_op) return true;
    if (other.m_op < m_op) return false;
    return &m_formula < &other.m_formula;
  }

  /// Get the temporal operator
  enum Op getOp () const { return m_op; }
  /// Get the subformula
  const class Ltl& getFormula () const { return m_formula; }

  /// Implements operator/operand specific details of the expansion algorithm.
  /// @param node	the node representing the formula
  /// @param to_expand	stack of nodes to be expanded
  void expand (class LtlGraphNode& node,
	       std::stack<class LtlGraphNode*>& to_expand) const;

# ifdef DEBUG
  /// Stream output
  void display (FILE* out) const {
    switch (m_op) {
    case next:
      putc ('X', out); break;
    case globally:
      putc ('G', out); break;
    case finally:
      putc ('F', out); break;
    }
    putc (' ', out);
    m_formula.display (out);
  }
# endif // DEBUG
};

/** LTL 'until' and 'release' operators */
class LtlUntil : public Ltl
{
private:
  /// Flag: release instead of until
  bool m_release;
  /// The left-hand-side sub-formula
  class Ltl& m_left;
  /// The right-hand-side sub-formula
  class Ltl& m_right;

public:
  /// Constructor
  /// @param release	flag: 'release' instead of 'until'
  /// @param l		the left-hand-side formula
  /// @param r		the right-hand-side formula
  /// @return		an LTL formula
  static class Ltl& construct (bool release,
			       class Ltl& l,
			       class Ltl& r) {
    return insert (*new class LtlUntil (release, l, r));
  }
private:
  /// Constructor
  /// @param release	flag: 'release' instead of 'until'
  /// @param l		the left-hand-side formula
  /// @param r		the right-hand-side formula
  LtlUntil (bool release,
	    class Ltl& l,
	    class Ltl& r) :
    m_release (release), m_left (l), m_right (r) {}
  /// Copy constructor
  LtlUntil (const class LtlUntil& old);
  /// Assignment operator
  class LtlUntil& operator= (const class LtlUntil& old);
public:
  /// Destructor
  ~LtlUntil () {}

  /// Returns a clone of the negation of this Ltl formula.
  class Ltl& negClone () const {
    return insert (*new class LtlUntil (!m_release,
					m_left.negClone (),
					m_right.negClone ()));
  }

  /// Determine the kind of the formula
  enum Kind getKind () const { return Until; }
  /// Less-than comparison
  bool operator< (const class LtlUntil& other) const {
    if (m_release < other.m_release) return true;
    if (other.m_release < m_release) return false;
    if (&m_left < &other.m_left) return true;
    if (&other.m_left < &m_left) return false;
    return &m_right < &other.m_right;
  }

  /// Determine whether this is a release operator instead of until
  bool isRelease () const { return m_release; }
  /// Get the left-hand-side subformula
  const class Ltl& getLeft () const { return m_left; }
  /// Get the right-hand-side subformula
  const class Ltl& getRight () const { return m_right; }

  /// Implements operator/operand specific details of the expansion algorithm.
  /// @param node	the node representing the formula
  /// @param to_expand	stack of nodes to be expanded
  void expand (class LtlGraphNode& node,
	       std::stack<class LtlGraphNode*>& to_expand) const;

# ifdef DEBUG
  /// Stream output
  void display (FILE* out) const {
    fputs (m_release ? "V " : "U ", out);
    m_left.display (out);
    putc (' ', out);
    m_right.display (out);
  }
# endif // DEBUG
};

#endif // Ltl_h_
