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

//===--- PipeStages.h - tracks which instructions down pipeline --*- C++ -*--=//
//
//! This object carries indexes to a instruction_queue_t down the
//! pipeline, so we can keep track of which instructions are in which
//! stages for the in-order parts of the pipeline.
//
//===----------------------------------------------------------------------===//

#ifndef __PIPE_STAGES_H
#define __PIPE_STAGES_H
#include <vector>
#include <algorithm>
#include "Event.h"
#include "Cycleable.h"

using std::min;

template <class Type>
class PipeStages : public Cycleable {
public:
  PipeStages() : m_pipeline() { m_head = m_tail = 0; }
  PipeStages(unsigned length, Type &t) : m_pipeline(length + 1, t) { m_head = m_tail = t; }
  void configure(unsigned length, Type &t) {
    m_pipeline.clear();
    m_pipeline.insert(m_pipeline.begin(), length + 1, t);
    m_head = m_tail = t; 
  }
  unsigned length() const { return m_pipeline.size() - 1; }

  virtual void cycle() {
    m_pipeline.back() = m_tail;
    unsigned last_stage = m_pipeline.size();
    for (unsigned i = 1 ; i < last_stage ; i ++) {
      m_pipeline[i-1] = m_pipeline[i];
    }
    m_head = m_pipeline[0];
  }
  void insertTail(Type &t) { 
    assert(t >= m_tail);
    m_tail = t; 
  }
  Type readHead() const {
    return m_head;
  }
  void squash(Type first_bad) {
    Type last_good = first_bad - 1;
    m_head = min(m_head, last_good);
    m_tail = min(m_tail, last_good);
    for (unsigned i = 0 ; i < m_pipeline.size() ; i ++) {
      m_pipeline[i] = min(m_pipeline[i], last_good);
    }
  }

private:
  Type m_head, m_tail;
  std::vector<Type> m_pipeline;
};

#endif  /* __PIPE_STAGES_H */
