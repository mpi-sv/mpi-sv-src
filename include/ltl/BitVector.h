// -*- c++ -*-
/// @file BitVector.h
/*
 *  Copyright © 2001 Marko Mäkelä <msmakela@tcs.hut.fi>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef BITVECTOR_H_
# define BITVECTOR_H_
# ifdef __GNUC__
#  pragma interface
# endif // __GNUC__

# include <limits.h>
# include <string.h>

/** Binary digit (bit) vector */
class BitVector
{
public:
  /** machine word */
  typedef unsigned long word_t;

  /** Constructor
   * @param size	number of elements in the vector
   */
  explicit BitVector (unsigned size = 0) :
    m_size (0), m_allocated (1), m_bits (new word_t[1]) {
    *m_bits = 0;
    setSize (size);
  }
  /** Copy constructor */
  explicit BitVector (const class BitVector& old) :
    m_size (old.m_size), m_allocated (old.m_allocated), m_bits (0) {
    memcpy (m_bits = new word_t[m_allocated], old.m_bits,
	    m_allocated * sizeof (word_t));
  }
private:
  /** Assignment operator */
  class BitVector& operator= (const class BitVector& old);
public:
  /** Destructor */
  ~BitVector () { delete[] m_bits; }

  /** Equality comparison
   * @param other	other bit vector to compare this against
   * @return		true if the bit vectors are identical,
   *			treating missing bits as unset
   */
  bool operator== (const class BitVector& other) const {
    unsigned tsize = getNumWords (m_size), osize = getNumWords (other.m_size);
    if (tsize < osize) {
      if (memcmp (m_bits, other.m_bits, tsize * sizeof (word_t)))
	return false;
      for (register const word_t* bits = other.m_bits + osize;
	   bits-- > other.m_bits + tsize; )
	if (*bits)
	  return false;
    }
    else {
      if (memcmp (m_bits, other.m_bits, osize * sizeof (word_t)))
	return false;
      for (register const word_t* bits = m_bits + tsize;
	   bits-- > m_bits + osize; )
	if (*bits)
	  return false;
    }
    return true;
  }

  /** Determine the number of words the specified amount of bits occupy */
  static unsigned getNumWords (unsigned bits) {
    const unsigned bitsPerWord = CHAR_BIT * sizeof (word_t);
    return (bits + bitsPerWord - 1) / bitsPerWord;
  }

  /** Set the size of the vector
   * @param size	the new size of the vector
   */
  void setSize (unsigned size) {
    const unsigned numWords = getNumWords (size);
    if (m_allocated < numWords) {
      while (m_allocated < numWords)
	m_allocated <<= 1;
      word_t* bits = new word_t[m_allocated];
      const unsigned init = getNumWords (m_size);
      memcpy (bits, m_bits, init * sizeof *m_bits);
      memset (bits + init, 0, (m_allocated - init) * sizeof *m_bits);
      delete[] m_bits;
      m_bits = bits;
    }
    m_size = size;
  }
  /** Determine the size of the vector
   * @return	the size of the vector
   */
  unsigned getSize () const { return m_size; }

  /** Read a binary digit
   * @param i	zero-based index of the element
   * @return	value of the ternary digit
   */
  bool operator[] (unsigned i) const {
    return i < m_size &&
      m_bits[i / (CHAR_BIT * sizeof (word_t))] &
      (word_t (1) << (i % (CHAR_BIT * sizeof (word_t))));
  }

  /** Set a binary digit
   * @param i	zero-based index of the element
   */
  void assign_true (unsigned i) {
    if (i >= m_size)
      setSize (i + 1);
    m_bits[i / (CHAR_BIT * sizeof (word_t))] |=
      word_t (1) << (i % (CHAR_BIT * sizeof (word_t)));
  }

  /** Clear a binary digit
   * @param i	zero-based index of the element
   */
  void assign_false (unsigned i) {
    m_bits[i / (CHAR_BIT * sizeof (word_t))] &=
      ~(word_t (1) << (i % (CHAR_BIT * sizeof (word_t))));
  }

  /** Determine whether the whole vector is filled with zero bits
   * @return	0 if all bits in the vector are clear;
   *		index of a nonzero entry + 1 otherwise
   */
  unsigned nonzero () const {
    if (!m_size)
      return 0;
    return findNext (0);
  }

  /** Find the next index at which there is a set bit
   * @param i	the starting index (counting forward)
   * @return	0 if all following bits in the vector are clear;
   *		index of the next nonzero entry + 1 otherwise
   */
  unsigned findNext (unsigned i) const {
    if (i >= m_size)
      return 0;

    register const word_t* w = &m_bits[i / (CHAR_BIT * sizeof (word_t))];
    register word_t bit = 1 << (i % (CHAR_BIT * sizeof (word_t)));

    if (*w >= bit) {
      // found in the same word
      do
	if (*w & bit)
	  return i + 1;
      while (i++, bit <<= 1);
    }

    const word_t* const w_end = m_bits + getNumWords (m_size);

    // search in the remaining words
    while (++w < w_end) {
      if (!*w) continue;
      for (i = 1, bit = 1; !(*w & bit); bit <<= 1, i++);
      return i + (w - m_bits) * CHAR_BIT * sizeof (word_t);
    }

    return 0;
  }

private:
  /** Number of elements in the vector */
  unsigned m_size;
  /** Size of the allocated vector in words */
  unsigned m_allocated;
  /** The vector */
  word_t* m_bits;
};

#endif // BITVECTOR_H_
