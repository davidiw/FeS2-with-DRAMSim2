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

//===-- Predictor.h - an object that holds predictor state -------*- C++ -*--=//
//
//! This file defines an interface for three types of branch
//! predictors: direct branch predictors, indirect branch predictors,
//! and return address stacks.
//
//===----------------------------------------------------------------------===//

#ifndef __PREDICTOR_H
#define __PREDICTOR_H

#include <map>
#include <vector>
#include "QPointer.h"
#include "globals.h"

enum PredType {PTYPE_COND, PTYPE_INDIRECT, PTYPE_RAS, PTYPE_NONE, PTYPE_MAX};

template <class Type>
class Predictor {
public:
  virtual Type predict(Waddr PC, QPointer q) = 0;
  virtual void resolve(QPointer q, Type result) = 0;
  virtual void commit(QPointer q) = 0;
  virtual void squash(QPointer first_bad) = 0;
};

typedef Predictor<bool> DirectPredictor;
typedef Predictor<Waddr> IndirectPredictor;

template <class Type>
class SquashableMap : public std::map<QPointer, Type> {
  typedef std::map<QPointer, Type> super;
public:
  typedef typename std::map<QPointer, Type>::iterator iterator;
  void squash(QPointer first_bad) {
    iterator i = super::lower_bound(first_bad); 
    for (iterator j = i ; j != super::end() ; j++) {
      assert(j->first >= first_bad);
    }
    super::erase(i, super::end());
  }
  void commit(QPointer q_ptr) {
    assert((q_ptr == 0) || (super::upper_bound(q_ptr - 1) == super::begin()));
    super::erase(q_ptr);
  }
};

class ReturnAddressStack {
public:
  ReturnAddressStack(int size);
  void push(QPointer q, Waddr return_target_rip);
  Waddr pop(QPointer q);  
  void squash(QPointer first_bad);
  void commit(QPointer q);

private:
  int rasIncrement(int index) const { return (index + 1) & (m_table.size() - 1); }

  class RasState {
  public:
    RasState() : m_next_free(0), m_top_of_stack(-1) {};
    RasState(const RasState &rhs)
      : m_next_free(rhs.m_next_free), m_top_of_stack(rhs.m_top_of_stack) {}

    int getNextFree() const { return m_next_free; }
    int getTopOfStack() const { return m_top_of_stack; }
    int setNextFree(int next_free) { m_next_free = next_free; }
    int setTopOfStack(int top_of_stack) { m_top_of_stack = top_of_stack; }

    RasState& operator=(const RasState &rhs) {
      if (this != &rhs) {
        m_next_free = rhs.m_next_free;
        m_top_of_stack = rhs.m_top_of_stack;
      }
      return *this;
    }
  private:
    int m_next_free, m_top_of_stack;
  };
  RasState m_speculative_rs, m_committed_rs;
  SquashableMap<RasState> m_inflights;
  std::vector<Waddr> m_table;
  std::vector<int> m_next_top_of_stack;
};

//! very simple direct branch predictor, which always predicts not-taken
class NotTakenPredictor : public DirectPredictor {
public:
  bool predict(Waddr rip, QPointer q_ptr) { return false; }
  void resolve(QPointer q_ptr, bool taken) {}
  void squash(QPointer first_bad) {}
  void commit(QPointer q_ptr) {}
};

//! pretty good history-based direct branch predictor
class GsharePredictor : public DirectPredictor {
public:
  typedef std::pair<int, unsigned> Record;
  GsharePredictor(unsigned history_length, unsigned table_bits) :
    m_history_length(history_length), m_num_bits(table_bits), m_table_size(1 << table_bits), 
    m_history_mask((1 << history_length) - 1), m_table(m_table_size, 1) {
    m_history = m_committed_history = 0;
  }
  bool predict(Waddr rip, QPointer q_ptr);
  void resolve(QPointer q_ptr, bool taken);
  void squash(QPointer first_bad);
  void commit(QPointer q_ptr);

private:
  unsigned hash(Waddr rip) const { 
    unsigned top_history = (m_num_bits == m_history_length) ? 0 :
      ((m_num_bits > m_history_length) ? 
       (m_history << (m_num_bits - m_history_length)) : (m_history >> (m_history_length - m_num_bits)));
    return (unsigned) ((rip << 8) ^ (rip >> 3) ^ m_history ^ top_history) & (m_table_size - 1); 
  }
  const int m_history_length, m_num_bits, m_table_size, m_history_mask;
  unsigned m_history, m_committed_history;
  SquashableMap<Record> m_inflights;
  std::vector<unsigned char> m_table;
};

//! a simple indirect branch predictor which simply hashes the rip into a table of targets
class SimpleIndirectPredictor : public IndirectPredictor {
public:
  typedef std::pair<int, Waddr> Record;
  SimpleIndirectPredictor(int num_bits) : 
    m_num_bits(num_bits), m_table_size(1 << num_bits), m_targets(m_table_size, 0) {}

  Waddr predict(Waddr rip, QPointer q_ptr);
  void resolve(QPointer q_ptr, Waddr target);
  void squash(QPointer first_bad) { m_inflights.squash(first_bad); }
  void commit(QPointer q_ptr);

private:
  int hash(Waddr rip) const { 
    return (int) ((rip >> (m_num_bits + 2)) ^ (rip >> 2)) & (m_table_size - 1); 
  }
  const int m_num_bits, m_table_size;
  SquashableMap<Record> m_inflights;
  std::vector<Waddr> m_targets;
};

class DynamicInst;

//! this class manages a collection of predictors, determining what
//! should be done to each for various operations on different types of
//! branches.
class PredictorSet {
public:
  PredictorSet();
  PredictorSet(DirectPredictor *direct, IndirectPredictor *indirect, 
               ReturnAddressStack *ras) : m_direct(direct), m_indirect(indirect), m_ras(ras) {
    for (int i = 0 ; i < (int) PTYPE_MAX ; i++) { 
      m_branches[i] = m_mispredictions[i] = 0;
    }
  }

  Waddr predict(DynamicInst *inst, Waddr fallthrough);
  void resolve(DynamicInst *inst, Waddr actual_addr);
  void squash(QPointer first_bad);
  void commit(DynamicInst *inst);

  void printAccuracies(FILE *file);
  void clearStats();
  
private:
  PredType getPredType(DynamicInst *inst);

  DirectPredictor *m_direct;
  IndirectPredictor *m_indirect;
  ReturnAddressStack *m_ras;

  W64 m_branches[PTYPE_MAX];
  W64 m_mispredictions[PTYPE_MAX];
};

#endif  /* __PREDICTOR_H */
