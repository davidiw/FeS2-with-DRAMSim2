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

#ifndef SET_H
#define SET_H

#include "Global.h"
#include "Vector.h"
#include "NodeID.h"

class Set {
public:
  // Constructors
  // creates and empty set
  Set();

  //  Set(const Set& obj);
  //  Set& operator=(const Set& obj);

  // Destructor
  // ~Set();
  
  // Public Methods

  void add(NodeID newElement);
  void addSet(const Set& set);
  void addRandom();
  void remove(NodeID newElement);
  void removeSet(const Set& set);
  void clear();
  void broadcast();
  int count() const;
  bool isEqual(const Set& set);

  Set OR(const Set& orSet) const;  // return the logical OR of this set and orSet
  Set AND(const Set& andSet) const;  // return the logical AND of this set and andSet

  // Returns true if the intersection of the two sets is non-empty
  bool intersectionIsNotEmpty(const Set& other_set) const;

  // Returns true if the intersection of the two sets is empty
  bool intersectionIsEmpty(const Set& other_set) const;

  bool isSuperset(const Set& test) const;
  bool isSubset(const Set& test) const { return test.isSuperset(*this); }
  bool isElement(NodeID element) const;
  bool isBroadcast() const;
  bool isEmpty() const;

  NodeID smallestElement() const;
  int getSize() const { return m_size; }

  void print(ostream& out) const;
private:
  // Private Methods
  void setSize (int size);
  Set(int size);

  // Data Members (m_ prefix)
  int m_size;
  uint32 m_bits;  // Set as a bit vector
  uint32 m_mask;  // a 000001111 mask where the number of 1s is equal to m_size
};

// Output operator declaration
ostream& operator<<(ostream& out, const Set& obj);

// ******************* Definitions *******************

// Output operator definition
extern inline 
ostream& operator<<(ostream& out, const Set& obj)
{
  obj.print(out);
  out << flush;
  return out;
}

#endif //BIGSET
