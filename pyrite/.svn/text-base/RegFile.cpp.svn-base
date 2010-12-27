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
 
//===----------------------------------------------------------------------===//
//
//! A physical register file holds the actual data, and any
//! instructions waiting for that data, while a logical file holds a
//! mapping from the architectural registers to physical registers.
//
//===----------------------------------------------------------------------===//

#include "RegFile.h"
#include "Processor.h"
#include <algorithm>

/*****************************************************************/
/******************** Physical Register File *********************/
/*****************************************************************/

PhysicalFile::PhysicalFile() {
}

PhysName 
PhysicalFile::allocate() {
  if (!m_free_list.empty()) {
    //! if the stack isn't empty, pop one off the stack
    PhysName ret_val = m_free_list.back();
    assert(!m_ready[ret_val]);
    m_free_list.pop_back();
    return ret_val;
  }
  //! otherwise allocate & initialize a new one
  PhysName name = m_values.size();
  assert(m_values.size() == m_ready.size());
  PhysReg temp = 0;
  m_values.push_back(temp);
  m_flags.push_back(temp);
  m_writers.push_back(0);
  m_waitlists.push_back(new WaitList);
  m_ready.push_back(false);
  m_ref_counts.push_back(0);
  assert(m_values.size() == m_ready.size());
  return name;
}

void 
PhysicalFile::deAllocate(PhysName num) {
  assert(m_ref_counts[num] == 0);
  assert(m_waitlists[num]->empty());
  m_writers[num] = 0;
  m_ready[num] = false;
  m_free_list.push_back(num);
}

void
PhysicalFile::reset() {
  // Be careful not to leak memory!
  m_values.clear();
  m_flags.clear();
  m_writers.clear();
  m_ready.clear();
  m_ref_counts.clear();
  m_free_list.clear();
  for(std::vector<WaitList*>::iterator i = m_waitlists.begin();
      i != m_waitlists.end(); ++i) {
    delete *i;
  }
  m_waitlists.clear();
}

/*****************************************************************/
/******************** Logical Register File***********************/
/*****************************************************************/

LogicalFile::LogicalFile(const LogicalFile &src)
  : m_physical(src.m_physical), m_logical(src.m_logical) {
  for(int i = 0; i != m_logical.size(); ++i) {
    if((m_logical[i] != REG_NULL_DEST) && (m_logical[i] != REG_NULL_SRC)) {
      m_physical.incrementRefCount(m_logical[i]);
    }
  }
}

LogicalFile&
LogicalFile::operator=(LogicalFile &rhs) {
  assert(&m_physical == &rhs.m_physical);
  if(this != &rhs) {
    for(int i = 0; i != m_logical.size(); ++i) {
      if((m_logical[i] != REG_NULL_DEST) && (m_logical[i] != REG_NULL_SRC)) {
        m_physical.decrementRefCount(m_logical[i]);
      }
    }

    m_logical = rhs.m_logical;

    for(int i = 0; i != m_logical.size(); ++i) {
      if((m_logical[i] != REG_NULL_DEST) && (m_logical[i] != REG_NULL_SRC)) {
        m_physical.incrementRefCount(m_logical[i]);
      }
    }
  }
}

void
LogicalFile::reset() {
  m_logical.clear();
  m_logical.resize(NUM_REGISTERS, REG_NULL_SRC);
}

bool
LogicalFile::operator==(LogicalFile &rhs) {
  assert(&m_physical == &rhs.m_physical);
  return (m_logical == rhs.m_logical);
}

