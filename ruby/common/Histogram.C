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

#include "Histogram.h"

Histogram::Histogram(int binsize, int bins)
{
  m_binsize = binsize;
  m_bins = bins;
  clear();
}

Histogram::~Histogram()
{
}

void Histogram::clear(int bins)
{
  m_bins = bins;
  m_largest_bin = 0;
  m_max = 0;
  m_data.setSize(m_bins);
  for (int i = 0; i < m_bins; i++) {
    m_data[i] = 0;
  }
  m_count = 0;
  m_max = 0;

  m_sumSamples = 0;
  m_sumSquaredSamples = 0;
}


void Histogram::add(int64 value)
{
  assert(value >= 0);
  m_max = max(m_max, value);
  m_count++;

  m_sumSamples += value;
  m_sumSquaredSamples += (value*value);

  int index;
  if (m_binsize == -1) {
    // This is a log base 2 histogram
    if (value == 0) {
      index = 0;
    } else {
      index = int(log(double(value))/log(2.0))+1;
    }
  } else {
    // This is a linear histogram
    while (m_max >= (m_bins * m_binsize)) {
      for (int i = 0; i < m_bins/2; i++) {
        m_data[i] = m_data[i*2] + m_data[i*2 + 1];
      }
      for (int i = m_bins/2; i < m_bins; i++) {
        m_data[i] = 0;
      }
      m_binsize *= 2;
    }
    index = value/m_binsize;
  }
  assert(index >= 0);
  m_data[index]++;
  m_largest_bin = max(m_largest_bin, index);
}

// Computation of standard deviation of samples a1, a2, ... aN
// variance = [SUM {ai^2} - (SUM {ai})^2/N]/(N-1)
// std deviation equals square root of variance
double Histogram::getStandardDeviation() const
{
  double variance;
  if(m_count > 1){
    variance = (double)(m_sumSquaredSamples - m_sumSamples*m_sumSamples/m_count)/(m_count - 1);
  } else {
    return 0;
  }
  return sqrt(variance);
}

void Histogram::print(ostream& out) const
{
  printWithMultiplier(out, 1.0);
}

void Histogram::printPercent(ostream& out) const
{
  if (m_count == 0) {
    printWithMultiplier(out, 0.0);
  } else {
    printWithMultiplier(out, 100.0/double(m_count));
  }
}

void Histogram::printWithMultiplier(ostream& out, double multiplier) const
{
  if (m_binsize == -1) {
    out << "[binsize: log2 ";
  } else {
    out << "[binsize: " << m_binsize << " ";
  }
  out << "max: " << m_max << " ";
  out << "count: " << m_count << " ";
  //  out << "total: " <<  m_sumSamples << " ";
  if (m_count == 0) {
    out << "average: NaN |";
    out << "standard deviation: NaN |";
  } else {
    out << "average: " << setw(5) << ((double) m_sumSamples)/m_count << " | ";
    out << "standard deviation: " << getStandardDeviation() << " |";
  }
  for (int i = 0; i < m_bins && i <= m_largest_bin; i++) {
    if (multiplier == 1.0) {
      out << " " << m_data[i];
    } else {
      out << " " << double(m_data[i]) * multiplier;
    }
  }
  out << " ]";
}

bool node_less_then_eq(const Histogram* n1, const Histogram* n2)
{
  return (n1->size() > n2->size());
}
