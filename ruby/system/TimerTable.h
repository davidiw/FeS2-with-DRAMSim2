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

#ifndef TIMERTABLE_H
#define TIMERTABLE_H

#include "Global.h"
#include "Map.h"
#include "Address.h"
class Consumer;

class TimerTable {
public:

  // Constructors
  TimerTable(NodeID id);

  // Destructor
  //~TimerTable();

  // Public Methods
  void setConsumer(Consumer* consumer_ptr) { ASSERT(m_consumer_ptr==NULL); m_consumer_ptr = consumer_ptr; }
  void setDescription(const string& name) { m_name = name; }

  bool isReady() const;
  const Address& readyAddress() const;
  bool isSet(const Address& address) const { return m_map.exist(address); }
  void set(const Address& address, Time relative_latency);
  void unset(const Address& address);

  void printConfig(ostream& out) {}
  void print(ostream& out) const;
private:
  // Private Methods
  void updateNext() const;

  // Private copy constructor and assignment operator
  TimerTable(const TimerTable& obj);
  TimerTable& operator=(const TimerTable& obj);
  
  // Data Members (m_prefix)
  Map<Address, Time> m_map;
  mutable bool m_next_valid;
  mutable Time m_next_time; // Only valid if m_next_valid is true
  mutable Address m_next_address;  // Only valid if m_next_valid is true
  Consumer* m_consumer_ptr;  // Consumer to signal a wakeup()
  string m_name;
};

// ******************* Definitions *******************

// Output operator definition
extern inline 
ostream& operator<<(ostream& out, const TimerTable& obj)
{
  obj.print(out);
  out << flush;
  return out;
}
#endif //TIMERTABLE_H
