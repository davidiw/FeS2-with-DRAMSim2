// ----------------------------------------------------------------------
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
// ----------------------------------------------------------------------

//===-- Event.h - event queues -----------------------------------*- C++ -*--=//
//
//! 
//
//===----------------------------------------------------------------------===//

#ifndef __EVENT_H
#define __EVENT_H

#include "Waiter.h"
#include "globals.h"
#include <vector>

typedef W64 Tick;
class Cycleable;

/*****************************************************************/
/************************* Events ********************************/
/*****************************************************************/

class Event : public WaitList {
public:
  Event() :
    WaitList(), m_prev_event(0), m_next_event(0), m_cycle(0) {};

  Event *getNextEvent() const { return m_next_event; }
  void setNextEvent(Event *event) { m_next_event = event; }

  Event *getPrevEvent() const { return m_prev_event; }
  void setPrevEvent(Event *event) { m_prev_event = event; }

  Tick getCycle() const { return m_cycle; }
  void setCycle(Tick cycle) { m_cycle = cycle; }

  void reset() { m_prev_event = 0; m_next_event = 0; m_cycle = 0; }
  void release();

private:
  Event *m_prev_event, *m_next_event;
  Tick m_cycle;
};

/*****************************************************************/
/************************* Event Queue ***************************/
/*****************************************************************/

class EventsQueue {
public:
  EventsQueue() : m_head(0), m_tail(0), m_current_cycle(0) {};

  void insert(Waiter *w, Tick delta_c);
  void insertAbsolute(Waiter *w, Tick c);
  void doCycle();
  void clear();
  void print();
  bool empty() { return (m_head == 0); }
  void addCycleable(Cycleable *c);

  Tick getCurrentCycle() { return m_current_cycle; }
  void resetCurrentCycle() { m_current_cycle = 0; }

  void release() {}

private:
  Event *m_head, *m_tail;
  Tick m_current_cycle;

  std::vector<Cycleable *> m_cycleables;
  
  static WaiterFreeList s_event_pool;
};

#endif // __EVENT_H
