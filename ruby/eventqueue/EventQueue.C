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

#include "EventQueue.h"
#include "Consumer.h"
#include "Profiler.h"
#include "System.h"
#include "PrioHeap.h"
#include "EventQueueNode.h"

// Class public method definitions

EventQueue::EventQueue()
{
  m_prio_heap_ptr = NULL;
  init();
}

EventQueue::~EventQueue()
{
  delete m_prio_heap_ptr;
}

void EventQueue::init()
{
  m_globalTime = 1;
  m_prio_heap_ptr = new PrioHeap<EventQueueNode>;
  m_prio_heap_ptr->init();
}

bool EventQueue::isEmpty() const 
{
  return (m_prio_heap_ptr->size() == 0);
}

void EventQueue::scheduleEventAbsolute(Consumer* consumer, Time timeAbs)
{
  // Check to see if this is a redundant wakeup
  //  Time time = timeDelta + m_globalTime;
  ASSERT(consumer != NULL);
  if (consumer->getLastScheduledWakeup() != timeAbs) {
    // This wakeup is not redundant
    EventQueueNode thisNode;
    thisNode.m_consumer_ptr = consumer;
    assert(timeAbs > m_globalTime);
    thisNode.m_time = timeAbs;
    m_prio_heap_ptr->insert(thisNode);
    consumer->setLastScheduledWakeup(timeAbs);
  }
}

void EventQueue::triggerEvents(Time t) 
{
  EventQueueNode thisNode;

  while(m_prio_heap_ptr->size() > 0 && m_prio_heap_ptr->peekMin().m_time <= t) {
    m_globalTime = m_prio_heap_ptr->peekMin().m_time;
    thisNode = m_prio_heap_ptr->extractMin();
    assert(thisNode.m_consumer_ptr != NULL);
    DEBUG_EXPR(EVENTQUEUE_COMP,MedPrio,*(thisNode.m_consumer_ptr));
    DEBUG_EXPR(EVENTQUEUE_COMP,MedPrio,thisNode.m_time);
    thisNode.m_consumer_ptr->triggerWakeup();
  }
  m_globalTime = t;
}

void EventQueue::triggerAllEvents()
{
  // FIXME - avoid repeated code
  EventQueueNode thisNode;

  while(m_prio_heap_ptr->size() > 0) {
    m_globalTime = m_prio_heap_ptr->peekMin().m_time;
    thisNode = m_prio_heap_ptr->extractMin();
    assert(thisNode.m_consumer_ptr != NULL);
    DEBUG_EXPR(EVENTQUEUE_COMP,MedPrio,*(thisNode.m_consumer_ptr));
    DEBUG_EXPR(EVENTQUEUE_COMP,MedPrio,thisNode.m_time);
    thisNode.m_consumer_ptr->triggerWakeup();
  }
}

// Class private method definitions

void 
EventQueue::print(ostream& out) const
{
  out << "[Event Queue: " << *m_prio_heap_ptr << "]";
}
