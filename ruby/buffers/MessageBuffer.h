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

#ifndef MESSAGEBUFFER_H
#define MESSAGEBUFFER_H

#include "Global.h"
#include "MessageBufferNode.h"
#include "Consumer.h"
#include "EventQueue.h"
#include "Message.h"
#include "PrioHeap.h"
#include "NodeID.h"
#include "util.h"

class MessageBuffer {
public:
  // Constructors
  MessageBuffer();
  MessageBuffer(NodeID id); // The NodeID is ignored, but could be used for extra debugging

  // Use Default Destructor 
  // ~MessageBuffer()
  
  // Public Methods
  void printConfig(ostream& out) {}

  // TRUE if head of queue timestamp <= SystemTime
  bool isReady() const { 
    return ((m_prio_heap.size() > 0) && 
            (m_prio_heap.peekMin().m_time <= g_eventQueue_ptr->getTime()));
  }

  bool areNSlotsAvailable(int n) const; 
  void setConsumer(Consumer* consumer_ptr) { ASSERT(m_consumer_ptr==NULL); m_consumer_ptr = consumer_ptr; }
  void setDescription(const string& name) { m_name = name; }
  string getDescription() { return m_name;}

  Consumer* getConsumer() { return m_consumer_ptr; }

  const Message* peekAtHeadOfQueue() const;
  const Message* peek() const { return peekAtHeadOfQueue(); }
  const MsgPtr& peekMsgPtr() const { assert(isReady()); return m_prio_heap.peekMin().m_msgptr; }
  const MsgPtr& peekMsgPtrEvenIfNotReady() const {return m_prio_heap.peekMin().m_msgptr; }

  void enqueue(const MsgPtr& message) { enqueue(message, 1); }
  void enqueue(const MsgPtr& message, Time delta);
  void dequeue(MsgPtr& message);
  void dequeue() { pop(); }
  void pop();
  void recycle();
  bool isEmpty() const { return m_prio_heap.size() == 0; }
  
  void setOrdering(bool order) { m_strict_fifo = order; m_ordering_set = true; }
  void setSize(int size) {m_max_size = size;}
  int getSize();
  void setRandomization(bool random_flag) { m_randomization = random_flag; }

  void clear();

  void print(ostream& out) const;
private:
  // Private Methods  

  // Private copy constructor and assignment operator
  MessageBuffer(const MessageBuffer& obj);
  MessageBuffer& operator=(const MessageBuffer& obj);

  // Data Members (m_ prefix)
  Consumer* m_consumer_ptr;  // Consumer to signal a wakeup(), can be NULL
  PrioHeap<MessageBufferNode> m_prio_heap;
  string m_name;

  int m_max_size;
  int m_size;

  Time m_time_last_time_size_checked;
  int m_size_last_time_size_checked;

  int m_msg_counter;
  bool m_strict_fifo;
  bool m_ordering_set;
  bool m_randomization;
  Time m_last_arrival_time;
};

// Output operator declaration
ostream& operator<<(ostream& out, const MessageBuffer& obj);

// ******************* Definitions *******************

// Output operator definition
extern inline 
ostream& operator<<(ostream& out, const MessageBuffer& obj)
{
  obj.print(out);
  out << flush;
  return out;
}

#endif //MESSAGEBUFFER_H
