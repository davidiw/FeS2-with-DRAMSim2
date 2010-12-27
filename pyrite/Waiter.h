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

//===-- Waiter.h - waiter and wait_list base classes -------------*- C++ -*--=//
//
//! A commonly used base class, used for managing objects that wait
//! for things.  Includes a variation that maintains a wait list
//! ordered by priority (this is mainly used by the scheduler).  Also,
//! includes WaiterFreeList which can be used to manage allocation pools
//! of objects that have Waiter as a base type.
//
//===----------------------------------------------------------------------===//

#ifndef __WAITER_H
#define __WAITER_H

#include "globals.h"

class WaitList;
class OrderedWaitList;
class OutorderScheduler;
class MemoryRequestRetry;

class Waiter { /* intrusive, doubly-linked list */
  friend class WaitList;
  friend class OrderedWaitList;
  friend class OutorderScheduler;
  friend class MemoryRequestRetry;
public:
  Waiter() { m_next = this; m_prev = this; }
  virtual ~Waiter();
  
  // bool Unique() { return (!Detached() && (m_next == m_prev)); }
  bool detached() const { return ((m_next == this) && (m_prev == this)); }
  bool waiting() const { return !detached(); }

  void detach() {
    m_next->m_prev = m_prev; m_prev->m_next = m_next;
    m_next = this; m_prev = this;
  }

  /* redefined by classes that want to use prioritized wait lists */
  virtual W64 priority() { return 0; }

  /* these two methods are to be defined by subclasses */
  virtual void release() { };
  virtual void wakeup() { };

protected:

  //! inserts "this" after "l"
  void insert(Waiter *l) { 
    m_next = l->m_next; m_prev = l; assert(m_next->m_prev == l);
    l->m_next = this; m_next->m_prev = this;
  }
  Waiter *getNextWaiter() const { return m_next; }
  Waiter *getPrevWaiter() const { return m_prev; }

  Waiter *m_next, *m_prev;

private:
  //! Disallow copying a waiter
  Waiter(const Waiter &w) { m_next = this; m_prev = this; }
};

class WaitList : public Waiter {
public:
  WaitList() : Waiter() { }
  virtual ~WaitList();
  
  bool empty() const { return detached(); }

  virtual void release() { assert(0); }
  virtual void wakeup() { assert(0); }

  //! inserts "waiter" at the end of the "this" wait list
  void insertWaiter(Waiter *waiter);

  void wakeupAll();
  void releaseAll();

  void printWaiters() const;

private:
  //! disallow copying a waitlist
  WaitList(const WaitList &w) : Waiter() {}
};

class OrderedWaitList : public WaitList {
public:
  //! inserts "waiter" according to its priority
  void insertWaiter(Waiter *waiter);

  void verify();
};

typedef Waiter *(*WaiterAllocator) ();

class WaiterFreeList : public WaitList {
public:
  WaiterFreeList(WaiterAllocator walloc) {
    allocator = walloc;
  }
  void insert(Waiter *w) { insertWaiter(w); }

  Waiter *get() {
    if (!empty()) {
      Waiter *w = m_prev;
      w->detach();
      return w;
    }
    ++ num_allocated;
    return (*allocator)();
  }

private:
  int num_allocated;
  WaiterAllocator allocator;
};

#endif // __WAITER_H
