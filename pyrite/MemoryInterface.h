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

//===-- MemoryInterface.h - Abstract memory model interface ------*- C++ -*--=//
//
//! The MemoryInterface class specifies the interface between the core processor
//! model and the memory system model.  It includes default definitions of
//! all of the interface except for 'request', including supporting classes.
//
//===----------------------------------------------------------------------===//

#ifndef __MEMORY_INTERFACE
#define __MEMORY_INTERFACE

#include <map>
#include "Waiter.h"

class MemoryInterface;
class Processor;

enum RequestType {NO_REQUEST, REQUEST_READ, REQUEST_WRITE};

class MemoryRequestRetry : public Waiter {
public:
  MemoryRequestRetry(MemoryInterface *mem_interface, Waddr request_addr,
                     RequestType request_type, Waiter *requester)
    : Waiter(), m_mem_interface(mem_interface), m_request_addr(request_addr),
      m_request_type(request_type), m_requester_waiter() {
    if (requester) {
      m_requester_waiter.insert(requester);
      assert(!requester->detached());
    }
  }

  void wakeup();
  void release();

private:
  MemoryInterface *m_mem_interface;
  Waddr m_request_addr;
  RequestType m_request_type;
  Waiter m_requester_waiter;
};

class MissedLineWaitList : public WaitList {
public:
  MissedLineWaitList(RequestType request_type) : m_request_type(request_type) {};

  RequestType getRequestType() const { return m_request_type; }
private:
  RequestType m_request_type;
};

class MemoryInterface {
public:
  MemoryInterface(Processor *processor)
    : m_processor(processor), m_requested_lines(),
      m_primary_misses(0), m_secondary_misses(0), m_miss_collisions(0) {};

  virtual void reset();

  virtual void fillRequest(Waddr a);
  virtual void request(Waddr addr, RequestType type, Waiter *requester) = 0;
  virtual bool ready() const { return true; }

  virtual void releaseAllWaiters();

protected:
  Processor *m_processor;

  std::map<Waddr, MissedLineWaitList *> m_requested_lines;

  /* stats */
  unsigned m_primary_misses;
  unsigned m_secondary_misses;
  unsigned m_miss_collisions;
};

#endif //__MEMORY_INTERFACE
