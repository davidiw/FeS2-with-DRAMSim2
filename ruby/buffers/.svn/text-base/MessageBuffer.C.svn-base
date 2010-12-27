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

#include "MessageBuffer.h"
#include "Param.h"

MessageBuffer::MessageBuffer()
{ 
  m_msg_counter = 0;
  m_consumer_ptr = NULL;
  m_ordering_set = false;
  m_strict_fifo = true;
  m_size = 0;
  m_max_size = -1;
  m_last_arrival_time = 0;
  m_randomization = true;
  m_size_last_time_size_checked = 0;
  m_time_last_time_size_checked = 0;
}

MessageBuffer::MessageBuffer(NodeID id)  // The NodeID is ignored, but could be used for extra debugging
{ 
  m_msg_counter = 0;
  m_consumer_ptr = NULL;
  m_ordering_set = false;
  m_strict_fifo = true;
  m_size = 0;
  m_max_size = -1;
  m_last_arrival_time = 0;
  m_randomization = true;
  m_size_last_time_size_checked = 0;
  m_time_last_time_size_checked = 0;
}

int MessageBuffer::getSize()
{
  if(m_time_last_time_size_checked == g_eventQueue_ptr->getTime()){
    return m_size_last_time_size_checked;
  } else {
    m_time_last_time_size_checked = g_eventQueue_ptr->getTime();
    m_size_last_time_size_checked = m_size;
    return m_size;
  }
}

bool MessageBuffer::areNSlotsAvailable(int n) const
{ 
  if(m_max_size == -1 || m_size+n <= m_max_size){ 
    return true; 
  } else {
    DEBUG_MSG(QUEUE_COMP,MedPrio,n);
    DEBUG_MSG(QUEUE_COMP,MedPrio,m_size);
    DEBUG_MSG(QUEUE_COMP,MedPrio,m_max_size);
    return false;
  }
}

const Message* MessageBuffer::peekAtHeadOfQueue() const
{
  const Message* msg_ptr;
  DEBUG_NEWLINE(QUEUE_COMP,MedPrio);
    
  DEBUG_MSG(QUEUE_COMP,MedPrio,"Peeking at head of queue " + m_name + " time: " 
            + int_to_string(g_eventQueue_ptr->getTime()) + ".");
  assert(isReady());

  msg_ptr = m_prio_heap.peekMin().m_msgptr.ref();
  assert(msg_ptr != NULL);

  DEBUG_EXPR(QUEUE_COMP,MedPrio,*msg_ptr);
  DEBUG_NEWLINE(QUEUE_COMP,MedPrio);
  return msg_ptr;
}

// FIXME - move me somewhere else
int random_time() 
{
  int time = 1;
  time += random() & 0x3;  // [0...3]
  if ((random() & 0x7) == 0) {  // 1 in 8 chance
    time += 100 + (random() % 0xf); // 100 + [1...15]
  }
  return time;
}

void MessageBuffer::enqueue(const MsgPtr& message, Time delta)
{
  DEBUG_NEWLINE(QUEUE_COMP,HighPrio);
  DEBUG_MSG(QUEUE_COMP,HighPrio,"enqueue " + m_name + " time: " 
            + int_to_string(g_eventQueue_ptr->getTime()) + ".");
  DEBUG_EXPR(QUEUE_COMP,MedPrio,message);
  DEBUG_NEWLINE(QUEUE_COMP,HighPrio);

  m_msg_counter++;
  m_size++;

  //  ASSERT(m_max_size == -1 || m_size <= m_max_size + 1);  
  // the plus one is a kluge because of a SLICC issue

  if (m_consumer_ptr == NULL) {
    WARN_EXPR(*this);
    WARN_EXPR(m_name);
    ERROR_MSG("No consumer set for this queue");
  }

  if (!m_ordering_set) {
    WARN_EXPR(*this);
    WARN_EXPR(m_name);
    ERROR_MSG("Ordering property of this queue has not been set");
  }

  // Calculate the arrival time of the message, that is, the first
  // cycle the message can be dequeued.
  assert(delta>0);
  Time current_time = g_eventQueue_ptr->getTime();
  Time arrival_time = 0;
  if (!g_param_ptr->RANDOMIZATION() || (m_randomization == false)) {
    // No randomization
    arrival_time = current_time + delta;
  } else {
    // Randomization - ignore delta
    if (m_strict_fifo) {
      if (m_last_arrival_time < current_time) {
        m_last_arrival_time = current_time;
      }
      arrival_time = m_last_arrival_time + random_time();
    } else {
      arrival_time = current_time + random_time();
    }
  }

  // Check the arrival time
  assert(arrival_time > current_time);
  if (m_strict_fifo) {
    if (arrival_time >= m_last_arrival_time) {
      
    } else {
      WARN_EXPR(*this);
      WARN_EXPR(m_name);
      WARN_EXPR(current_time);
      WARN_EXPR(delta);
      WARN_EXPR(arrival_time);
      WARN_EXPR(m_last_arrival_time);
      ERROR_MSG("FIFO ordering violated");
    }
  }
  m_last_arrival_time = arrival_time;

  // Insert the message into the priority heap
  MessageBufferNode thisNode(arrival_time, m_msg_counter, message);
  m_prio_heap.insert(thisNode);

  DEBUG_NEWLINE(QUEUE_COMP,HighPrio);
  DEBUG_MSG(QUEUE_COMP,HighPrio,"enqueue " + m_name 
            + " with arrival_time " + int_to_string(arrival_time)
            + " cur_time: " + int_to_string(g_eventQueue_ptr->getTime()) + ".");
  DEBUG_EXPR(QUEUE_COMP,MedPrio,message);
  DEBUG_NEWLINE(QUEUE_COMP,HighPrio);

  // Schedule the wakeup
  g_eventQueue_ptr->scheduleEventAbsolute(m_consumer_ptr, arrival_time);
}

void MessageBuffer::dequeue(MsgPtr& message)
{ 
  DEBUG_MSG(QUEUE_COMP,MedPrio,"dequeue from " + m_name);
  message = m_prio_heap.peekMin().m_msgptr;
  pop();
  DEBUG_EXPR(QUEUE_COMP,MedPrio,message);
}

void MessageBuffer::pop()
{
  DEBUG_MSG(QUEUE_COMP,MedPrio,"pop from " + m_name);
  assert(isReady());
  Time ready_time = m_prio_heap.extractMin().m_time;
  m_size--;
}

void MessageBuffer::clear()
{
  while(m_prio_heap.size() > 0){
    m_prio_heap.extractMin();
  }

  ASSERT(m_prio_heap.size() == 0);

  m_msg_counter = 0;
  m_size = 0;
}

void MessageBuffer::recycle()
{
  const int RECYCLE_LATENCY = 3;
  DEBUG_MSG(QUEUE_COMP,MedPrio,"recycling " + m_name);
  assert(isReady());
  MessageBufferNode node = m_prio_heap.extractMin();
  node.m_time = g_eventQueue_ptr->getTime() + RECYCLE_LATENCY;
  m_prio_heap.insert(node);
  g_eventQueue_ptr->scheduleEventAbsolute(m_consumer_ptr, g_eventQueue_ptr->getTime() + RECYCLE_LATENCY);
}

void MessageBuffer::print(ostream& out) const
{
  //  out << "[MessageBuffer: FIXME]";
  out << "[MessageBuffer: ";
  if (m_consumer_ptr != NULL) {
    out << " consumer-yes ";
  }
  out << m_prio_heap << "] " << m_name << endl;
}
