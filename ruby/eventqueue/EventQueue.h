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
 * Description: The EventQueue class implements an event queue which
 * can be trigger events, allowing our simulation to be event driven.  
 *
 * Currently, the only event we support is a Consumer being signaled
 * by calling the consumer's wakeup() routine.  Adding the event to
 * the queue does not require a virtual function call, though calling
 * wakeup() is a virtual function call.  
 *
 * The method triggerEvents() is called with a global time.  All
 * events which are before or at this time are triggered in timestamp
 * order.  No ordering is enforced for events scheduled to occur at
 * the same time.  Events scheduled to wakeup the same consumer at the
 * same time are combined into a single event.
 *
 * The method scheduleConsumerWakeup() is called with a global time
 * and a consumer pointer.  The event queue will call the wakeup()
 * method of the consumer at the appropriate time.
 *
 * This implementation of EventQueue uses a dynamically sized array
 * managed as a heap.  The algorithms used has O(lg n) for insert and
 * O(lg n) for extract minimum element. (Based on chapter 7 of Cormen,
 * Leiserson, and Rivest.)  The array is dynamically sized and is
 * automatically doubled in size when necessary.
 *
 */

#ifndef EVENTQUEUE_H
#define EVENTQUEUE_H

#include "Global.h"
#include "Vector.h"

class Consumer;
template <class TYPE> class PrioHeap;
class EventQueueNode;

class EventQueue {
public:
  // Constructors
  EventQueue(); 

  // Destructor
  ~EventQueue();
  
  // Public Methods
  Time getTime() const { return m_globalTime; }
  void scheduleEvent(Consumer* consumer, Time timeDelta) { scheduleEventAbsolute(consumer, timeDelta + m_globalTime); }
  void scheduleEventAbsolute(Consumer* consumer, Time timeAbs);
  void triggerEvents(Time t); // called to handle all events <= time t
  void triggerAllEvents();
  void print(ostream& out) const;
  bool isEmpty() const;

  // Private Methods

private:
  // Private copy constructor and assignment operator
  void init();
  EventQueue(const EventQueue& obj);
  EventQueue& operator=(const EventQueue& obj);
  
  // Data Members (m_ prefix)
  PrioHeap<EventQueueNode>* m_prio_heap_ptr;
  Time m_globalTime;
};

// Output operator declaration
inline extern 
ostream& operator<<(ostream& out, const EventQueue& obj);

// ******************* Definitions *******************

// Output operator definition
inline extern 
ostream& operator<<(ostream& out, const EventQueue& obj)
{
  obj.print(out);
  out << flush;
  return out;
}

#endif //EVENTQUEUE_H
