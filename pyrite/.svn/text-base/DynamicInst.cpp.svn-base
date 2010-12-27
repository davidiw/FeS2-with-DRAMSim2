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

#include <simics/api.h>
#undef unlikely
#undef likely
#include <simics/arch/x86.h>
#include <simics/alloc.h>
#include <simics/utils.h>

#include "pyrite.h"
#include "globals.h"
#include "ptlhwdef.h"

#include "TraceDecoder.h"
#include "EventLog.h"
#include "RegFile.h"
#include "DynamicInst.h"
#include "Processor.h"
#include "RubyMemoryInterface.h"
#include "params.h"

void
DynamicInst::init(Processor *processor, const TransOp &trans_op, Waddr rip,
                  uopimpl_func_t exec, QPointer q_ptr, W8 latency) {
  reset();
  m_processor = processor;
  m_trans_op = trans_op;
  m_rip = rip;
  m_exec = exec;
  m_q_ptr = q_ptr;
  m_latency = latency;
}

void 
DynamicInst::reset() {
  m_executed = false;
  m_lsq_inserted = false;
  m_unaligned = false;
  m_mispredicted = false;
  m_load_ordering_violation = false;
  m_load_consistency_violation = false;
  m_check_exception = false;
  m_starts_x86_op = false;
  m_ends_x86_op = false;

  m_rip = 0;
  m_pred_target = 0;

  
  m_q_ptr = 0;
  m_ra_preg = m_rb_preg = m_rc_preg = REG_NULL_SRC;
  m_rd_preg = m_rd_preg_old = m_rof_preg_old = m_rcf_preg_old = m_rzapsf_preg_old = REG_NULL_DEST; 
  m_mem_operand.clear();
  m_stage = FETCH_STAGE;
  assert(m_store_waiters.empty());
}

DynamicInst &
DynamicInst::operator= (const DynamicInst &rhs) {
  if (this != &rhs) {
    assert(rhs.detached());
    assert(rhs.m_store_waiters.empty());
    m_trans_op = rhs.m_trans_op;
    m_executed = rhs.m_executed;
    m_lsq_inserted = rhs.m_lsq_inserted;
    m_unaligned = rhs.m_unaligned;
    m_latency = rhs.m_latency;

    m_mispredicted = rhs.m_mispredicted;
    m_pred_target = rhs.m_pred_target;
  
    m_q_ptr = rhs.m_q_ptr;
    m_ra_preg = rhs.m_ra_preg;
    m_rb_preg = rhs.m_rb_preg;
    m_rc_preg = rhs.m_rc_preg;
    m_rd_preg = rhs.m_rd_preg;
    m_rd_preg_old = rhs.m_rd_preg_old;
    m_rof_preg_old = rhs.m_rof_preg_old;
    m_rcf_preg_old = rhs.m_rcf_preg_old;
    m_rzapsf_preg_old = rhs.m_rzapsf_preg_old;
    m_mem_operand = rhs.m_mem_operand;
    m_exec = rhs.m_exec;
    m_is = rhs.m_is;
    m_rb = rhs.m_rb;
    m_stage = rhs.m_stage;
    m_record = 0;
    assert(m_store_waiters.empty());
  }
}

inst_record_t::stage_t STAGE_MAP[MAX_STAGES] = {
  /* STAGE_MAP[DynamicInst::FETCH_STAGE] = */ inst_record_t::FETCH_STAGE,
  /* STAGE_MAP[DynamicInst::DECODE_STAGE] = */ inst_record_t::DECODE_STAGE,
  /* STAGE_MAP[DynamicInst::WAIT_RA_STAGE] = */ inst_record_t::READY_STAGE,
  /* STAGE_MAP[DynamicInst::WAIT_RB_STAGE] = */ inst_record_t::READY_STAGE,
  /* STAGE_MAP[DynamicInst::WAIT_RC_STAGE] = */ inst_record_t::READY_STAGE,
  /* STAGE_MAP[DynamicInst::READY_STAGE] = */ inst_record_t::READY_STAGE, 
  /* STAGE_MAP[DynamicInst::EXECUTE_STAGE] = */ inst_record_t::EXECUTE_STAGE,
  /* STAGE_MAP[DynamicInst::MEMORY_STAGE] = */ inst_record_t::COMPLETE_STAGE,
  /* STAGE_MAP[DynamicInst::COMPLETE_STAGE] = */ inst_record_t::COMPLETE_STAGE, 
  /* STAGE_MAP[DynamicInst::RETIRE_STAGE] = */ inst_record_t::RETIRE_STAGE};

void 
DynamicInst::setStage(enum StageType _stage) { 
  if (m_record) {
    m_record->Set(STAGE_MAP[_stage], m_processor->getCurrentCycle());
  }
  m_stage = _stage; 
}

bool 
DynamicInst::uopIsLegal(){
  // check for insns using CTX reg in anything other than ra
  if ((m_trans_op.rb == REG_ctx) || (m_trans_op.rc == REG_ctx) ||
      (m_trans_op.rd == REG_ctx)){
    g_stats.incrementInstructionsUsingCTXReg(m_processor->getProcNum());
    return false;
  }
  return true;
}

void
DynamicInst::squash() {
  assert(m_store_waiters.empty());

  if (waiting()) { detach(); }
  if (m_record) {
    m_record->Free();
    m_record = NULL;
  }
  unwindRegisters();

  m_executed = false;
  m_lsq_inserted = false;
  m_unaligned = false;
  m_mispredicted = false;
  m_load_ordering_violation = false;
  m_load_consistency_violation = false;
  m_check_exception = false;

  m_mem_operand.clear();
}

void
DynamicInst::unwindRegisters() {
  if(isRenamed() && !isRetired()) {
    LogicalFile &front_end_map = m_processor->getFrontEndMap();
    PhysicalFile &physical_file = m_processor->getPhysicalFile();
      
    // undo frontend map updates for all squashed instructions that have been renamed

    // printf("---undo rename for uop: %d\n", m_q_ptr);
    if (m_ra_preg != REG_NULL_SRC) { 
      physical_file.decrementRefCount(m_ra_preg);
      m_ra_preg = REG_NULL_SRC;
    }
    if (m_rb_preg != REG_NULL_SRC) { 
      physical_file.decrementRefCount(m_rb_preg);
      m_rb_preg = REG_NULL_SRC;
    }
    if (m_rc_preg != REG_NULL_SRC) { 
      physical_file.decrementRefCount(m_rc_preg);
      m_rc_preg = REG_NULL_SRC;
    }
    if(m_rd_preg != REG_NULL_DEST) {
      if (m_trans_op.setflags && !m_trans_op.nouserflags) {
        W64 flagmask = setflags_to_x86_flags[m_trans_op.setflags];
        if(flagmask & FLAG_OF) {
          front_end_map.setMapping(REG_of, m_rof_preg_old);
          physical_file.decrementRefCount(m_rof_preg_old);
        }
        if(flagmask & FLAG_CF) {
          front_end_map.setMapping(REG_cf, m_rcf_preg_old);
          physical_file.decrementRefCount(m_rcf_preg_old);
        }
        if(flagmask & FLAG_ZAPS) {
          front_end_map.setMapping(REG_zf, m_rzapsf_preg_old);
          physical_file.decrementRefCount(m_rzapsf_preg_old);
        }
      }

      front_end_map.setMapping(m_trans_op.rd, m_rd_preg_old);
      physical_file.decrementRefCount(m_rd_preg_old);
      m_rd_preg = m_rd_preg_old = m_rof_preg_old = m_rcf_preg_old = m_rzapsf_preg_old = REG_NULL_DEST; 
    }
  }

  if (m_lsq_inserted) {
    // remove squashed loads and stores from the load/store queue
    m_processor->getLSQ().remove(this);
    m_lsq_inserted = false;
  }
}

void 
DynamicInst::rename() {
  /* instruction scheduling priority is passed in a decode time (can
     later be updated. */
  assert(m_stage == FETCH_STAGE);
  setStage(DECODE_STAGE);

  assert(!m_executed);

  LogicalFile &front_end_map = m_processor->getFrontEndMap();
  PhysicalFile &physical_file = m_processor->getPhysicalFile();
	 
  // rename source registers
  m_ra_preg = front_end_map.getMapping(m_trans_op.ra);
  physical_file.incrementRefCount(m_ra_preg);

//  printf("ra mapping: %d->%d\n", m_trans_op.ra, m_ra_preg);

  if(m_trans_op.rb != REG_imm) {
    m_rb_preg = front_end_map.getMapping(m_trans_op.rb);
    physical_file.incrementRefCount(m_rb_preg);
//     printf("rb mapping: %d->%d\n", m_trans_op.rb, m_rb_preg);
  }
  
  if (m_trans_op.rc != REG_imm) {
    m_rc_preg = front_end_map.getMapping(m_trans_op.rc);
    physical_file.incrementRefCount(m_rc_preg);
//	 printf("rc mapping: %d->%d\n", m_trans_op.rc, m_rc_preg);
  }
  
  assert(!uopWritesDestinationRegister(m_trans_op) ||
         (uopWritesDestinationRegister(m_trans_op) &&
          (!isbranch(m_trans_op.opcode) ||
           !(m_trans_op.setflags && !m_trans_op.nouserflags))));

  // allocate a new physical registers, capture the old mapping and update rename map
  if (uopWritesDestinationRegister(m_trans_op) && !isbranch(m_trans_op.opcode) &&
      (m_trans_op.rd != REG_zero)) {
    m_rd_preg_old = front_end_map.getMapping(m_trans_op.rd);
    physical_file.incrementRefCount(m_rd_preg_old);
    assert(m_rd_preg_old > REG_NULL_SRC);
    m_rd_preg = front_end_map.getNewMapping(m_trans_op.rd);
//	 printf("rd mapping: %d->%d (old = %d)\n", m_trans_op.rd, m_rd_preg, m_rd_preg_old);

    if (m_trans_op.setflags && !m_trans_op.nouserflags) {
      assert(!isbranch(m_trans_op.opcode));
      W64 flagmask = setflags_to_x86_flags[m_trans_op.setflags];
      if(flagmask & FLAG_OF) {
        m_rof_preg_old = front_end_map.getMapping(REG_of);
        physical_file.incrementRefCount(m_rof_preg_old);
        front_end_map.setMapping(REG_of, m_rd_preg);
//           printf("rof mapping: %d->%d (old = %d)\n", REG_of, m_rd_preg, m_rof_preg_old);
      }
      if(flagmask & FLAG_CF) {
        m_rcf_preg_old = front_end_map.getMapping(REG_cf);
        physical_file.incrementRefCount(m_rcf_preg_old);
        front_end_map.setMapping(REG_cf, m_rd_preg);
//           printf("rcf mapping: %d->%d (old = %d)\n", REG_cf, m_rd_preg, m_rcf_preg_old);
      }
      if(flagmask & FLAG_ZAPS) {
        m_rzapsf_preg_old = front_end_map.getMapping(REG_zf);
        physical_file.incrementRefCount(m_rzapsf_preg_old);
        front_end_map.setMapping(REG_zf, m_rd_preg);
//           printf("rzapsf mapping: %d->%d (old = %d)\n", REG_zf, m_rd_preg, m_rzapsf_preg_old);
      }
    }
  }

  /* for tracking register dependences when recording instruction execution */
  if (m_record) {
    if (m_ra_preg != REG_NULL_SRC) { 
      m_record->SetRegProducer(physical_file.getWriter(m_ra_preg));
    }
    if (m_rb_preg != REG_NULL_SRC) { 
      m_record->SetRegProducer(physical_file.getWriter(m_rb_preg));
    }
    if (m_rc_preg != REG_NULL_SRC) { 
      m_record->SetRegProducer(physical_file.getWriter(m_rc_preg));
    }

    if (m_rd_preg != REG_NULL_DEST) { 
      physical_file.setWriter(m_rd_preg, m_q_ptr);
    }  
  }
}

void 
DynamicInst::queue() { 
  LOG_EVENT(DEBUG_UOP, Box<TransOp>(m_trans_op), "Current uop.");

  if (m_record) { 
    m_record->Set(inst_record_t::QUEUE_STAGE, m_processor->getCurrentCycle());
  }
  setStage(WAIT_RA_STAGE);
}

void
DynamicInst::schedule() { 
  /* after decoding the instruction we put it in the window. Check
     whether the instruction is ready to be m_executed.  If register
     value is not ready, put the instruction in a waiting list for
     that register, otherwise send the instruction to the scheduler */
  PhysicalFile &pfile = m_processor->getPhysicalFile();

  switch (m_stage) {
    case WAIT_RA_STAGE:
      if ((m_ra_preg != REG_NULL_SRC) && !pfile.isReady(m_ra_preg)) {
        pfile.addWaiter(m_ra_preg, this);
        break;
      }
      setStage(WAIT_RB_STAGE);
      /* fall through */
    case WAIT_RB_STAGE:
      if ((m_rb_preg != REG_NULL_SRC) && !pfile.isReady(m_rb_preg)) {
        pfile.addWaiter(m_rb_preg, this);
        break;
      }

      if (isload(m_trans_op.opcode) || isstore(m_trans_op.opcode)) { 
        if (!addressGenerate()) {
          break;
        }
      }

      setStage(WAIT_RC_STAGE);

      /* fall through */
    case WAIT_RC_STAGE:
      if ((m_rc_preg != REG_NULL_SRC) && !pfile.isReady(m_rc_preg)) {
        pfile.addWaiter(m_rc_preg, this);
        break;
      }
      setStage(READY_STAGE);
      /* fall through */
    case READY_STAGE:
      /* when all registers are ready, tell the scheduler that this
         instruction is ready */
      m_processor->wakeup(this);
      break;
    case EXECUTE_STAGE:
      execute();
      break;
    case MEMORY_STAGE: 
      complete(); 
      break;
    default:
      /* should never reach this m_stage */
      assert(0);
  }
}

void 
DynamicInst::beginExecution() { 
  setStage(EXECUTE_STAGE); 
}

bool
DynamicInst::execute() {
  if (!uopIsLegal()){
    return false;
  }
  
  assert(!m_executed);
	 
  // read registers
  W64 ra, rc;  // rb is allocated into "this" to survive to do "Complete()" if deferred by a cache miss
  W16 raflags, rbflags, rcflags;
  readRegisters(ra, m_rb, rc, raflags, rbflags, rcflags);
	 
  // execute
  m_processor->logUopExecution(m_trans_op, ra, m_rb, rc, raflags, rbflags, rcflags);
	 
  bzero( &m_is, sizeof(IssueState) );
  m_is.reg.rdflags = 0;
  m_is.reg.rddata = 0;
	 
  if (isfence(m_trans_op.opcode)) {
    // treated as a no-op for now
  } else if (isload(m_trans_op.opcode)) { // load
    if (m_unaligned) { 
      m_processor->alignmentException(m_q_ptr);
      return true;
    }

    if (m_mem_operand.getSize() == 0) {
      assert(m_trans_op.cond == LDST_ALIGN_HI);
      complete();
      return true;
    }
    
    LoadStoreQueue &lsq = m_processor->getLSQ();
    DoubleWord lsq_data(m_mem_operand);
    bool needs_to_wait = !lsq.loadSearch(this, &m_mem_operand, &lsq_data);

    if (needs_to_wait) {
      return true;  // can't complete now, loadSearch put us on a store's waitlist
    }

    if (!lsq_data.covers(m_mem_operand) && !m_trans_op.internal) {
        
      m_processor->cacheAccess(this, REQUEST_READ, m_mem_operand.alignedAddr());
      setStage(MEMORY_STAGE);
        
      if (!detached()) { // must have missed
        recordEvent(inst_record_t::DCACHE_MISS);
        return true;  // don't complete
      }
    }

    assert(detached());
    // otherwise we can complete now
  } else if (isstore(m_trans_op.opcode)) { // store
    if (m_unaligned) {
      m_processor->alignmentException(m_q_ptr);
      return true;
    }

    if (m_mem_operand.getSize() == 0) {
      assert(m_trans_op.cond == LDST_ALIGN_HI);
      complete();
      return true;
    }
    
    W64 value = (m_trans_op.cond == LDST_ALIGN_HI) ? (rc >> (m_mem_operand.getSize() * 8)) : rc;
	 
    // printf("uop: %d  P: %d(%x) %d\n", m_q_ptr, (unsigned)phys_addr, (unsigned)phys_addr, num_bytes);
    m_mem_operand.setValue(value);

    LoadStoreQueue &lsq = m_processor->getLSQ();
    lsq.storeSearch(this);

    setStage(MEMORY_STAGE);
    m_store_waiters.wakeupAll();

    if (!m_trans_op.internal) {
      // Issue a prefetch-exclusive for this line
      m_processor->cacheAccess(0, REQUEST_WRITE, m_mem_operand.alignedAddr());
    }

    //NOTE: stores don't really complete until they retire.
    return true;
  } else{ // vanilla instruction
    if (isbranch(m_trans_op.opcode)){
      m_is.brreg.riptaken = m_trans_op.riptaken;
      m_is.brreg.ripseq = m_trans_op.ripseq;
    }
    m_exec(m_is, ra, m_rb, rc, raflags, rbflags, rcflags);
  }
  complete();
  return true;
}

bool
DynamicInst::addressGenerate() { 
  assert(isload(m_trans_op.opcode) || isstore(m_trans_op.opcode));
  W64 ra = m_processor->getPhysicalFile().getValue(m_ra_preg);
  W64 rb = (m_trans_op.rb == REG_imm) ? m_trans_op.rbimm : m_processor->getPhysicalFile().getValue(m_rb_preg);
  W64 num_bytes = 1 << m_trans_op.size;

  Waddr addr;
  switch(m_trans_op.cond) {
    case LDST_ALIGN_NORMAL:
      addr = ra + rb;
      break;
    case LDST_ALIGN_LO:
      addr = ra;
      break;
    case LDST_ALIGN_HI:
      addr = ra + (1 << g_params.getMemoryBlockBits());
      break;
    default:
      assert(0);
  }

  W64 phys_addr;
  if (!m_trans_op.internal) {
    if (!m_processor->translateAddress(addr, phys_addr, Sim_DI_Data)) {
      m_processor->logNoTranslationDataMemoryOperation(*this, addr, isload(m_trans_op.opcode));
      m_processor->setOldestBad(m_q_ptr);
      return false;
    }
  } else {
    phys_addr = addr;
    assert(addr >= (W64)m_processor);
  }
  
  // printf("uop: %d  P: %d(%x) %d\n", m_q_ptr, (unsigned)phys_addr, (unsigned)phys_addr, num_bytes);

  size_t overflow = m_mem_operand.setAddress(phys_addr, num_bytes);
  assert(!overflow || !m_trans_op.internal);

  if ((overflow != 0) && (m_trans_op.cond == LDST_ALIGN_NORMAL)) {
    m_processor->logUnalignedDataMemoryOperation(*this, addr, isload(m_trans_op.opcode));
    m_unaligned = true;
    return true;
  } else if (m_trans_op.cond == LDST_ALIGN_LO) {
    overflow = m_mem_operand.setAddress(phys_addr, num_bytes - overflow);
    assert(overflow == 0);
  } else if (m_trans_op.cond == LDST_ALIGN_HI) {
    if (overflow) {
      overflow = m_mem_operand.setAddress(phys_addr & ~((Waddr)(1 << g_params.getMemoryBlockBits()) - 1), overflow);
      assert(overflow == 0);
    } else {
      m_mem_operand.clear();
      assert(m_mem_operand.getSize() == 0);
    }
  }

  if (!m_trans_op.internal) {
    g_stats.incrementMemoryAccesses(m_processor->getProcNum());
  }
  
  LoadStoreQueue &lsq = m_processor->getLSQ();
  lsq.insert(this);
  m_lsq_inserted = true;

  return true;
}

void 
DynamicInst::complete() { 
  if (isload(m_trans_op.opcode)) { // load
    if (m_mem_operand.getSize() == 0) {
      assert(m_trans_op.cond == LDST_ALIGN_HI);
      m_is.reg.rddata = m_rb;
    } else {
      DoubleWord lsq_data(m_mem_operand);
      LoadStoreQueue &lsq = m_processor->getLSQ();
      bool needs_to_wait = !lsq.loadSearch(this, &m_mem_operand, &lsq_data);

      if (needs_to_wait) {
        assert(!detached()); // should now be waiting on a store, who will wake it up so that is can complete
        return;
      }

      if (!lsq_data.covers(m_mem_operand)) {
        W64 load_value;
        if (!m_trans_op.internal) {
          load_value = m_processor->readFromSimicsMemory(m_mem_operand.addr(),
                                                         m_mem_operand.getSize());
        } else {
          load_value = *((W64 *)m_mem_operand.addr());
        }
        m_mem_operand.setValue(load_value);
      }
      lsq_data.writeValue(m_mem_operand);
	 
      m_is.reg.rddata = m_mem_operand.value();
      if (m_trans_op.opcode == OP_ldx) {
        m_is.reg.rddata = signext64(m_is.reg.rddata, 8 * (1 << m_trans_op.size));
      } else if (m_trans_op.cond == LDST_ALIGN_HI) {
        W64 num_bytes = 1 << m_trans_op.size;
        W64 overflow = m_mem_operand.getSize();
        W64 underflow = num_bytes - overflow;
        m_is.reg.rddata = m_rb | (m_is.reg.rddata << (underflow * 8));
      }
    }
    LOG_EVENT(DEBUG_MEMORY, Box<TransOp>(m_trans_op), "Value: " << hex << m_is.reg.rddata << dec);
  }

  // write rd and flags
  m_executed = true;
  if (uopWritesDestinationRegister(m_trans_op)) {
    if (isbranch(m_trans_op.opcode)) {
      assert(m_trans_op.rd == REG_rip);
      m_mispredicted = m_is.reg.rddata != m_pred_target;
      m_processor->resolveBranch(m_q_ptr, m_is.reg.rddata);
    }
    else if(m_trans_op.rd != REG_zero) {
      m_processor->getPhysicalFile().setValue(m_rd_preg, m_is.reg.rddata);
    }
    if ((m_trans_op.setflags) && (m_trans_op.rd != REG_zero)) {
      m_processor->getPhysicalFile().setFlags(m_rd_preg, m_is.reg.rdflags);
      m_processor->logUopExecutionResult(m_trans_op, m_is, m_is.reg.rdflags);
    }
  }

  if (ischeck(m_trans_op.opcode) && (m_is.reg.rdflags & FLAG_INV)) {
    assert(m_is.reg.rddata == EXCEPTION_SkipBlock);

    // if this uop already ends its x86 op, it is being replayed and no exception needs to be thrown
    if(endsX86Op()) {
      setCheckException();
    } else {
      // check failed and we should flush the remaining uops in this x86 op
      m_processor->checkException(m_q_ptr);
    }
  }

  m_processor->logStatsForUop(m_trans_op);
  setStage(COMPLETE_STAGE);
}

bool
DynamicInst::demandStore() {
  if (!m_trans_op.internal) {
    m_processor->cacheAccess(this, REQUEST_WRITE, m_mem_operand.alignedAddr());
    if (detached()) {  // must have succeeded immediately
      complete();
      return true;
    }
    recordEvent(inst_record_t::DCACHE_MISS);
    return false;  // will succeed at some point
  } else {
    // internal "store"
    W64 internal_value = *((W64 *)m_mem_operand.addr()) & ~m_mem_operand.mask();
    *((W64 *)m_mem_operand.addr()) = internal_value | m_mem_operand.value();
    complete();
    return true;
  }
}

void 
DynamicInst::completeAtDecode() {
  if (m_record) { 
    QPointer cycle = m_processor->getCurrentCycle();
    m_record->Set(inst_record_t::DECODE_STAGE, cycle);
    m_record->Set(inst_record_t::QUEUE_STAGE, cycle);
    m_record->Set(inst_record_t::READY_STAGE, cycle);
    m_record->Set(inst_record_t::EXECUTE_STAGE, cycle);
  }
  m_executed = true;
  setStage(COMPLETE_STAGE);
}

void
DynamicInst::commit() {
  LogicalFile &retire_map = m_processor->getRetireMap();
  PhysicalFile &physical_file = m_processor->getPhysicalFile();

  assert(m_stage == COMPLETE_STAGE);
  setStage(RETIRE_STAGE);

  assert(m_executed);

  // update retire map
  // printf("+++retire uop: %d\n", m_q_uop_retire);
  if (m_ra_preg != REG_NULL_SRC) { 
    physical_file.decrementRefCount(m_ra_preg);
  }
  if (m_rb_preg != REG_NULL_SRC) { 
    physical_file.decrementRefCount(m_rb_preg);
  }
  if (m_rc_preg != REG_NULL_SRC) { 
    physical_file.decrementRefCount(m_rc_preg);
  }
  if (m_rd_preg != REG_NULL_DEST) {
    // printf("free old rdflags: %d\n", m_trans_op.rdflags_preg_old);
    if (m_trans_op.setflags && !m_trans_op.nouserflags) {
      W64 flagmask = setflags_to_x86_flags[m_trans_op.setflags];
      if(flagmask & FLAG_OF) {
        retire_map.setMapping(REG_of, m_rd_preg);
        physical_file.decrementRefCount(m_rof_preg_old);
      }
      if(flagmask & FLAG_CF) {
        retire_map.setMapping(REG_cf, m_rd_preg);
        physical_file.decrementRefCount(m_rcf_preg_old);
      }
      if(flagmask & FLAG_ZAPS) {
        retire_map.setMapping(REG_zf, m_rd_preg);
        physical_file.decrementRefCount(m_rzapsf_preg_old);
      }
    }

    // printf("free old rd: %d\n", m_trans_op.m_rd_preg_old);
    retire_map.setMapping(m_trans_op.rd, m_rd_preg);
    physical_file.decrementRefCount(m_rd_preg_old);
  }
}

void
DynamicInst::setLoadOrderingViolation() {
  m_load_ordering_violation = true;
  g_stats.incrementLoadOrderingSquashes(m_processor->getProcNum());
}

void
DynamicInst::setLoadConsistencyViolation() {
  m_load_consistency_violation = true;
  g_stats.incrementLoadConsistencySquashes(m_processor->getProcNum());
}

void
DynamicInst::setCheckException() {
  m_check_exception = true;
  g_stats.incrementCheckExceptions(m_processor->getProcNum());
}


bool 
DynamicInst::isMemoryInst() {
  return isload(m_trans_op.opcode) || isstore(m_trans_op.opcode);
}

void 
DynamicInst::setRecord(real_inst_record_t *r) { 
  assert(r); 
  r->SetPC(m_rip);
  m_record = r; 
}

real_inst_record_t *
DynamicInst::getRecord() { 
  real_inst_record_t *ret_val = m_record;
  m_record = 0;
  return ret_val; 
}

void 
DynamicInst::recordEvent(inst_record_t::event_t e) {
  if (m_record) {
    m_record->AddEvent(e);
  }
}

void
DynamicInst::readRegisters(W64& ra, W64& rb, W64& rc, 
                           W16& raflags, W16& rbflags, W16& rcflags) {
  
  rb = rc = 0;
  rbflags = rcflags = 0;

  ra = m_processor->getPhysicalFile().getValue(m_ra_preg);
  raflags = m_processor->getPhysicalFile().getFlags(m_ra_preg);

  if (m_trans_op.rb == REG_imm){
    rb = m_trans_op.rbimm;
  } else{
    rb = m_processor->getPhysicalFile().getValue(m_rb_preg);
    rbflags = m_processor->getPhysicalFile().getFlags(m_rb_preg);
  }

  if (m_trans_op.rc == REG_imm) {
    rc = m_trans_op.rcimm;
  } else {
    rc = m_processor->getPhysicalFile().getValue(m_rc_preg);
    rcflags = m_processor->getPhysicalFile().getFlags(m_rc_preg);
  }
}
