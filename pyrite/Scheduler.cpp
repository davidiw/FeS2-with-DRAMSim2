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
//! The load queue and the store queue are used for two purposes: 1)
//! to synchronize loads with inflight stores and provide them data,
//! and 2) to detect load-store ordering violations.
//
//===----------------------------------------------------------------------===//

#include "Processor.h"
#include "Scheduler.h"
#include "QPointer.h"
#include "DynamicInst.h"
#include "LoadStoreQueue.h"
#include "MemoryInterface.h"

extern unsigned g_execute_width;
extern unsigned g_memory_issue_width;
extern unsigned g_store_issue_width;

/*****************************************************************/
/********************** Scheduler base class *********************/
/*****************************************************************/

void 
Scheduler::wakeup(DynamicInst *d) { 
  insertWaiter(d); 
}

void 
Scheduler::selectInstruction(DynamicInst *d) {
  d->detach();
  d->beginExecution();
  int latency = d->getLatency();
  m_processor->getEventsQueue().insert(d, latency);
}

/*****************************************************************/
/*********************** In-order Scheduler **********************/
/*****************************************************************/

void 
InorderScheduler::select() {
  int num_executed = 0;
  int num_memory = 0;
  int num_store = 0;

  while (!empty()) {
    DynamicInst *d = dynamic_cast<DynamicInst *>(getNextWaiter());
    assert(d != NULL);
    if (d->getQPointer() != m_head) { break; }
    assert(d->isExecuteReady());
    const TransOp *trans_op = d->getTransOp();
    bool is_load = isload(trans_op->opcode);
    bool is_store = isstore(trans_op->opcode);

    if ((is_load && (num_memory >= g_memory_issue_width)) ||
        (is_store && (num_store >= g_store_issue_width))) {
      break;
    }

    if ((is_load || is_store) && !m_processor->cacheReady()) {
      break;
    }

    //! at this point we've decided to execute the instruction
    m_head ++;
    selectInstruction(d);
    num_executed ++;
    if (is_load) { 
      ++ num_memory; 
    } else if (is_store) {
      ++ num_memory;
      ++ num_store;
    }
    if (num_executed >= g_execute_width) { 
      break;
    }
  }
}

void 
InorderScheduler::squash(QPointer first_bad) {
  m_head = std::min(m_head, first_bad);
}

/*****************************************************************/
/******************** Out-of-order Scheduler *********************/
/*****************************************************************/

void 
OutorderScheduler::select() {
  int num_executed = 0;
  int num_memory = 0;
  int num_store = 0;
  Waiter *w = this, *m_next;

  while ((m_next = w->getNextWaiter()) != this) {
    DynamicInst *d = dynamic_cast<DynamicInst *>(m_next);
    assert(d != NULL);
    assert(d->isExecuteReady());
    const TransOp *trans_op = d->getTransOp();
    bool is_load = isload(trans_op->opcode);
    bool is_store = isstore(trans_op->opcode);

    if ((is_load && (num_memory >= g_memory_issue_width)) ||
        (is_store && (num_store >= g_store_issue_width))) {
      w = m_next;
      continue; 
    }

    if ((is_load || is_store) && !m_processor->cacheReady()) {
      w = m_next;
      continue;
    }

    //! at this point we've decided to execute the instruction
    selectInstruction(d);
    num_executed ++;
    if (is_load) { 
      ++ num_memory; 
    } else if (is_store) {
      ++ num_memory;
      ++ num_store;
    }
    if (num_executed >= g_execute_width) {
      break;
    }
  }
}

