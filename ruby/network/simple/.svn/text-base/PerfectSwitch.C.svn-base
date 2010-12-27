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


#include "PerfectSwitch.h"
#include "NetworkMessage.h"
#include "Profiler.h"
#include "System.h"
#include "SimpleNetwork.h"
#include "util.h"
#include "MessageBuffer.h"
#include "Param.h"

// Operator for helper class
bool operator<(const LinkOrder& l1, const LinkOrder& l2) {
  return (l1.m_value < l2.m_value);
}

PerfectSwitch::PerfectSwitch(SwitchID sid, SimpleNetwork* network_ptr)
{
  m_virtual_networks = g_param_ptr->NUMBER_OF_VIRTUAL_NETWORKS();
  m_switch_id = sid;
  m_round_robin_start = 0;
  m_network_ptr = network_ptr;
}

void PerfectSwitch::addInPort(const Vector<MessageBuffer*>& in)
{
  assert(in.size() == m_virtual_networks);
  NodeID port = m_in.size();
  m_in.insertAtBottom(in);
  for (int j = 0; j < m_virtual_networks; j++) {
    m_in[port][j]->setConsumer(this);
    string desc = "[Queue from port " +  NodeIDToString(m_switch_id) + " " + NodeIDToString(port) + " to PerfectSwitch]";
    m_in[port][j]->setDescription(desc);
  }
}

void PerfectSwitch::addOutPort(const Vector<MessageBuffer*>& out, const NetDest& routing_table_entry)
{
  assert(out.size() == m_virtual_networks);

  // Setup link order
  LinkOrder l;
  l.m_value = 0;
  l.m_link = m_out.size();
  m_link_order.insertAtBottom(l);

  // Add to routing table
  m_out.insertAtBottom(out);
  m_routing_table.insertAtBottom(routing_table_entry);
}

PerfectSwitch::~PerfectSwitch()
{
}

void PerfectSwitch::wakeup()
{
  DEBUG_EXPR(NETWORK_COMP, MedPrio, m_switch_id);

  MsgPtr msg_ptr;

  // Look at all nodes and route any waiting messages
  for (int vnet = 0; vnet < m_virtual_networks; vnet++) {
    // For all components incoming queues
    int incoming = m_round_robin_start; // This is for round-robin scheduling
    m_round_robin_start++;
    if (m_round_robin_start >= m_in.size()) {
      m_round_robin_start = 0;
    }

    // for all input ports, use round robin scheduling
    for (int counter = 0; counter < m_in.size(); counter++) {

      // Round robin scheduling
      incoming++;
      if (incoming >= m_in.size()) {
        incoming = 0;
      }

      // temporary vectors to store the routing results
      Vector<LinkID> output_links;
      Vector<NetDest> output_link_destinations;

      // Is there a message waiting?
      while (m_in[incoming][vnet]->isReady()) {
        DEBUG_EXPR(NETWORK_COMP, MedPrio, incoming);

        // Peek at message
        msg_ptr = m_in[incoming][vnet]->peekMsgPtr();
        NetworkMessage* net_msg_ptr = dynamic_cast<NetworkMessage*>(msg_ptr.ref());
        DEBUG_EXPR(NETWORK_COMP, MedPrio, *net_msg_ptr);

        output_links.clear();
        output_link_destinations.clear();
        NetDest msg_destinations = net_msg_ptr->getInternalDestination();

        // Unfortunately, the token-protocol sends some
        // zero-destination messages, so this assert isn't valid
        // assert(msg_destinations.count() > 0);

        assert(m_link_order.size() == m_routing_table.size());
        assert(m_link_order.size() == m_out.size());

        if (g_param_ptr->ADAPTIVE_ROUTING()) {
          if (m_network_ptr->isVNetOrdered(vnet)) {
            // Don't adaptively route
            for (int outlink=0; outlink<m_out.size(); outlink++) {
              m_link_order[outlink].m_link = outlink;
              m_link_order[outlink].m_value = 0;
            }
          } else {
            // Find how clogged each link is
            for (int outlink=0; outlink<m_out.size(); outlink++) {
              int out_queue_length = 0;
              for (int v=0; v<m_virtual_networks; v++) {
                out_queue_length += m_out[outlink][v]->getSize();
              }
              m_link_order[outlink].m_link = outlink;
              m_link_order[outlink].m_value = 0;
              m_link_order[outlink].m_value |= (out_queue_length << 8);
              m_link_order[outlink].m_value |= (random() & 0xff);
            }
            m_link_order.sortVector();  // Look at the most empty link first
          }
        }

        for (int i=0; i<m_routing_table.size(); i++) {
          // pick the next link to look at
          int link = m_link_order[i].m_link;

          if (msg_destinations.intersectionIsNotEmpty(m_routing_table[link])) {

            // Remember what link we're using
            output_links.insertAtBottom(link);
            
            // Need to remember which destinations need this message
            // in another vector.  This Set is the intersection of the
            // routing_table entry and the current destination set.
            // The intersection must not be empty, since we are inside "if"
            output_link_destinations.insertAtBottom(msg_destinations.AND(m_routing_table[link]));
            
            // Next, we update the msg_destination not to include
            // those nodes that were already handled by this link
            msg_destinations.removeNetDest(m_routing_table[link]);
          }
        }

        assert(msg_destinations.count() == 0);
        //assert(output_links.size() > 0);

        // Check for resources - for all outgoing queues
        bool enough = true;
        for (int i=0; i<output_links.size(); i++) {
          int outgoing = output_links[i];
          enough = enough && m_out[outgoing][vnet]->areNSlotsAvailable(1);
        }

        // There were not enough resources
        if(!enough) {
          g_eventQueue_ptr->scheduleEvent(this, 1);
          DEBUG_MSG(NETWORK_COMP, HighPrio, "Can't deliver message to anyone since a node is blocked");
          DEBUG_EXPR(NETWORK_COMP, HighPrio, incoming);
          break; // go to next incoming port
        }

        // Enqueue it - for all outgoing queues
        for (int i=0; i<output_links.size(); i++) {
          int outgoing = output_links[i];

          if (i > 0) {  
            // If we are sending this message down more than one link
            // (i>0), we need to make a copy of the message so each
            // branch can have a different internal destination
            msg_ptr = *(msg_ptr.ref());  // This magic line creates a private copy of the message
          }

          // Change the internal destination set of the message so it
          // knows which destinations this link is responsible for.
          NetworkMessage* net_msg_ptr = dynamic_cast<NetworkMessage*>(msg_ptr.ref());
          net_msg_ptr->getInternalDestination() = output_link_destinations[i];

          // Enqeue msg
          DEBUG_NEWLINE(NETWORK_COMP,HighPrio);
          DEBUG_MSG(NETWORK_COMP,HighPrio,"switch: " + int_to_string(m_switch_id)
                    + " enqueuing net msg from inport[" + int_to_string(incoming) + "][" 
                    + int_to_string(vnet) +"] to outport [" + int_to_string(outgoing) 
                    + "][" + int_to_string(vnet) +"]" 
                    + " time: " + int_to_string(g_eventQueue_ptr->getTime()) + ".");
          DEBUG_NEWLINE(NETWORK_COMP,HighPrio);
          m_out[outgoing][vnet]->enqueue(msg_ptr);
        }

        // Dequeue msg
        m_in[incoming][vnet]->pop();
      }
    }
  }
  //  g_eventQueue_ptr->scheduleEvent(this, 1);  // Just to be safe
}

void PerfectSwitch::printStats(ostream& out) const
{ 
}

void PerfectSwitch::clearStats()
{
}

void PerfectSwitch::printConfig(ostream& out) const
{ 
}

void PerfectSwitch::print(ostream& out) const
{
  out << "[PerfectSwitch " << m_switch_id << "]";
}

