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

#include "CacheProfiler.h"
#include "System.h"
#include "Profiler.h"

CacheProfiler::CacheProfiler(const string& description)
  : m_requestSize(-1)
{
  m_description = description;
  m_accessModeType_vec.setSize(AccessModeType_NUM);
  m_requestType_vec.setSize(CacheRequestType_NUM);
  clearStats();
}

CacheProfiler::~CacheProfiler()
{
}

void CacheProfiler::printStats(ostream& out) const
{
  out << m_description << " cache stats: " << endl;
  string description = "  " + m_description;

  out << description << "_total_misses: " << m_misses << endl;
  out << description << "_total_prefetches: " << m_prefetches << endl;

  double inst_executed = double(g_system_ptr->getProfiler()->getTotalInstructionsExecuted());

  out << description << "_misses_per_instruction: " << double(m_misses) / inst_executed << endl;
  out << description << "_instructions_per_misses: ";
  if (m_misses > 0) {
    out << inst_executed / double(m_misses) << endl;
  } else {
    out << "NaN" << endl;
  }
  out << endl;

  int requests = 0;
  for(int i=0; i<CacheRequestType_NUM; i++) {
    requests += m_requestType_vec[i];
  }
  
  assert(m_misses == requests);

  if (requests > 0) {
    for(int i=0; i<CacheRequestType_NUM; i++) {
      out << description << "_request_type_" << CacheRequestType(i) << ":   " << m_requestType_vec[i]
          << "    " << (100.0 * m_requestType_vec[i]) / double(requests) << "%" << endl;
    }
    
    out << endl;
    
    for(int i=0; i<AccessModeType_NUM; i++){
      out << description << "_access_mode_type_" << AccessModeType(i) << ":   " << m_accessModeType_vec[i] 
          << "    " << (100.0 * m_accessModeType_vec[i]) / double(requests) << "%" << endl;
    }
  }

  out << description << "_request_size: " << m_requestSize << endl;
  out << endl;

}

void CacheProfiler::clearStats()
{
  m_requestSize.clear();
  m_misses = 0;
  m_prefetches = 0;
  for(int i=0; i<AccessModeType_NUM; i++) {
    m_accessModeType_vec[i] = 0;
  }
  for(int i=0; i<CacheRequestType_NUM; i++) {
    m_requestType_vec[i] = 0;
  }
}

void CacheProfiler::addStatSample(CacheRequestType request_type, AccessModeType type, int msgSize, PrefetchBit pfBit)
{
  m_misses++;
  m_requestType_vec[request_type]++;
  m_accessModeType_vec[type]++;
  m_requestSize.add(msgSize);
  if (pfBit == PrefetchBit_Yes) {
    m_prefetches++;
  }
}

