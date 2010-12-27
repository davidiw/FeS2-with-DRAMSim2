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

//===-- DynamicInst.h - class for dynamic instructions -----------*- C++ -*--=//
//
//! The dynamic instruction class is the objects responsible for
//! holding instance specific state of instructions.
//
//===----------------------------------------------------------------------===//

#ifndef __DYNAMIC_INST_H
#define __DYNAMIC_INST_H

#include "globals.h"
#include "ptlhwdef.h"
#include "RegFile.h"
#include "Event.h"
#include "DoubleWord.h"
#include "inst_record.h"

struct TransOp;
class Processor;

enum StageType {FETCH_STAGE, DECODE_STAGE, WAIT_RA_STAGE, 
                WAIT_RB_STAGE, WAIT_RC_STAGE, READY_STAGE, 
                EXECUTE_STAGE, MEMORY_STAGE,
                COMPLETE_STAGE, RETIRE_STAGE, MAX_STAGES};

/*****************************************************************/
/********************* Dynamic Instruction ***********************/
/*****************************************************************/

class DynamicInst : public Waiter {
public:
  DynamicInst() : m_record(0) { reset(); }
  DynamicInst(const DynamicInst &rhs) { assert(0); }

  void init(Processor *procesor, const TransOp &trans_op, Waddr rip, /*X86Op *x86_op,*/
            uopimpl_func_t exec, QPointer q_ptr, W8 latency);
  void reset();

  void setStage(enum StageType _stage);
  bool isRenamed() const { return (m_stage >= DECODE_STAGE); }
  bool isExecuteReady() const { return (m_stage == READY_STAGE); }
  bool isExecuted() const { return (m_stage > READY_STAGE); }
  bool isMemoryIssued() const { return (m_stage >= MEMORY_STAGE); }
  bool isRetireReady() const { return (m_stage == COMPLETE_STAGE); }
  bool isRetired() const { return (m_stage == RETIRE_STAGE); }
  void wakeup() { schedule(); }
  virtual W64 priority() { return m_q_ptr; }

  bool uopIsLegal();
  QPointer getQPointer() const { return m_q_ptr; }
  Waddr getRIP() const { return m_rip; }

  void squash();
  void unwindRegisters();

  void rename();
  void queue();
  void schedule();
  void beginExecution();
  bool execute();
  bool addressGenerate();
  void complete();
  void completeAtDecode();
  bool demandStore();
  void commit();

  void setLoadOrderingViolation();
  bool hasLoadOrderingViolation() { return m_load_ordering_violation; }
  void setLoadConsistencyViolation();
  bool hasLoadConsistencyViolation() { return m_load_consistency_violation; }
  void setCheckException();
  bool hasCheckException() { return m_check_exception; }

  bool isMemoryInst();
  bool isInternal() { return m_trans_op.internal; }

  /* record related */
  void setRecord(real_inst_record_t *r);
  real_inst_record_t *peekRecord() { return m_record; }
  real_inst_record_t *getRecord();
  void recordEvent(inst_record_t::event_t e);

  W8 getLatency() const { return m_latency; }
  byte getOpcode() const { return m_trans_op.opcode; }
  const DoubleWord *getMemOperand() const { return &m_mem_operand; }
  const TransOp *getTransOp() const { return &m_trans_op; }

  void insertStoreWaiter(Waiter *store_waiter) { m_store_waiters.insertWaiter(store_waiter); }
  void setPredTarget(Waddr pred_target) { m_pred_target = pred_target; }
  bool isMispredicted() const { return m_mispredicted; }
  bool isLSQInserted() const { return m_lsq_inserted; }

  void setStartsX86Op(bool starts_x86_op = true) { m_starts_x86_op = starts_x86_op; }
  bool startsX86Op() const { return m_starts_x86_op; }

  void setEndsX86Op(bool ends_x86_op = true) { m_ends_x86_op = ends_x86_op; }
  bool endsX86Op() const { return m_ends_x86_op; }

  DynamicInst &operator= (const DynamicInst &rhs);

private:
  void readRegisters(W64& ra, W64& rb, W64& rc, 
                     W16& raflags, W16& rbflags, W16& rcflags);

  TransOp m_trans_op;

  bool m_executed;
  bool m_lsq_inserted;
  bool m_unaligned;
  bool m_mispredicted;
  bool m_load_ordering_violation;
  bool m_load_consistency_violation;
  bool m_check_exception;
  bool m_starts_x86_op;
  bool m_ends_x86_op;

  Waddr m_rip;
  Waddr m_pred_target;
  W8 m_latency;

  QPointer m_q_ptr;
  PhysName m_ra_preg;
  PhysName m_rb_preg;
  PhysName m_rc_preg;
  PhysName m_rd_preg;
  PhysName m_rd_preg_old;     // the previous mapping for rd
  PhysName m_rcf_preg_old;    // the previous mapping for cf
  PhysName m_rof_preg_old;    // the previous mapping for of
  PhysName m_rzapsf_preg_old; // the previous mapping for zapsf
  DoubleWord m_mem_operand;
  uopimpl_func_t m_exec;
  IssueState m_is;
  W64 m_rb;
  WaitList m_store_waiters;
  
  Processor *m_processor;

  StageType m_stage;
  real_inst_record_t *m_record;
};

#endif // __DYNAMIC_INST_H
