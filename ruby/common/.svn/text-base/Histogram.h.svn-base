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

/*
 * Description: The histogram class implements a simple histogram
 * 
 */

#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include "Global.h"
#include "Vector.h"

class Histogram {
public:
  // Constructors
  Histogram(int binsize = 1, int bins = 50);

  // Destructor
  ~Histogram();
  
  // Public Methods

  void add(int64 value);
  void clear() { clear(m_bins); }
  void clear(int bins);
  int64 size() const { return m_count; }
  int64 getTotal() const { return m_sumSamples; }

  void printWithMultiplier(ostream& out, double multiplier) const;
  void printPercent(ostream& out) const;
  void print(ostream& out) const;
private:
  // Private Methods

  // Private copy constructor and assignment operator
  //  Histogram(const Histogram& obj);
  //  Histogram& operator=(const Histogram& obj);
  
  // Data Members (m_ prefix)
  Vector<int64> m_data;
  int64 m_max;          // the maximum value seen so far
  int64 m_count;                // the number of elements added
  int m_binsize;                // the size of each bucket
  int m_bins;           // the number of buckets
  int m_largest_bin;      // the largest bin used

  int64 m_sumSamples;   // the sum of all samples
  int64 m_sumSquaredSamples; // the sum of the square of all samples

  double getStandardDeviation() const;
};

bool node_less_then_eq(const Histogram* n1, const Histogram* n2);

// Output operator declaration
ostream& operator<<(ostream& out, const Histogram& obj);

// ******************* Definitions *******************

// Output operator definition
extern inline 
ostream& operator<<(ostream& out, const Histogram& obj)
{
  obj.print(out);
  out << flush;
  return out;
}

#endif //HISTOGRAM_H
