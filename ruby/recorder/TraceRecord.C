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

#include "TraceRecord.h"
#include "Sequencer.h"
#include "System.h"
#include "CacheMsg.h"

TraceRecord::TraceRecord(NodeID id, const Address& data_addr, const Address& pc_addr, CacheRequestType type, Time time) 
{
  m_node_num = id;
  m_data_address = data_addr;
  m_pc_address = pc_addr;
  m_time = time;
  m_type = type;

  // Don't differentiate between store misses and atomic requests in
  // the trace
  if (m_type == CacheRequestType_ATOMIC) {
    m_type = CacheRequestType_ST;  
  }
}

// Public copy constructor and assignment operator
TraceRecord::TraceRecord(const TraceRecord& obj)
{
  *this = obj;  // Call assignment operator
}

TraceRecord& TraceRecord::operator=(const TraceRecord& obj)
{
  m_node_num = obj.m_node_num;
  m_time = obj.m_time;
  m_data_address = obj.m_data_address;
  m_pc_address = obj.m_pc_address;
  m_type = obj.m_type;
  return *this;
}

void TraceRecord::issueRequest() const
{
  // Lookup sequencer pointer from system
  Sequencer* sequencer_ptr = g_system_ptr->getSequencer(m_node_num);
  assert(sequencer_ptr != NULL);
    
  CacheMsg request(m_data_address, m_type, m_pc_address, AccessModeType_UserMode, 0, PrefetchBit_Yes);

  // Clear out the sequencer
  while (!sequencer_ptr->empty()) {
    g_eventQueue_ptr->triggerEvents(g_eventQueue_ptr->getTime() + 100);
  }

  sequencer_ptr->makeRequest(request);

  // Clear out the sequencer
  while (!sequencer_ptr->empty()) {
    g_eventQueue_ptr->triggerEvents(g_eventQueue_ptr->getTime() + 100);
  }
}

void TraceRecord::print(ostream& out) const
{
  out << "[TraceRecord: Node, " << m_node_num << ", " << m_data_address << ", " << m_pc_address << ", " << m_type << ", Time: " << m_time << "]";
}

void TraceRecord::output(ostream& out) const
{
  out << m_node_num << " ";
  m_data_address.output(out);
  out << " ";
  m_pc_address.output(out);
  out << " ";
  out << m_type;
  out << endl;
}

bool TraceRecord::input(istream& in)
{
  in >> m_node_num;
  m_data_address.input(in);
  m_pc_address.input(in);
  string type;
  if (!in.eof()) {
    in >> type;
    m_type = string_to_CacheRequestType(type);
    
    // Ignore the rest of the line
    char c = '\0';
    while ((!in.eof()) && (c != '\n')) {
      in.get(c);
    }
    
    return true;
  } else {
    return false;
  }
}
