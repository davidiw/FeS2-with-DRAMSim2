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

#include "AccessTraceForAddress.h"
#include "Histogram.h"

AccessTraceForAddress::AccessTraceForAddress()
{
  m_histogram_ptr = NULL;
}

AccessTraceForAddress::AccessTraceForAddress(const Address& addr)
{
  m_addr = addr;
  m_total = 0;
  m_loads = 0;
  m_stores = 0;
  m_atomics = 0;
  m_user = 0;
  m_sharing = 0;
  m_histogram_ptr = NULL;
}

AccessTraceForAddress::~AccessTraceForAddress()
{
  if (m_histogram_ptr != NULL) {
    delete m_histogram_ptr;
    m_histogram_ptr = NULL;
  }
}

void AccessTraceForAddress::print(ostream& out) const
{
  out << m_addr;

  if (m_histogram_ptr == NULL) {
    out << " " << m_total;
    out << " | " << m_loads;
    out << " " << m_stores;
    out << " " << m_atomics;
    out << " | " << m_user;
    out << " " << m_total-m_user;
    out << " | " << m_sharing;
    out << " | " << m_touched_by.count();
  } else {
    assert(m_total == 0);
    out << " " << (*m_histogram_ptr);
  }
}

void AccessTraceForAddress::update(CacheRequestType type, AccessModeType access_mode, NodeID cpu, bool sharing_miss)
{
  m_touched_by.add(cpu);
  m_total++;
  if(type == CacheRequestType_ATOMIC) {
    m_atomics++;
  } else if(type == CacheRequestType_LD){
    m_loads++;
  } else if (type == CacheRequestType_ST){
    m_stores++;
  } else {
    //  ERROR_MSG("Trying to add invalid access to trace");
  }
  
  if (access_mode == AccessModeType_UserMode) {
    m_user++;
  }

  if (sharing_miss) {
    m_sharing++;
  }
}

int AccessTraceForAddress::getTotal() const
{
  if (m_histogram_ptr == NULL) {
    return m_total;
  } else {
    return m_histogram_ptr->getTotal();
  }
}

void AccessTraceForAddress::addSample(int value)
{
  assert(m_total == 0); 
  if (m_histogram_ptr == NULL) {
    m_histogram_ptr = new Histogram;
  }
  m_histogram_ptr->add(value);
}

bool node_less_then_eq(const AccessTraceForAddress* n1, const AccessTraceForAddress* n2)
{
  return (n1->getTotal() > n2->getTotal());
}
