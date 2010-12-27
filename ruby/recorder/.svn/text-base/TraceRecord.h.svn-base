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
 * Description: A entry in the cache request record. It is aware of
 *              the ruby time and can issue the request back to the
 *              cache.
 *
 */

#ifndef TRACERECORD_H
#define TRACERECORD_H

#include "Global.h"
#include "Address.h"
#include "NodeID.h"
#include "CacheRequestType.h"
class CacheMsg;

class TraceRecord {
public:
  // Constructors
  TraceRecord(NodeID id, const Address& data_addr, const Address& pc_addr, CacheRequestType type, Time time);
  TraceRecord() { m_node_num = 0; m_time = 0; m_type = CacheRequestType_NULL; }

  // Destructor
  //  ~TraceRecord();
  
  // Public copy constructor and assignment operator
  TraceRecord(const TraceRecord& obj);
  TraceRecord& operator=(const TraceRecord& obj);

  // Public Methods
  bool node_less_then_eq(const TraceRecord& rec) const { return (this->m_time <= rec.m_time); }
  void issueRequest() const;

  void print(ostream& out) const;
  void output(ostream& out) const;
  bool input(istream& in);
private:
  // Private Methods
  
  // Data Members (m_ prefix)
  NodeID m_node_num;
  Time m_time;
  Address m_data_address;
  Address m_pc_address;
  CacheRequestType m_type;
};

inline extern bool node_less_then_eq(const TraceRecord& n1, const TraceRecord& n2);

// Output operator declaration
ostream& operator<<(ostream& out, const TraceRecord& obj);

// ******************* Definitions *******************

inline extern
bool node_less_then_eq(const TraceRecord& n1, const TraceRecord& n2)
{
  return n1.node_less_then_eq(n2);
}

// Output operator definition
extern inline 
ostream& operator<<(ostream& out, const TraceRecord& obj)
{
  obj.print(out);
  out << flush;
  return out;
}

#endif //TRACERECORD_H
