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
 
//===-- Scheduler.h - instruction scheduler ----------------------*- C++ -*--=//
//
//! Schedulers decide which ready instructions are executed on a given cycle.
//
//===----------------------------------------------------------------------===//

#ifndef __SCHEDULER_H
#define __SCHEDULER_H

#include "Waiter.h"

class Processor;
class DynamicInst;
class MemoryInterface;

//! an abstract base class
class Scheduler : public OrderedWaitList {
public:
  Scheduler(Processor *processor, MemoryInterface *mem_interface) :
    OrderedWaitList(), m_processor(processor), m_mem_interface(mem_interface) { }
  virtual ~Scheduler() {} 

  virtual void wakeup(DynamicInst *d);
  virtual void select() = 0;
protected:
  void selectInstruction(DynamicInst *d);

  Processor *m_processor;
  MemoryInterface *m_mem_interface;
};

//! schedules instructions in-order
class InorderScheduler : public Scheduler {
public:
  InorderScheduler(Processor *processor, MemoryInterface *mem_interface) :
    Scheduler(processor, mem_interface) {
    m_head = 0;
  }

  virtual void select();
  virtual void squash(QPointer first_bad);

private:
  QPointer m_head;
};

//! schedules instructions out-of-order
class OutorderScheduler : public Scheduler {
public:
  OutorderScheduler(Processor *processor, MemoryInterface *mem_interface) :
    Scheduler(processor, mem_interface) { }

  virtual void select();
  virtual void squash(QPointer first_bad) {}
};


#endif /* __SCHEDULER_H */
