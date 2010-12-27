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

#ifndef __SIMPLE_MEMORY_INTERFACE
#define __SIMPLE_MEMORY_INTERFACE

#include "MemoryInterface.h"
#include "Waiter.h"
#include "Event.h"

class Processor;
class SimpleMemoryInterface;
struct cache_t;

class SimpleMissWaiter : public Waiter {
public:
  SimpleMissWaiter(SimpleMemoryInterface *mem_interface, Waddr line_addr)
    : m_mem_interface(mem_interface), m_line_addr(line_addr) {};
  void wakeup();
  void release();
private:
  SimpleMemoryInterface *m_mem_interface;
  Waddr m_line_addr;
};

class SimpleMemoryInterface : public MemoryInterface {
public:
  SimpleMemoryInterface(Processor *processor);

  void releaseRequest(Waddr line_addr);
  void initHitLatencies();

  // MemoryInterface implementations
  void reset();
  void request(Waddr addr, RequestType type, Waiter *requester);
private:
  cache_t *m_l1_dcache;
};

#endif /* __SIMPLE_MEMORY_INTERFACE */
