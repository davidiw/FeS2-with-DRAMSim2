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
 * Description: Perfect switch, of course it is perfect and no latency or what
 *              so ever. Every cycle it is woke up and perform all the
 *              necessary routings that must be done. Note, this switch also
 *              has number of input ports/output ports and has a routing table
 *              as well.
 *
 */

#ifndef PerfectSwitch_H
#define PerfectSwitch_H

#include "Global.h"
#include "Vector.h"
#include "Consumer.h"
#include "NodeID.h"

class MessageBuffer;
class NetDest;
class SimpleNetwork;

class LinkOrder {
public:
  int m_link;
  int m_value;
};

class PerfectSwitch : public Consumer {
public:
  // Constructors

  // constructor specifying the number of ports
  PerfectSwitch(SwitchID sid, SimpleNetwork* network_ptr);
  void addInPort(const Vector<MessageBuffer*>& in);
  void addOutPort(const Vector<MessageBuffer*>& out, const NetDest& routing_table_entry);
  int getInLinks() const { return m_in.size(); }
  int getOutLinks() const { return m_out.size(); }

  // Destructor
  ~PerfectSwitch();
  
  // Public Methods
  void wakeup();

  void printStats(ostream& out) const;
  void clearStats();
  void printConfig(ostream& out) const;

  void print(ostream& out) const;
private:

  // Private copy constructor and assignment operator
  PerfectSwitch(const PerfectSwitch& obj);
  PerfectSwitch& operator=(const PerfectSwitch& obj);
  
  // Data Members (m_ prefix)
  SwitchID m_switch_id;
  
  // vector of queues from the components
  Vector<Vector<MessageBuffer*> > m_in;
  Vector<Vector<MessageBuffer*> > m_out;
  Vector<NetDest> m_routing_table;
  Vector<LinkOrder> m_link_order;
  int m_virtual_networks;
  int m_round_robin_start;
  SimpleNetwork* m_network_ptr;
};

// Output operator declaration
ostream& operator<<(ostream& out, const PerfectSwitch& obj);

// ******************* Definitions *******************

// Output operator definition
extern inline 
ostream& operator<<(ostream& out, const PerfectSwitch& obj)
{
  obj.print(out);
  out << flush;
  return out;
}

#endif //PerfectSwitch_H
