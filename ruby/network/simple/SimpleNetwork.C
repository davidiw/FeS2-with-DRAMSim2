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

#include "SimpleNetwork.h"
#include "Profiler.h"
#include "System.h"
#include "Switch.h"
#include "NetDest.h"
#include "Topology.h"
#include "TopologyType.h"
#include "MachineType.h"
#include "MessageBuffer.h"
#include "Map.h"
#include "Param.h"

SimpleNetwork::SimpleNetwork()
{
  m_nodes = g_param_ptr->NUM_NODES() * MachineType_NUM;
  m_virtual_networks = g_param_ptr->NUMBER_OF_VIRTUAL_NETWORKS();
  m_endpoint_switches.setSize(m_nodes);

  m_in_use.setSize(m_virtual_networks);
  m_ordered.setSize(m_virtual_networks);
  for (int i = 0; i < m_virtual_networks; i++) {
    m_in_use[i] = false;
    m_ordered[i] = false;
  }
  
  // Allocate to and from queues
  m_toNetQueues.setSize(m_nodes);
  m_fromNetQueues.setSize(m_nodes);
  for (int node = 0; node < m_nodes; node++) {
    m_toNetQueues[node].setSize(m_virtual_networks);
    m_fromNetQueues[node].setSize(m_virtual_networks);
    for (int j = 0; j < m_virtual_networks; j++) {
      m_toNetQueues[node][j] = new MessageBuffer;
      m_toNetQueues[node][j]->setDescription("[Queue to network, vnet=" + int_to_string(j) + ", port=" + int_to_string(node) + "]");
      m_fromNetQueues[node][j] = new MessageBuffer;
      m_fromNetQueues[node][j]->setDescription("[Queue from network, vnet=" + int_to_string(j) + ", port=" + int_to_string(node) + "]");
    }
  }

  // Setup the network switches
  m_topology_ptr = new Topology(this, m_nodes);
  int number_of_switches = m_topology_ptr->numSwitches();
  for (int i=0; i<number_of_switches; i++) {
    m_switch_ptr_vector.insertAtBottom(new Switch(i, this));
  }
  m_topology_ptr->createLinks();
}

SimpleNetwork::~SimpleNetwork()
{
  for (int i = 0; i < m_nodes; i++) {
    m_toNetQueues[i].deletePointers();
    m_fromNetQueues[i].deletePointers();
  }
  m_switch_ptr_vector.deletePointers();
  m_buffers_to_free.deletePointers();
  delete m_topology_ptr;
}

// From a switch to an endpoint node
void SimpleNetwork::makeOutLink(SwitchID src, NodeID dest, const NetDest& routing_table_entry, int link_latency, int bw_multiplier)
{
  assert(dest < m_nodes);
  assert(src < m_switch_ptr_vector.size());
  assert(m_switch_ptr_vector[src] != NULL);
  m_switch_ptr_vector[src]->addOutPort(m_fromNetQueues[dest], routing_table_entry, link_latency, bw_multiplier);
  m_endpoint_switches[dest] = m_switch_ptr_vector[src];
}

// From an endpoint node to a switch
void SimpleNetwork::makeInLink(NodeID src, SwitchID dest, const NetDest& routing_table_entry, int link_latency, int bw_multiplier)
{
  assert(src < m_nodes);
  m_switch_ptr_vector[dest]->addInPort(m_toNetQueues[src]);
}

// From a switch to a switch
void SimpleNetwork::makeInternalLink(SwitchID src, SwitchID dest, const NetDest& routing_table_entry, int link_latency, int bw_multiplier)
{
  // Create a set of new MessageBuffers
  Vector<MessageBuffer*> queues;
  for (int i = 0; i < m_virtual_networks; i++) {
    // allocate a buffer
    MessageBuffer* buffer_ptr = new MessageBuffer;
    buffer_ptr->setOrdering(true);
    queues.insertAtBottom(buffer_ptr);
    // remember to deallocate it
    m_buffers_to_free.insertAtBottom(buffer_ptr);
  }
  
  // Connect it to the two switches
  m_switch_ptr_vector[dest]->addInPort(queues);
  m_switch_ptr_vector[src]->addOutPort(queues, routing_table_entry, link_latency, bw_multiplier);
}

void SimpleNetwork::checkNetworkAllocation(bool ordered, int network_num)
{
  ASSERT(network_num < m_virtual_networks);

  if (ordered) {
    m_ordered[network_num] = true;
  }
  m_in_use[network_num] = true;
}

MessageBuffer* SimpleNetwork::getToNetQueue(MachineType machine, NodeID id, bool ordered, int network_num)
{
  checkNetworkAllocation(ordered, network_num);
  int port_number = machine + (id * MachineType_NUM);
  return m_toNetQueues[port_number][network_num];
}

MessageBuffer* SimpleNetwork::getFromNetQueue(MachineType machine, NodeID id, bool ordered, int network_num)
{
  checkNetworkAllocation(ordered, network_num);
  int port_number = machine + (id * MachineType_NUM);
  return m_fromNetQueues[port_number][network_num];
}

const Vector<Throttle*>* SimpleNetwork::getThrottles(NodeID id) const
{
  assert(id >= 0);
  assert(id < m_nodes);
  assert(m_endpoint_switches[id] != NULL);
  return m_endpoint_switches[id]->getThrottles();
}

void SimpleNetwork::printStats(ostream& out) const
{
  out << endl;
  out << heading("Network Stats");
  out << endl;
  for(int i=0; i<m_switch_ptr_vector.size(); i++) {
    m_switch_ptr_vector[i]->printStats(out);
  }
}

void SimpleNetwork::clearStats()
{
  for(int i=0; i<m_switch_ptr_vector.size(); i++) {
    m_switch_ptr_vector[i]->clearStats();
  }
}

void SimpleNetwork::printConfig(ostream& out) const 
{
  out << endl;
  out << heading("Network Configuration");
  out << "network: SIMPLE_NETWORK" << endl;
  out << endl;

  for (int i = 0; i < m_virtual_networks; i++) {
    out << "virtual_net_" << i << ": ";
    if (m_in_use[i]) {
      out << "active, ";
      if (m_ordered[i]) {
        out << "ordered" << endl;
      } else {
        out << "unordered" << endl;
      }
    } else {
      out << "inactive" << endl;
    }
  }
  out << endl;
  for(int i=0; i<m_switch_ptr_vector.size(); i++) {
    m_switch_ptr_vector[i]->printConfig(out);
  }
}

void SimpleNetwork::print(ostream& out) const
{
  out << "[SimpleNetwork]";
}
