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

//===-- Predictor.cpp - an object that holds predictor state -------*- C++ -*--=//
//
//! This file defines an interface for three types of branch
//! predictors: direct branch predictors, indirect branch predictors,
//! and return address stacks.
//
//===----------------------------------------------------------------------===//

#include "DynamicInst.h"
#include "QPointer.h"
#include "Predictor.h"

ReturnAddressStack::ReturnAddressStack(int size)
  : m_speculative_rs(), m_committed_rs(), m_table(size), m_next_top_of_stack(size, -1) {
}

void 
ReturnAddressStack::push(QPointer q_ptr, Waddr return_target_rip) {
  // push address on stack
  m_table[m_speculative_rs.getNextFree()] = return_target_rip;
  m_next_top_of_stack[m_speculative_rs.getNextFree()] = m_speculative_rs.getTopOfStack();

  /* update pointers */
  m_speculative_rs.setTopOfStack(m_speculative_rs.getNextFree());
  m_speculative_rs.setNextFree(rasIncrement(m_speculative_rs.getNextFree()));

  // checkpoint ras pointers
  m_inflights[q_ptr] = m_speculative_rs;
}

Waddr 
ReturnAddressStack::pop(QPointer q_ptr) {
  //! checkpoint ras pointers
  m_inflights[q_ptr] = m_speculative_rs;

  if (m_speculative_rs.getTopOfStack() == -1) { 
    return 0; 
  }

  /* read entry from table */
  Waddr ret_val = m_table[m_speculative_rs.getTopOfStack()];

  /* update pointers */
  int next_tos = m_next_top_of_stack[m_speculative_rs.getTopOfStack()];
  if (next_tos == -1) { 
    m_speculative_rs.setTopOfStack(-1); 
  } else {
    bool next_free_gt_tos = m_speculative_rs.getNextFree() > m_speculative_rs.getTopOfStack();
    bool next_free_gt_next_tos = m_speculative_rs.getNextFree() > next_tos;
    bool tos_gt_next_tos = m_speculative_rs.getTopOfStack() > next_tos;
    // if next_tos is garbage if it was overwritten by a younger prediction.  This will
    // have occurred if next_free is logically between the current top_of_stack and the
    // next_top_of_stack.  The following 3-way xor checks for this case (i.e. if 2 or 0
    // of the above conditions are true).
    m_speculative_rs.setTopOfStack((next_free_gt_tos ^ next_free_gt_next_tos ^ tos_gt_next_tos) ? next_tos : -1);
  }
  /* next_free -- no change */

  // checkpoint ras pointers
  m_inflights[q_ptr] = m_speculative_rs;

  return ret_val;
}

void 
ReturnAddressStack::squash(QPointer first_bad) {
  m_inflights.squash(first_bad);
  m_speculative_rs = (m_inflights.empty()) ? m_committed_rs : m_inflights.rbegin()->second;
}

void 
ReturnAddressStack::commit(QPointer q_ptr) {
  assert(m_inflights.find(q_ptr) != m_inflights.end());
  m_committed_rs = m_inflights[q_ptr];
  m_inflights.commit(q_ptr);
}

bool
GsharePredictor::predict(Waddr rip, QPointer q_ptr) {
  int hashed_rip = hash(rip);
  bool pred = (m_table[hashed_rip] >= 2);
  m_history = m_history_mask & ((m_history << 1) + (pred ? 1 : 0));
  m_inflights[q_ptr] = Record(hashed_rip, m_history);
  return pred;
}

void 
GsharePredictor::resolve(QPointer q_ptr, bool taken) {
  assert(m_inflights.find(q_ptr) != m_inflights.end());
  Record &record = m_inflights[q_ptr];
  record.second = (record.second & ~0x1) + (taken ? 1 : 0);
}

void 
GsharePredictor::squash(QPointer first_bad) {
  m_inflights.squash(first_bad);
  m_history = (m_inflights.empty()) ? m_committed_history : m_inflights.rbegin()->second.second;
}

void 
GsharePredictor::commit(QPointer q_ptr) {
  const unsigned char update[2][4] = {{0, 0, 1, 2}, {1, 2, 3, 3}};
  assert(m_inflights.find(q_ptr) != m_inflights.end());
  Record &record = m_inflights[q_ptr];
  unsigned char &counter = m_table[record.first];
  assert(counter < 4);
  counter = update[record.second & 0x1][counter];
  m_committed_history = record.second;
  m_inflights.commit(q_ptr);
}

Waddr 
SimpleIndirectPredictor::predict(Waddr rip, QPointer q_ptr) {
  int hashed_rip = hash(rip);
  Waddr pred = m_targets[hashed_rip];
  m_inflights[q_ptr] = Record(hashed_rip, pred);
  return pred;
}

void 
SimpleIndirectPredictor::resolve(QPointer q_ptr, Waddr target) {
  assert(m_inflights.find(q_ptr) != m_inflights.end());
  Record &record = m_inflights[q_ptr];
  if (record.second != target) { //! a mispredict
    record.second = target;
  }
}

void 
SimpleIndirectPredictor::commit(QPointer q_ptr) {
  assert(m_inflights.find(q_ptr) != m_inflights.end());
  Record &record = m_inflights[q_ptr];
  m_targets[record.first] = record.second;
  m_inflights.commit(q_ptr);
}

PredictorSet::PredictorSet() {
  /* configure direct branch predictor */
  unsigned hist_length = 8;
  unsigned direct_table_bits = 12;
  m_direct = new GsharePredictor(hist_length, direct_table_bits);

  /* configure indirect branch predictor */
  int indirect_table_bits = 8;
  m_indirect = new SimpleIndirectPredictor(indirect_table_bits);

  /* configure ras */
  int ras_size = 32;
  m_ras = new ReturnAddressStack(ras_size);

  /* init data recording stuff */
  for (int i = 0 ; i < (int) PTYPE_MAX ; i ++) { 
    m_branches[i] = m_mispredictions[i] = 0;
  }
}

PredType
PredictorSet::getPredType(DynamicInst *inst) {
  int opcode = inst->getOpcode();
  int extshift = inst->getTransOp()->extshift;
  assert(isbranch(opcode));

  if(isclass(opcode, OPCLASS_COND_BRANCH)) {
    return PTYPE_COND;
  } else if(extshift == BRANCH_HINT_POP_RAS) {
    return PTYPE_RAS;
  } else if(isclass(opcode, OPCLASS_INDIR_BRANCH)) {
    return PTYPE_INDIRECT;
  } 
  assert(isclass(opcode, OPCLASS_UNCOND_BRANCH));
  return PTYPE_NONE;
}

Waddr
PredictorSet::predict(DynamicInst *inst, Waddr fallthrough) {
  Waddr branch_rip = inst->getRIP();
  const TransOp *trans_op = inst->getTransOp();
  int opcode = inst->getOpcode();
  Waddr pred_target = 0;
  PredType pred_type = getPredType(inst);
  
  switch(pred_type) {
    case PTYPE_COND: {
      bool taken = m_direct->predict(branch_rip, inst->getQPointer());
      pred_target = (taken) ?  trans_op->riptaken : trans_op->ripseq;
      break;
    }
    case PTYPE_INDIRECT:
      pred_target = m_indirect->predict(branch_rip, inst->getQPointer());
      break;
    case PTYPE_RAS:
      pred_target = m_ras->pop(inst->getQPointer());
      break;
    case PTYPE_NONE:
      assert(isclass(opcode, OPCLASS_UNCOND_BRANCH));
      pred_target = trans_op->riptaken;
      break;
    default:
      assert(0);
      break;
  }

  if(trans_op->extshift == BRANCH_HINT_PUSH_RAS) {
    m_ras->push(inst->getQPointer(), fallthrough);
  }

  inst->setPredTarget(pred_target);
  return pred_target;
}

void 
PredictorSet::resolve(DynamicInst *inst, Waddr actual_target) {
  const TransOp *trans_op = inst->getTransOp();
  PredType pred_type = getPredType(inst);
  
  switch(pred_type) {
    case PTYPE_COND: {
      bool taken = actual_target != trans_op->ripseq;
      m_direct->resolve(inst->getQPointer(), taken);
      break;
    }
    case PTYPE_INDIRECT:
      m_indirect->resolve(inst->getQPointer(), actual_target);
      break;
    default:
      // do nothing
      break;
  }
}

void 
PredictorSet::squash(QPointer first_bad) {
  m_direct->squash(first_bad);
  m_indirect->squash(first_bad);
  m_ras->squash(first_bad);
}

void 
PredictorSet::commit(DynamicInst *inst) {
  Waddr branch_rip = inst->getRIP();
  const TransOp *trans_op = inst->getTransOp();
  PredType pred_type = getPredType(inst);
  
  m_branches[pred_type] ++;
  if (inst->isMispredicted()) {
    m_mispredictions[pred_type] ++;
  }

  switch(pred_type) {
    case PTYPE_COND:
      m_direct->commit(inst->getQPointer());
      break;
    case PTYPE_INDIRECT:
      m_indirect->commit(inst->getQPointer());
      break;
    case PTYPE_RAS:
      m_ras->commit(inst->getQPointer());
      break;
    default:
      // do nothing
      break;
  }

  if(trans_op->extshift == BRANCH_HINT_PUSH_RAS) {
    m_ras->commit(inst->getQPointer());
  }
}
  
void 
PredictorSet::printAccuracies(FILE *file) {
  const char *branch_types[] = {"PTYPE_COND    ",
                                "PTYPE_INDIRECT", 
                                "PTYPE_RAS     ",
                                "PTYPE_NONE    "};
  fprintf(file, "\n");
  W64 total_misp = 0, total_branches = 0;
  for (int i = 0 ; i < (int) PTYPE_MAX ; i ++) { 
    fprintf(file, "%s: %d\t%d\t%f\n", branch_types[i], (int)m_mispredictions[i], 
            (int)m_branches[i], 1.0 - (float)m_mispredictions[i]/(float)m_branches[i]);
    total_misp += m_mispredictions[i];
    total_branches += m_branches[i];
  }
  total_branches -= m_branches[PTYPE_NONE]; /* these aren't really predictions */
  fprintf(file, "All Predictions %d\t%d\t%f\n", (int)total_misp, 
          (int)total_branches, 1.0 - (float)total_misp/(float)total_branches);
}

void 
PredictorSet::clearStats() {
  for (int i = 0 ; i < (int) PTYPE_MAX ; i ++) {
    m_branches[i] = 0;
    m_mispredictions[i] = 0;
  }  
}
