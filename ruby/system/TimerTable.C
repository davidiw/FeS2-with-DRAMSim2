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

#include "Global.h"
#include "TimerTable.h"
#include "EventQueue.h"

TimerTable::TimerTable(NodeID id)
{
  m_consumer_ptr  = NULL;
  m_next_valid = false;
  m_next_address = Address(0);
  m_next_time = 0;
}

bool TimerTable::isReady() const
{
  if (m_map.size() == 0) {
    return false;
  } 

  if (!m_next_valid) {
    updateNext();
  }
  assert(m_next_valid);
  return (g_eventQueue_ptr->getTime() >= m_next_time);
}

const Address& TimerTable::readyAddress() const
{
  assert(isReady());

  if (!m_next_valid) {
    updateNext();
  }
  assert(m_next_valid);
  return m_next_address;
}

void TimerTable::set(const Address& address, Time relative_latency)
{
  assert(address == line_address(address));
  assert(relative_latency > 0);
  assert(m_map.exist(address) == false);
  Time ready_time = g_eventQueue_ptr->getTime() + relative_latency;
  m_map.add(address, ready_time);
  assert(m_consumer_ptr != NULL);
  g_eventQueue_ptr->scheduleEventAbsolute(m_consumer_ptr, ready_time);
  m_next_valid = false;

  // Don't always recalculate the next ready address
  if (ready_time <= m_next_time) {
    m_next_valid = false;
  }
}

void TimerTable::unset(const Address& address)
{
  assert(address == line_address(address));
  assert(m_map.exist(address) == true);
  m_map.remove(address);

  // Don't always recalculate the next ready address
  if (address == m_next_address) {
    m_next_valid = false;
  }
}

void TimerTable::print(ostream& out) const
{
}


void TimerTable::updateNext() const
{
  if (m_map.size() == 0) {
    assert(m_next_valid == false);
    return;
  }

  Vector<Address> addresses = m_map.keys();
  m_next_address = addresses[0];
  m_next_time = m_map.lookup(m_next_address);

  // Search for the minimum time
  int size = addresses.size();
  for (int i=1; i<size; i++) {
    Address maybe_next_address = addresses[i];
    Time maybe_next_time = m_map.lookup(maybe_next_address);
    if (maybe_next_time < m_next_time) {
      m_next_time = maybe_next_time;
      m_next_address= maybe_next_address;
    }
  }
  m_next_valid = true;
}
