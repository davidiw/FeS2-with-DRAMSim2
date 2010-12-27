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

#include "Event.h"
#include "Cycleable.h"

/*****************************************************************/
/************************* Event Queue ***************************/
/*****************************************************************/

Waiter *eventAllocator() { return new Event; }
WaiterFreeList EventsQueue::s_event_pool(eventAllocator);

void
Event::release() {
  assert(detached());
  assert(m_next_event == 0);
  assert(m_prev_event == 0);
  assert(m_cycle == 0);
}

void 
EventsQueue::doCycle() {
  /* do all of the events which are supposed to be done this cycle -
     they all should be in a single event */
  ++m_current_cycle;
  assert((m_head == NULL) || (m_current_cycle <= m_head->getCycle()));
  if (m_head && (m_head->getCycle() == m_current_cycle)) {
    Event *temp = m_head;
    m_head = m_head->getNextEvent();
    if (m_head) { m_head->setPrevEvent(0); }
    else { m_tail = 0; }
    temp->wakeupAll();
    temp->reset();
    s_event_pool.insert(temp);
  }
  for (std::vector<Cycleable *>::iterator i = m_cycleables.begin() ; i != m_cycleables.end() ; ++ i) {
    (*i)->cycle();
  }
}

void 
EventsQueue::insert(Waiter *w, Tick delta_c) {
  insertAbsolute(w, m_current_cycle + delta_c);
}

void 
EventsQueue::insertAbsolute(Waiter *w, Tick c) {
  assert(c > m_current_cycle);
  
  /* we tend to Insert events into the future so traverse the list
     backwards starting at the end */
  Event *trav = m_tail;
  while (trav && (trav->getCycle() >= c)) {
    if (trav->getCycle() == c) { 
      trav->insertWaiter(w); 
      return;
    }
    /* the current event should be inserted before the "trav" event */
    trav = trav->getPrevEvent();
  }
  /* do the actual insertion, since we are inserting after trav, so
     trav is our prev */
  Event *new_event = dynamic_cast<Event *>(s_event_pool.get());
  assert(new_event);
  new_event->setCycle(c);
  new_event->insertWaiter(w);
  new_event->setPrevEvent(trav);

  if (trav) { /* inserted somewhere in the list */
    new_event->setNextEvent(trav->getNextEvent());
    trav->setNextEvent(new_event); 
  } else { /* if we should be inserted at the head of the list */
    new_event->setNextEvent(m_head);
    m_head = new_event; 
  }

  if (!new_event->getNextEvent()) { /* at the end of the list */
    m_tail = new_event;
  } else { /* there is someone after me who should point to me */
    new_event->getNextEvent()->setPrevEvent(new_event);
  }
}

void
EventsQueue::clear() {
  while (m_head != NULL) {
    Event *temp = m_head;
    m_head = m_head->getNextEvent();
    if (m_head) { m_head->setPrevEvent(0); }
    else { m_tail = 0; }
    temp->releaseAll();
    temp->reset();
    s_event_pool.insert(temp);
  }
}

void 
EventsQueue::print() {
  Event *trav = m_head;
  while (trav) {
    printf("%d:", (int)trav->getCycle());
    trav->printWaiters();
    printf("\n");
    trav = trav->getNextEvent();
  }
}

void 
EventsQueue::addCycleable(Cycleable *c) {
  m_cycleables.push_back(c);
}
