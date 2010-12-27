/*
 * Copyright (c) 1999 by Mark Hill and David Wood for the Wisconsin
 * Multifacet Project.  ALL RIGHTS RESERVED.
 *
 * ##HEADER##
 *
 * This software is furnished under a license and may be used and
 * copied only in accordance with the terms of such license and the
 * inclusion of the above copyright notice.  This software or any
 * other copies thereof or any derivative works may not be provided or
 * otherwise made available to any other persons.  Title to and
 * ownership of the software is retained by Mark Hill and David Wood.
 * Any use of this software must include the above copyright notice.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS".  THE LICENSOR MAKES NO
 * WARRANTIES ABOUT ITS CORRECTNESS OR PERFORMANCE.
 * */
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
 * Topology.h
 *
 * Description: The topology here is configurable; it can be a
 * hierachical switch or a 2D torus. Basically, the class has a vector
 * of nodes and edges. First 2*m_nodes elements in the node vector are
 * input and output ports. Edges are represented in two vectors of src
 * and dest nodes.
 *
 * */

#ifndef TOPOLOGY_H
#define TOPOLOGY_H

#include "Global.h"
#include "Vector.h"
#include "NodeID.h"

class SimpleNetwork;
class NetDest;

class Topology {
public:
  // Constructors
  Topology(SimpleNetwork* network_ptr, int number_of_nodes);

  // Destructor
  ~Topology() {}

  // Public Methods
  int numSwitches() const { return m_number_of_switches; }
  void createLinks();

  void printStats(ostream& out) const {}
  void clearStats() {}
  void printConfig(ostream& out) const {}
  void print(ostream& out) const { out << "[Topology]"; }

private:
  // Private Methods
  void init();
  SwitchID newSwitchID();
  void addLink(SwitchID src, SwitchID dest, int link_latency);
  void addLink(SwitchID src, SwitchID dest, int link_latency, int bw_multiplier);
  void makeLink(SwitchID src, SwitchID dest, const NetDest& routing_table_entry, int link_latency, int bw_multiplier);

  void makeHierarchicalSwitch(Vector<SwitchID> network_in_switches, Vector<SwitchID> network_out_switches, int fan_out_degree);
  void make2DTorus(Vector<SwitchID> network_in_switches, Vector<SwitchID> network_out_switches);
  // Private copy constructor and assignment operator
  Topology(const Topology& obj);
  Topology& operator=(const Topology& obj);

  // Data Members (m_ prefix)
  SimpleNetwork* m_network_ptr;
  NodeID m_nodes;
  int m_number_of_switches;

  Vector<SwitchID> m_links_src_vector;
  Vector<SwitchID> m_links_dest_vector;
  Vector<int> m_links_latency_vector;
  Vector<int> m_bw_multiplier_vector;
};

// Output operator declaration
ostream& operator<<(ostream& out, const Topology& obj);

// ******************* Definitions *******************

// Output operator definition
extern inline
ostream& operator<<(ostream& out, const Topology& obj)
{
  obj.print(out);
  out << flush;
  return out;
}

#endif
