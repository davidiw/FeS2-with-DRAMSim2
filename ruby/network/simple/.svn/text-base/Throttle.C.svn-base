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
 * Description: see Throttle.h
 *
 */

#include "Throttle.h"
#include "MessageBuffer.h"
#include "Network.h"
#include "System.h"
#include "NetworkMessage.h"
#include "Param.h"

const int HIGH_RANGE = 256;
const int ADJUST_INTERVAL = 50000;
const int MESSAGE_SIZE_MULTIPLIER = 1000;

static int network_message_to_size(NetworkMessage* net_msg_ptr);

Throttle::Throttle(int sID, NodeID node, int link_latency, int link_bandwidth_multiplier)
{
  m_sID = sID;
  m_node = node;
  m_vnets = 0;
  m_last_vnet = 0;

  assert(link_bandwidth_multiplier > 0);
  m_link_bandwidth_multiplier = link_bandwidth_multiplier;
  m_link_latency = link_latency;

  m_bash_counter = HIGH_RANGE;
  m_bandwidth_since_sample = 0;
  m_last_bandwidth_sample = 0;
  clearStats();
}

void Throttle::clear()
{
  for (int counter = 0; counter < m_vnets; counter++) {
    m_in[counter]->clear();
    m_out[counter]->clear();
  }
}

void Throttle::addLinks(const Vector<MessageBuffer*>& in_vec, const Vector<MessageBuffer*>& out_vec)
{
  assert(in_vec.size() == out_vec.size());
  for (int i=0; i<in_vec.size(); i++) {
    addVirtualNetwork(in_vec[i], out_vec[i]);
  }

  m_message_counters.setSize(MessageSizeType_NUM);
  for (int i=0; i<MessageSizeType_NUM; i++) {
    m_message_counters[i].setSize(in_vec.size());
    for (int j=0; j<m_message_counters[i].size(); j++) {
      m_message_counters[i][j] = 0;
    }
  }
}

void Throttle::addVirtualNetwork(MessageBuffer* in_ptr, MessageBuffer* out_ptr)
{
  m_units_remaining.insertAtBottom(0);
  m_in.insertAtBottom(in_ptr);
  m_out.insertAtBottom(out_ptr);
  
  // Set consumer and description
  m_in[m_vnets]->setConsumer(this);
  string desc = "[Queue to Throttle " + NodeIDToString(m_sID) + " " + NodeIDToString(m_node) + "]";
  m_in[m_vnets]->setDescription(desc);
  m_vnets++;
}

int Throttle::getLinkBandwidth() const
{
  return g_param_ptr->LINK_BANDWIDTH() * m_link_bandwidth_multiplier; 
}

void Throttle::wakeup()
{
  // Limits the number of message sent to a limited number of bytes/cycle.
  assert(getLinkBandwidth() > 0);
  int bw_remaining = getLinkBandwidth();

  // Look at each virtual network
  for (int counter = 0; counter < m_vnets; counter++) {

    // Does the link have any bandwidth remaining this cycle?
    if (bw_remaining == 0) {
      break;
    }

    // Rotate the priority by starting with the vnet at which we left off
    int vnet = m_last_vnet;

    // Increment the last vnet counter to rotate the priority
    m_last_vnet++;
    if (m_last_vnet >= m_vnets) {  // check for wrap around
      m_last_vnet = 0;
    }

    assert(m_out[vnet] != NULL);
    assert(m_in[vnet] != NULL);
    assert(m_units_remaining[vnet] >= 0);
    
    while ((bw_remaining > 0) && ((m_in[vnet]->isReady()) || (m_units_remaining[vnet] > 0)) && m_out[vnet]->areNSlotsAvailable(1)) {
      
      // See if we are done transferring the previous message on this virtual network
      if (m_units_remaining[vnet] == 0 && m_in[vnet]->isReady()) {
        
        // Find the size of the message we are moving
        MsgPtr msg_ptr = m_in[vnet]->peekMsgPtr();
        NetworkMessage* net_msg_ptr = dynamic_cast<NetworkMessage*>(msg_ptr.ref());
        m_units_remaining[vnet] += network_message_to_size(net_msg_ptr);
        // Move the message
        m_out[vnet]->enqueue(m_in[vnet]->peekMsgPtr(), m_link_latency);
        m_in[vnet]->pop();

        // Count the message
        m_message_counters[net_msg_ptr->getMessageSize()][vnet]++;
      }

      // Calculate the amount of bandwidth we spent on this message
      int diff = m_units_remaining[vnet] - bw_remaining;
      m_units_remaining[vnet] = max(0, diff);
      bw_remaining = max(0, -diff);
    }
  }
  
  // We should only wake up when we use the bandwidth
  //  assert(bw_remaining != getLinkBandwidth());  // This is only mostly true

  // Record that we used some or all of the link bandwidth this cycle
  double ratio = 1.0-(double(bw_remaining)/double(getLinkBandwidth()));
  // If ratio = 0, we used no bandwidth, if ratio = 1, we used all
  linkUtilized(ratio);

  // Sample the link bandwidth utilization over a number of cycles
  int bw_used = getLinkBandwidth()-bw_remaining;
  m_bandwidth_since_sample += bw_used;

  // Update the predictor
  Time current_time = g_eventQueue_ptr->getTime();
  while ((current_time - m_last_bandwidth_sample) > ADJUST_INTERVAL) {
    double utilization = m_bandwidth_since_sample/double(ADJUST_INTERVAL * getLinkBandwidth());

    if (utilization > g_param_ptr->BASH_BANDWIDTH_ADAPTIVE_THRESHOLD()) {
      // Used more bandwidth
      m_bash_counter++;
    } else {
      // Used less bandwidth
      m_bash_counter--;
    }

    // Make sure we don't overflow
    m_bash_counter = min(HIGH_RANGE, m_bash_counter);
    m_bash_counter = max(0, m_bash_counter);

    // Reset samples
    m_last_bandwidth_sample += ADJUST_INTERVAL;
    m_bandwidth_since_sample = 0;
  }
  
  if (bw_remaining > 0) {
    // We have extra bandwidth, we must not have anything else to do until another message arrives.
  } else {
    // We are out of bandwidth for this cycle, so wakeup next cycle and continue
    g_eventQueue_ptr->scheduleEvent(this, 1);
  }
}

bool Throttle::broadcastBandwidthAvailable(int rand) const
{
  bool result =  !(m_bash_counter > ((HIGH_RANGE/4) + (rand % (HIGH_RANGE/2))));
  return result;
}

void Throttle::printStats(ostream& out) const
{
  out << "utilized_percent: " << getUtilization() << endl;  
}

void Throttle::clearStats()
{
  m_ruby_start = g_eventQueue_ptr->getTime();
  m_links_utilized = 0.0;

  for (int i=0; i<m_message_counters.size(); i++) {
    for (int j=0; j<m_message_counters[i].size(); j++) {
      m_message_counters[i][j] = 0;
    }
  }
}

void Throttle::printConfig(ostream& out) const
{
  
}

double Throttle::getUtilization() const
{
  return (100.0 * double(m_links_utilized)) / (double(g_eventQueue_ptr->getTime()-m_ruby_start));
}

void Throttle::print(ostream& out) const
{
  out << "[Throttle: " << m_sID << " " << m_node << " bw: " << getLinkBandwidth() << "]";
}

// Helper function

static 
int network_message_to_size(NetworkMessage* net_msg_ptr)
{
  assert(net_msg_ptr != NULL);
  return (MessageSizeType_to_int(net_msg_ptr->getMessageSize()) * MESSAGE_SIZE_MULTIPLIER);
}
