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
 
//===-- RegFile.h - logical and physical register files ---------*- C++ -*--=//
//
//! A physical register file holds the actual data, and any
//! instructions waiting for that data, while a logical file holds a
//! mapping from the architectural registers to physical registers.
//
//===----------------------------------------------------------------------===//

#ifndef __REG_FILE_H
#define __REG_FILE_H

#include <simics/api.h>
#undef unlikely
#undef likely
#include <simics/arch/x86.h>
#include <simics/alloc.h>
#include <simics/utils.h>

#include <vector>
#include <stack>

#include "globals.h" 

#include "TraceDecoder.h"
#include "Vector.h"
#include "Waiter.h"
#include "QPointer.h"

const int NUM_REGISTERS = 80;

typedef W32s PhysName;
typedef W32s LogicalName;
typedef W64 PhysReg;

class LogicalFile;

//! we keep two special register names, one for reading zeroes and one
//! for writing things that should never be read.
const PhysName REG_NULL_SRC = -1;
const PhysName REG_NULL_DEST = -2;

/*****************************************************************/
/********************** Physical Register ************************/
/*****************************************************************/

/* A physical register holds one of the inflight values associated
   with an architectural register.  The free bit indicates whether the
   register has been allocated, and the ready bit tracks whether the
   producer has produced yet. All registers are reference counted
   and are considered free when their reference count drops to 0.

   Each register also has a flags field that holds the inflight values
   associated with the EFLAGS register.
*/

/*****************************************************************/
/******************** Physical Register File *********************/
/*****************************************************************/

class PhysicalFile {
  friend class LogicalFile;
public:
  PhysicalFile(); // constructor

  void reset();

  /****************** Register Ready Information *******************/
  bool isReady(PhysName num) const {
    /* we should never read from REG_NULL_DEST or REG_NULL_SRC */
    assert(m_ref_counts[num]);
    assert((num != REG_NULL_DEST) && (num != REG_NULL_SRC));
    return m_ready[num];
  }

  /****************** Getting and Setting Values *******************/

  const PhysReg &getValue(PhysName num) const {
    assert(isReady(num));
    return m_values[num];
  }

  const PhysReg &getFlags(PhysName num) const {
    assert(isReady(num));
    return m_flags[num];
  }

  void setValue(PhysName num, PhysReg value) {
    assert((num != REG_NULL_DEST) && (num != REG_NULL_SRC));
    assert (!isReady(num) && m_ref_counts[num]);
    m_values[num] = value;
    setReady(num);
  }

  void setFlags(PhysName num, PhysReg flags) {
    assert((num != REG_NULL_DEST) && (num != REG_NULL_SRC));
    m_flags[num] = flags;
  }

  /******************* Managing Reference Counts *******************/

  void incrementRefCount(PhysName num) {
    assert((num != REG_NULL_DEST) && (num != REG_NULL_SRC));
    ++m_ref_counts[num];
  }

  void decrementRefCount(PhysName num) {
    assert((num != REG_NULL_DEST) && (num != REG_NULL_SRC));
    --m_ref_counts[num];
    if (!m_ref_counts[num]) deAllocate(num);
  }

  /******************* Waiting on Registers ************************/

  //! Add a waiter to the wait list
  void addWaiter(PhysName num, Waiter *w) {
    assert(m_ref_counts[num]);
    m_waitlists[num]->insertWaiter(w);
  }

  /******************* Who wrote this Registers *********************/
  void setWriter(PhysName num, QPointer q_ptr) { m_writers[num] = q_ptr; }
  QPointer getWriter(PhysName num) const { return m_writers[num]; }

protected:
  /******************* Allocation of Registers *********************/
  PhysName allocate();
  void deAllocate(PhysName num); 

  void setReady(PhysName i) {
    assert(m_ref_counts[i]);
    m_ready[i] = true;
    /* wakeup all instructions waiting for this instruction */
    m_waitlists[i]->wakeupAll();
  }

  /* values, writers, and wait lists */
  std::vector<PhysReg> m_values; /* physical registers */
  std::vector<PhysReg> m_flags; /*  flags registers */
  std::vector<QPointer> m_writers; /* q_pointer of instruction that wrote this reg */
  std::vector<WaitList *> m_waitlists; /* wait lists */

  /* ready bits for each register */
  std::vector<bool> m_ready;

  /* reference counts for each register */
  std::vector<PhysName> m_ref_counts;

  /* currently unallocated registers */
  std::vector<PhysName> m_free_list;
};

/*****************************************************************/
/******************** Logical Register File **********************/
/*****************************************************************/

class LogicalFile {
public:
  LogicalFile(PhysicalFile &physical) :
    m_physical(physical), m_logical(NUM_REGISTERS, REG_NULL_SRC) {}

  LogicalFile(const LogicalFile &src);

  LogicalFile& operator=(LogicalFile &rhs);

  void reset();

  PhysName getMapping(LogicalName num) const { 
    assert(num != REG_rip);
    if(num == REG_zf) {
      num = REG_flags;
    }
    return m_logical[num];
  }

  PhysName getNewMapping(LogicalName num) {
    assert(num != REG_rip);
    if(num == REG_zf) {
      num = REG_flags;
    }
    PhysName new_preg = m_physical.allocate();
    setMapping(num, new_preg);
    return new_preg;
  }

  void setMapping(LogicalName num, PhysName phys_reg)  { 
    assert(num != REG_rip);
    if(num == REG_zf) {
      num = REG_flags;
    }
    if((m_logical[num] != REG_NULL_DEST) && (m_logical[num] != REG_NULL_SRC)) {
      m_physical.decrementRefCount(m_logical[num]);
    }
    m_physical.incrementRefCount(phys_reg);
    m_logical[num] = phys_reg;
  }

  bool operator==(LogicalFile &rhs);
  //void MarkAllocatedRegs(bool *free);

  PhysicalFile& getPhysicalFile() const { return m_physical; }

private:  
  PhysicalFile& m_physical;
  /* index from logical register to physical register */
  std::vector<PhysName> m_logical;  
};

#endif /* __REG_FILE_H */
