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

#include "Topology.h"
#include "NetDest.h"
#include "SimpleNetwork.h"
#include "TopologyType.h"
#include "util.h"
#include "MachineType.h"
#include "Param.h"

static const int INFINITE_LATENCY = 10000; // Yes, this is a big hack
static const int DEFAULT_BW_MULTIPLIER = 1;  // Just to be consistent with above :)

// Note: In this file, we use the first 2*m_nodes SwitchIDs to
// represent the input and output endpoint links.  These really are
// not 'switches', as they will not have a Switch object allocated for
// them. The first m_nodes SwitchIDs are the links into the network,
// the second m_nodes set of SwitchIDs represent the the output queues
// of the network.

// Helper functions based on chapter 29 of Cormen et al.
typedef Vector<Vector<int> > Matrix;
static Matrix extend_shortest_path(const Matrix& current_dist);
static Matrix shortest_path(const Matrix& weights);
static bool link_is_shortest_path_to_node(SwitchID src, SwitchID next, SwitchID final, const Matrix& weights, const Matrix& dist);
static NetDest shortest_path_to_node(SwitchID src, SwitchID next, const Matrix& weights, const Matrix& dist, int nodes);


Topology::Topology(SimpleNetwork* network_ptr, int number_of_nodes)
{
  m_network_ptr = network_ptr;
  m_nodes = number_of_nodes;
  m_number_of_switches = 0;
  init();
}

void Topology::init()
{ 
  assert(m_nodes >= MachineType_NUM);
  if (m_nodes == MachineType_NUM) {
    SwitchID id = newSwitchID();
    for (int machine=0; machine<MachineType_NUM; machine++) {
      addLink(machine, id, g_param_ptr->NETWORK_LINK_LATENCY());
      addLink(id, m_nodes+machine, g_param_ptr->NETWORK_LINK_LATENCY());
    }
    return;
  }

  Vector<SwitchID> network_in_switches;
  Vector<SwitchID> network_out_switches;
  for (int node = 0; node < m_nodes; node++) {
    network_in_switches.insertAtBottom(node);          // numbered [0...m_nodes-1]
    network_out_switches.insertAtBottom(node+m_nodes); // numbered [m_nodes-1...m_nodes+m_nodes-1]
  }

  // topology-specific set-up
  switch (g_param_ptr->NETWORK_TOPOLOGY()) {
  case TopologyType_TORUS_2D:
    make2DTorus(network_in_switches, network_out_switches);
    break;
  case TopologyType_HIERARCHICAL_SWITCH:
    makeHierarchicalSwitch(network_in_switches, network_out_switches, g_param_ptr->FAN_OUT_DEGREE());
    break;
  case TopologyType_CROSSBAR:
    makeHierarchicalSwitch(network_in_switches, network_out_switches, 1024);
    break;
  default:
    ERROR_MSG("Unexpected typology type")
  }
}

// 2D torus topology

void Topology::make2DTorus(Vector<SwitchID> network_in_switches, Vector<SwitchID> network_out_switches)
{
  int lengthOfSide = (int)sqrt((double)m_nodes);
  Vector<SwitchID> torusSwitches;

  for(int i=0; i<m_nodes; i++){
    SwitchID new_switch = newSwitchID();
    torusSwitches.insertAtBottom(new_switch);
  }

  for(int i=0; i<m_nodes; i++){
    SwitchID new_switch = torusSwitches[i];
    // add links between switch and node
    SwitchID port_into_network = network_in_switches[i];
    SwitchID port_from_network = network_out_switches[i];
    addLink(port_into_network, new_switch, g_param_ptr->NETWORK_LINK_LATENCY());

    // Latency of 1 because we're not crossing a chip boundary (on-chip switch to processor)
    // Bandwidth multiplier of 10 so this on-chip link to the processor isn't the bandwidth bottleneck
    addLink(new_switch, port_from_network, 1, 10); 

    // left
    SwitchID leftNeighbor;
    if(new_switch%lengthOfSide == 0){
      leftNeighbor = new_switch - 1 + lengthOfSide; 
    } else {
      leftNeighbor = new_switch - 1;
    }
    addLink(new_switch, leftNeighbor, g_param_ptr->NETWORK_LINK_LATENCY());
    // right
    SwitchID rightNeighbor;
    if((new_switch + 1)%lengthOfSide == 0){
      rightNeighbor = new_switch + 1 - lengthOfSide;
    } else { 
      rightNeighbor = new_switch + 1;
    }
    addLink(new_switch, rightNeighbor, g_param_ptr->NETWORK_LINK_LATENCY());
    // top
    SwitchID topNeighbor;
    if(new_switch - lengthOfSide < 2*m_nodes){
      topNeighbor = new_switch - lengthOfSide + (lengthOfSide*lengthOfSide);
    } else {
      topNeighbor = new_switch - lengthOfSide;
    }
    addLink(new_switch, topNeighbor, g_param_ptr->NETWORK_LINK_LATENCY());
    // bottom
    SwitchID bottomNeighbor;
    if(new_switch + lengthOfSide >= 3*m_nodes){  // sorin: bad bug if this is a > instead of a >=
      bottomNeighbor = new_switch + lengthOfSide - (lengthOfSide*lengthOfSide);
    } else {
      bottomNeighbor = new_switch + lengthOfSide;
    }
    addLink(new_switch, bottomNeighbor, g_param_ptr->NETWORK_LINK_LATENCY());
  }
}

// hierarchical switch topology

void Topology::makeHierarchicalSwitch(Vector<SwitchID> network_in_switches, Vector<SwitchID> network_out_switches, int fan_out_degree)
{
  // Make a row of switches with only one input.  This extra row makes
  // sure the links out of the nodes have latency and limited
  // bandwidth.
  Vector<SwitchID> last_level;
  int counter = 0;
  for (int i=0; i<(network_in_switches.size()/MachineType_NUM); i++) {
    SwitchID new_switch = newSwitchID();
    last_level.insertAtBottom(new_switch);

    // Add a link per machine
    for (int machine=0; machine<MachineType_NUM; machine++) {
      SwitchID port_into_network = network_in_switches[counter];
      counter++;
      addLink(port_into_network, new_switch, g_param_ptr->NETWORK_LINK_LATENCY());
    }
  }

  Vector<SwitchID> next_level;
  while(last_level.size() > 1) {
    for (int i=0; i<last_level.size(); i++) {
      if ((i % fan_out_degree) == 0) {
        next_level.insertAtBottom(newSwitchID());
      }
      // Add this link to the last switch we created
      addLink(last_level[i], next_level[next_level.size()-1], g_param_ptr->NETWORK_LINK_LATENCY());
    }
    
    // Make the current level the last level to get ready for next
    // iteration
    last_level = next_level;
    next_level.clear();
  }
  
  // Make the root switch
  SwitchID root_switch = last_level[0];
  
  // Build the down network from the endpoints to the root

  // Make a row of switches for endpoints
  last_level.clear();
  counter = 0;
  for (int j=0; j<(network_out_switches.size()/MachineType_NUM); j++) {
    SwitchID new_switch = newSwitchID();
    last_level.insertAtBottom(new_switch);

    // Add a link per machine
    for (int machine=0; machine<MachineType_NUM; machine++) {
      SwitchID port_from_network = network_out_switches[counter];
      counter++;
      addLink(new_switch, port_from_network, 1, 10);
    }
  }
  
  // Build the down network from the endpoints to the root
  next_level.clear();
  while(last_level.size() != 1) {
    
    // A level of switches
    NodeID node_counter = 0;
    for (int i=0; i<last_level.size(); i++) {
      if ((i % fan_out_degree) == 0) {
        if (last_level.size() > fan_out_degree) {
          next_level.insertAtBottom(newSwitchID());
        } else {
          next_level.insertAtBottom(root_switch);
        }
      }
      // Add this link to the last switch we created
      addLink(next_level[next_level.size()-1], last_level[i], g_param_ptr->NETWORK_LINK_LATENCY());
    }
    
    // Make the current level the last level to get ready for next
    // iteration
    last_level = next_level;
    next_level.clear();
  }
}

void Topology::createLinks()
{
  // Find maximum switchID

  SwitchID max_switch_id = 0;
  for (int i=0; i<m_links_src_vector.size(); i++) {
    max_switch_id = max(max_switch_id, m_links_src_vector[i]);
    max_switch_id = max(max_switch_id, m_links_dest_vector[i]);
  }

  // Initialize weight vector
  Matrix topology_weights;
  Matrix topology_bw_multis;
  int num_switches = max_switch_id+1;
  topology_weights.setSize(num_switches);
  topology_bw_multis.setSize(num_switches);
  for(int i=0; i<topology_weights.size(); i++) {
    topology_weights[i].setSize(num_switches);
    topology_bw_multis[i].setSize(num_switches);
    for(int j=0; j<topology_weights[i].size(); j++) {
      topology_weights[i][j] = INFINITE_LATENCY;
      topology_bw_multis[i][j] = -1;  // initialize to an invalid value
    }
  }

  // Set identity weights to zero
  for(int i=0; i<topology_weights.size(); i++) {
    topology_weights[i][i] = 0;
  }

  // Fill in the topology weights and bandwidth multipliers
  for (int i=0; i<m_links_src_vector.size(); i++) {
    topology_weights[m_links_src_vector[i]][m_links_dest_vector[i]] = m_links_latency_vector[i];
    topology_bw_multis[m_links_src_vector[i]][m_links_dest_vector[i]] = m_bw_multiplier_vector[i];
  }

  // Walk topology and hookup the links
  Matrix dist = shortest_path(topology_weights);
  for(int i=0; i<topology_weights.size(); i++) {
    for(int j=0; j<topology_weights[i].size(); j++) {
      int weight = topology_weights[i][j];
      if (weight > 0 && weight != INFINITE_LATENCY) {
        int bw_multiplier = topology_bw_multis[i][j];
        assert(bw_multiplier > 0);
        NetDest destination_set = shortest_path_to_node(i, j, topology_weights, dist, m_nodes);
        makeLink(i, j, destination_set, weight, bw_multiplier);
      }
    }
  }
}

SwitchID Topology::newSwitchID()
{
  m_number_of_switches++;
  return m_number_of_switches-1+m_nodes+m_nodes;
}

void Topology::addLink(SwitchID src, SwitchID dest, int link_latency)
{
  addLink(src, dest, link_latency, DEFAULT_BW_MULTIPLIER);
}

void Topology::addLink(SwitchID src, SwitchID dest, int link_latency, int bw_multiplier)
{
  ASSERT(src <= m_number_of_switches+m_nodes+m_nodes);
  ASSERT(dest <= m_number_of_switches+m_nodes+m_nodes);
  m_links_src_vector.insertAtBottom(src);
  m_links_dest_vector.insertAtBottom(dest);
  m_links_latency_vector.insertAtBottom(link_latency);
  m_bw_multiplier_vector.insertAtBottom(bw_multiplier);
}

void Topology::makeLink(SwitchID src, SwitchID dest, const NetDest& routing_table_entry, int link_latency, int bw_multiplier)
{
  // Make sure we're not trying to connect two end-point nodes directly together
  assert((src >= 2*m_nodes) || (dest >= 2*m_nodes));

  if (src < m_nodes) {
    m_network_ptr->makeInLink(src, dest-(2*m_nodes), routing_table_entry, link_latency, bw_multiplier);    
  } else if (dest < 2*m_nodes) {
    assert(dest >= m_nodes);
    NodeID node = dest-m_nodes;
    m_network_ptr->makeOutLink(src-(2*m_nodes), node, routing_table_entry, link_latency, bw_multiplier);
  } else {
    assert((src >= 2*m_nodes) && (dest >= 2*m_nodes));
    m_network_ptr->makeInternalLink(src-(2*m_nodes), dest-(2*m_nodes), routing_table_entry, link_latency, bw_multiplier);
  }
}

/**************************************************************************/

// The following all-pairs shortest path algorithm is based on the
// discussion from Cormen et al., Chapter 26.1.

static void extend_shortest_path(Matrix& current_dist)
{
  bool change = true;
  int nodes = current_dist.size();

  while (change) {
    change = false;
    for (int i=0; i<nodes; i++) {
      for (int j=0; j<nodes; j++) {
        int minimum = current_dist[i][j];
        for (int k=0; k<nodes; k++) {
          minimum = min(minimum, current_dist[i][k] + current_dist[k][j]);   
        }
        if (current_dist[i][j] != minimum) {
          change = true;
          current_dist[i][j] = minimum;
        }
      }
    }
  }
}

static Matrix shortest_path(const Matrix& weights)
{
  Matrix dist = weights;
  extend_shortest_path(dist);    
  return dist;
}

static bool link_is_shortest_path_to_node(SwitchID src, SwitchID next, SwitchID final, 
                                          const Matrix& weights, const Matrix& dist)
{
  return (weights[src][next] + dist[next][final] == dist[src][final]);
}

static NetDest shortest_path_to_node(SwitchID src, SwitchID next,
                                     const Matrix& weights, const Matrix& dist, 
                                     int nodes)
{
  NetDest result;
  for (int i=0; i<nodes; i++) {
    // we use "i+m_nodes" below since the "destination" switches for the
    // nodes are numbered [nodes...nodes+nodes-1]
    if (link_is_shortest_path_to_node(src, next, i+nodes, weights, dist)) { 
      result.add(MachineType(i % MachineType_NUM), i / MachineType_NUM);
    }
  }
  return result;
}
