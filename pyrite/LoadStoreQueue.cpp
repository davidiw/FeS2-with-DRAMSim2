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

//===--LoadStoreQueue.cpp - load/store queue ---------------------*- C++ -*--=//
//
// The load queue and the store queue are used for two purposes: 1)
// to synchronize loads with inflight stores and provide them data,
// and 2) to detect load-store ordering violations.
//
//===----------------------------------------------------------------------===//

#include "ptlhwdef.h"
#include "DynamicInst.h"
#include "Address.h"
#include "LoadStoreQueue.h"

void 
LoadStoreQueue::reset() {  // called when we flush the whole pipeline.  ("Nuke it from orbit... It's the only way to be sure...")
  for (LoadStoreMap::iterator i = m_loadStoreMap.begin() ; i != m_loadStoreMap.end() ; ++ i) {
    i->second.clear();
  }
  m_loadStoreMap.clear();
}

void 
LoadStoreQueue::insert(DynamicInst *inst) {
  assert(isstore(inst->getOpcode()) || isload(inst->getOpcode()));

  Waddr alignedAddr = inst->getMemOperand()->alignedAddr();
  LoadStoreList &list = m_loadStoreMap[alignedAddr];

  // walk the list until iterator points to an element earlier than "inst"
  LoadStoreList::iterator list_i = list.begin();
  while ((list_i != list.end()) && ((*list_i)->getQPointer() > inst->getQPointer())) { 
    ++ list_i; 
  }

  // insert the element
  list.insert(list_i, inst);
}

void 
LoadStoreQueue::remove(DynamicInst *m) {
  Waddr alignedAddr = m->getMemOperand()->alignedAddr();

  // find the list and remove all matching elements (a little inefficient...)
  LoadStoreList &list = m_loadStoreMap[alignedAddr];
  list.remove(m);

  // if the list is empty, go ahead and remove the list
  if (list.empty()) {
    m_loadStoreMap.erase(alignedAddr);
  }
}

bool
LoadStoreQueue::loadSearch(DynamicInst *load, const DoubleWord *mem_operand, DoubleWord *result) {
  LoadStoreList &list = m_loadStoreMap[mem_operand->alignedAddr()];

  // walk the list backwards (increasing q_pointers) until iterator finds itself (element "load")
  LoadStoreList::reverse_iterator list_ri = list.rbegin();
  while (*list_ri != load) {
    if (isstore((*list_ri)->getOpcode())) {
      unsigned index = 0;
      if (!(*list_ri)->isMemoryIssued()) { // store hasn't issued 
        if (mem_operand->overlaps(*(*list_ri)->getMemOperand())) { 
          (*list_ri)->insertStoreWaiter(load);
          return false;
        }
      }
      (*list_ri)->getMemOperand()->overlay(*result);
    }
    ++ list_ri; 
  }
  return true;
}

void
LoadStoreQueue::storeSearch(DynamicInst *store) {
  DynamicInst *oldestIssuedLoad = 0;

  Waddr alignedAddr = store->getMemOperand()->alignedAddr();

  DoubleWord dw(*store->getMemOperand());
  LoadStoreList &list = m_loadStoreMap[alignedAddr];

  // walk the list backwards (increasing q_pointers) until iterator finds element "store"
  LoadStoreList::reverse_iterator list_ri = list.rbegin();
  while (*list_ri != store) { ++ list_ri; }
	 
  while (list_ri != list.rend()) { 
    if (isstore((*list_ri)->getOpcode())) {
      (*list_ri)->getMemOperand()->overlay(dw);
    } else {
      assert(isload((*list_ri)->getOpcode()));
      if ((*list_ri)->isMemoryIssued() &&   // if it has grabbed a value
          !(*list_ri)->getMemOperand()->overlapMatches(dw)) { // and that value is wrong
        if ((oldestIssuedLoad == 0) || (oldestIssuedLoad->getQPointer() > (*list_ri)->getQPointer())) {
          oldestIssuedLoad = *list_ri;
        }
        break;
      }
    }
    ++ list_ri;
  }

  if (oldestIssuedLoad) {
    oldestIssuedLoad->setLoadOrderingViolation();
    store->recordEvent(inst_record_t::ORDERING_VIOLATION_S);
  }
}

void
LoadStoreQueue::invalidationSearch(Waddr line_addr) {
  DynamicInst *oldestIssuedLoad = 0;

  // An entire cache line has been invalidated, so check each 8-byte block inside it to
  // see which loads are affected (if any).
  assert(Address(line_addr).isBlockAligned());
  for(Waddr block_addr = line_addr; block_addr < (line_addr + ADDRESS_WIDTH); block_addr += 8) {
    LoadStoreList &list = m_loadStoreMap[block_addr];
    // walk the entire list backwards (increasing q_pointers) and invalidate all loads
    for(LoadStoreList::reverse_iterator list_ri = list.rbegin(); list_ri != list.rend(); ++list_ri) {
      if (isload((*list_ri)->getOpcode()) &&
          (*list_ri)->isMemoryIssued()) {   // if load has grabbed a value
        assert((*list_ri)->getMemOperand()->getSize() != 0);
        if ((oldestIssuedLoad == 0) || (oldestIssuedLoad->getQPointer() > (*list_ri)->getQPointer())) {
          oldestIssuedLoad = *list_ri;
        }
      }
    }
  }

  if (oldestIssuedLoad) {
    oldestIssuedLoad->setLoadConsistencyViolation();
  }
}
