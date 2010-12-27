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
 * Description: The class to implement bandwidth and latency throttle. An
 *              instance of consumer class that can be woke up. It is only used
 *              to control bandwidth at output port of a switch. And the
 *              throttle is added *after* the output port, means the message is
 *              put in the output port of the PerfectSwitch (a
 *              intermediateBuffers) first, then go through the Throttle.
 *
 */

#ifndef THROTTLE_H
#define THROTTLE_H

#include "Global.h"
#include "Vector.h"
#include "Consumer.h"
#include "NodeID.h"

class MessageBuffer;

class Throttle : public Consumer {
public:
  // Constructors
  Throttle(int sID, NodeID node, int link_latency, int link_bandwidth_multiplier);

  // Destructor
  ~Throttle() {}
  
  // Public Methods
  void addLinks(const Vector<MessageBuffer*>& in_vec, const Vector<MessageBuffer*>& out_vec);
  void wakeup();
  bool broadcastBandwidthAvailable(int rand) const;

  void printStats(ostream& out) const;
  void clearStats();
  void printConfig(ostream& out) const;
  double getUtilization() const; // The average utilization (a percent) since last clearStats()
  int getLatency() const { return m_link_latency; }
  const Vector<Vector<int> >& getCounters() const { return m_message_counters; }

  void clear();

  void print(ostream& out) const;

private:
  // Private Methods
  void addVirtualNetwork(MessageBuffer* in_ptr, MessageBuffer* out_ptr);
  int getLinkBandwidth() const;
  void linkUtilized(double ratio) { m_links_utilized += ratio; }

  // Private copy constructor and assignment operator
  Throttle(const Throttle& obj);
  Throttle& operator=(const Throttle& obj);
  
  // Data Members (m_ prefix)
  Vector<MessageBuffer*> m_in;
  Vector<MessageBuffer*> m_out;
  Vector<Vector<int> > m_message_counters;
  int m_vnets;
  int m_last_vnet;
  Vector<int> m_units_remaining;
  int m_sID;
  NodeID m_node;
  int m_bash_counter;
  int m_bandwidth_since_sample;
  Time m_last_bandwidth_sample;
  int m_link_bandwidth_multiplier;
  int m_link_latency;

  // For tracking utilization
  Time m_ruby_start;
  double m_links_utilized;
};

// Output operator declaration
ostream& operator<<(ostream& out, const Throttle& obj);

// ******************* Definitions *******************

// Output operator definition
extern inline 
ostream& operator<<(ostream& out, const Throttle& obj)
{
  obj.print(out);
  out << flush;
  return out;
}

#endif //THROTTLE_H
