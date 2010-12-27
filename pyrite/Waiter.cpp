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

//===-- Waiter.cpp - waiter and wait_list base classes -----------*- C++ -*--=//
//
//! A commonly used base class, used for managing objects that wait
//! for things.  Includes a variation that maintains a wait list
//! ordered by priority (this is mainly used by the scheduler).  Also,
//! includes WaiterFreeList which can be used to manage allocation pools
//! of objects that have Waiter as a base type.
//
//===----------------------------------------------------------------------===//

#include "Waiter.h"

Waiter::~Waiter() {
  if ((m_next != this) || (m_prev != this)) {
    assert ((m_next != this) && (m_prev != this));
    detach();
  }
}

WaitList::~WaitList() {
  if ((m_next != this) || (m_prev != this)) {
    assert ((m_next != this) && (m_prev != this));
    releaseAll();
  }
  assert(empty());
}

//! inserts "waiter" at the end of the "this" wait list
void 
WaitList::insertWaiter(Waiter *waiter) { 
  assert(waiter->detached());
  waiter->m_next = this;
  waiter->m_prev = m_prev;
  assert(m_prev->m_next == this);
  m_prev->m_next = waiter;
  m_prev = waiter;
}

void 
WaitList::wakeupAll() {
  while(!empty()) {
    Waiter *w = m_next;
    w->detach();
    w->wakeup();
  }
}

void 
WaitList::releaseAll() {
  while(!empty()) {
    Waiter *w = m_next;
    w->detach();
    w->release();
  }
}

void 
WaitList::printWaiters() const {
  for (Waiter *wtrav = m_next ; wtrav != (Waiter *)this ; wtrav = wtrav->getNextWaiter()) {
    printf(" %p", wtrav);
  }
}

void 
OrderedWaitList::insertWaiter(Waiter *waiter) {
  assert(waiter->detached());
  if (empty()) {
    WaitList::insertWaiter(waiter);
    return;
  }
  
  W64 priority = waiter->priority();
  for (Waiter *wtrav = m_prev ; wtrav != this ; wtrav = wtrav->getPrevWaiter()) {
    if (wtrav->priority() <= priority) {
      waiter->insert(wtrav);
      return;
    }
  }
  waiter->insert(this);
}

void 
OrderedWaitList::verify() {
  W64 last_value = 0;
  for (Waiter *wtrav = m_next ; wtrav != this ; wtrav = wtrav->getNextWaiter()) {
    W64 p = wtrav->priority();
    assert(p >= last_value);
    last_value = p;
  }
}
