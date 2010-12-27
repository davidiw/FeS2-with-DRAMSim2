/* This file is part of the Ruby Multiprocessor Memory System
   Simulator, a component of the Multifacet GEMS (General
   Execution-driven Multiprocessor Simulator) software toolset
   originally developed at the University of Wisconsin-Madison.

   Ruby was originally developed primarily by Milo Martin and Daniel
   Sorin with contributions from Ross Dickson, Carl Mauer, and Manoj
   Plakal.

   Substantial further development of Multifacet GEMS at the
   University of Wisconsin was performed by Alaa Alameldeen, Brad
   Beckmann, Ross Dickson, Pacia Harper, Milo Martin, Michael Marty,
   Carl Mauer, Kevin Moore, Manoj Plakal, Daniel Sorin, Min Xu, and
   Luke Yen.

   Additional development was performed at the University of
   Pennsylvania by Milo Martin.

   --------------------------------------------------------------------

   Copyright (C) 1999-2005 by Mark D. Hill and David A. Wood for the
   Wisconsin Multifacet Project.  Contact: gems@cs.wisc.edu
   http://www.cs.wisc.edu/gems/

   This file is based upon a pre-release version of Multifacet GEMS
   from 2004 and may also contain additional modifications and code
   Copyright (C) 2004-2007 by Milo Martin for the Penn Architecture
   and Compilers Group.  Contact: acg@lists.seas.upenn.edu
   http://www.cis.upenn.edu/acg/

   --------------------------------------------------------------------

   Multifacet GEMS is free software; you can redistribute it and/or
   modify it under the terms of version 2 of the GNU General Public
   License as published by the Free Software Foundation.  This 
   software comes with ABSOLUTELY NO WARRANTY.  For details see the 
   file LICENSE included with the distribution.
*/
// -----------------------------------------------------------------------------
//
//  This file is part of FeS2.
//
//  FeS2 is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  FeS2 is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with FeS2.  If not, see <http://www.gnu.org/licenses/>.
//
// -----------------------------------------------------------------------------

#include "Set.h"
#include "Param.h"

Set::Set()
{ 
  setSize(g_param_ptr->NUM_NODES());
}

Set::Set(int size)
{ 
  setSize(size);
}

bool Set::isEqual(const Set& set)
{
  return (m_bits == set.m_bits);
}

void Set::add(NodeID index)
{
  assert((m_bits & m_mask) == m_bits);  // check for any bits outside the range
  assert(index < m_size);
  m_bits |= (1 << index);
  assert((m_bits & m_mask) == m_bits);  // check for any bits outside the range
}

void Set::addSet(const Set& set)
{
  assert(m_size == set.m_size);
  m_bits |= set.m_bits;
  assert((m_bits & m_mask) == m_bits);  // check for any bits outside the range
}

void Set::addRandom()
{
  m_bits |= random();
  m_bits &= m_mask;
  assert((m_bits & m_mask) == m_bits);  // check for any bits outside the range
}

void Set::remove(NodeID index)
{
  assert(index < m_size);
  m_bits &= ~(1 << index);
  assert((m_bits & m_mask) == m_bits);  // check for any bits outside the range
}

void Set::removeSet(const Set& set)
{
  assert(m_size == set.m_size);
  m_bits &= ~(set.m_bits);
  assert((m_bits & m_mask) == m_bits);  // check for any bits outside the range
}

void Set::clear()
{
  m_bits = 0;
}

void Set::broadcast()
{
  m_bits = m_mask;
}

int Set::count() const
{
  int counter = 0;
  for (int i=0; i<m_size; i++) {
    if ((m_bits & (1 << i)) != 0) {
      counter++;
    }
  }
  return counter;
}

NodeID Set::smallestElement() const
{
  assert(count() > 0);
  int counter = 0;
  for (int i=0; i<m_size; i++) {
    if (isElement(i)) {
      return i;
    }
  }
  ERROR_MSG("No smallest element of an empty set.");
}

// Returns true iff all bits are set
bool Set::isBroadcast() const
{
  assert((m_bits & m_mask) == m_bits);  // check for any bits outside the range
  return (m_mask == m_bits);
}

// Returns true iff no bits are set
bool Set::isEmpty() const
{
  assert((m_bits & m_mask) == m_bits);  // check for any bits outside the range
  return (m_bits == 0);
}

// returns the logical OR of "this" set and orSet 
Set Set::OR(const Set& orSet) const
{
  assert(m_size == orSet.m_size);
  Set result(m_size);
  result.m_bits = (m_bits | orSet.m_bits);
  assert((result.m_bits & result.m_mask) == result.m_bits);  // check for any bits outside the range
  return result;
}

// returns the logical AND of "this" set and andSet 
Set Set::AND(const Set& andSet) const
{
  assert(m_size == andSet.m_size);
  Set result(m_size);
  result.m_bits = (m_bits & andSet.m_bits);
  assert((result.m_bits & result.m_mask) == result.m_bits);  // check for any bits outside the range
  return result;
}

// Returns true if the intersection of the two sets is non-empty
bool Set::intersectionIsNotEmpty(const Set& other_set) const
{
  assert(m_size == other_set.m_size);
  return ((m_bits & other_set.m_bits) != 0);
}

// Returns true if the intersection of the two sets is empty
bool Set::intersectionIsEmpty(const Set& other_set) const
{
  assert(m_size == other_set.m_size);
  return ((m_bits & other_set.m_bits) == 0);
}

bool Set::isSuperset(const Set& test) const
{
  assert(m_size == test.m_size);
  uint32 temp = (test.m_bits & (~m_bits));
  return (temp == 0);
}

bool Set::isElement(NodeID element) const
{
  return ((m_bits & (1 << element)) != 0);
}

void Set::setSize(int size) 
{
  // We're using 32 bit ints, and the 32nd bit acts strangely due to
  // signed/unsigned, so restrict the set size to 31 bits.
  assert(size < 32); 
  m_size = size;
  m_bits = 0;
  m_mask = ~((~0) << m_size);
  assert(m_mask != 0);
  assert((m_bits & m_mask) == m_bits);  // check for any bits outside the range
}

void Set::print(ostream& out) const
{
  out << "[Set (" << m_size << ") ";
  
  for (int i=0; i<m_size; i++) {
    out << (bool) isElement(i) << " ";
  }
  out << "]";
}
