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

//===-- LoadStoreQueue.h - load/store queue ----------------------*- C++ -*--=//
//
//! The load queue and the store queue are used for two purposes: 1)
//! to synchronize loads with inflight stores and provide them data,
//! and 2) to detect load-store ordering violations.
//
//===----------------------------------------------------------------------===//

#ifndef __LOAD_STORE_QUEUE_H
#define __LOAD_STORE_QUEUE_H

#include <set>
#include <map>
#include <list>
#include <iostream>

class DynamicInst;
class DoubleWord;

class LoadStoreQueue {
  typedef std::list<DynamicInst *> LoadStoreList;
  typedef std::map<Waddr, LoadStoreList> LoadStoreMap;

public:
  LoadStoreQueue() {}
  ~LoadStoreQueue() {} 
  
  void reset();

  void insert(DynamicInst *m);
  void remove(DynamicInst *m);
  bool loadSearch(DynamicInst *load, const DoubleWord *mem_operand, DoubleWord *result);
  void storeSearch(DynamicInst *store);
  void invalidationSearch(Waddr line_addr);

  bool empty() const { return m_loadStoreMap.empty(); }

private:
  LoadStoreMap m_loadStoreMap;
};

#endif /* __LOAD_STORE_QUEUE_H */


