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

#include <simics/api.h>
#undef unlikely
#undef likely
#include <simics/arch/x86.h>
#include <simics/alloc.h>
#include <simics/utils.h>

#include "Global.h" 
#include "Address.h" 
#include "DynamicInst.h"
#include "Processor.h"
#include "RubyMemoryInterface.h"
#include "CacheMsg.h"
#include "System.h"
#include "Sequencer.h"
#include "Waiter.h"
#include "PyriteDriver.h"

extern unsigned g_L2_latency;

RubyMemoryInterface::RubyMemoryInterface(Processor *processor) : MemoryInterface(processor) {
  assert(g_system_ptr != NULL);
  dynamic_cast<PyriteDriver *>(g_system_ptr->getDriver())->addMemoryInterface(processor->getProcNum(), this);
}

void
RubyMemoryInterface::request(Waddr addr, RequestType type, Waiter *requester) {
  physical_address_t line_addr = Address(addr).getLineAddress();

  // Coalesce misses to the same address
  if (m_requested_lines.find(line_addr) != m_requested_lines.end()) {
    // Write requests must wait for outstanding read requests to complete.  Otherwise
    // it wouldn't be possible to determine which request is satisfied by a
    // subsequent line fill.  Add the write request to the read requests waitlist
    // anyway, and it will retry when the read fill occurs.
    if ((type == REQUEST_WRITE) && (m_requested_lines[line_addr]->getRequestType() == REQUEST_READ)) {
      m_miss_collisions++; 
      m_requested_lines[line_addr]->insertWaiter(new MemoryRequestRetry(this, addr, type, requester));
    } else {
      if (requester) m_requested_lines[line_addr]->insertWaiter(requester);
      m_secondary_misses++;
    }
    return;
  }

  assert(m_requested_lines.find(line_addr) == m_requested_lines.end());
  m_requested_lines.insert(std::make_pair(line_addr, new MissedLineWaitList(type)));

  CacheMsg request(Address(line_addr), ((type == REQUEST_WRITE) ? CacheRequestType_ST : CacheRequestType_LD),
                   /* local_ProgramCounter */ Address(0), AccessModeType_UserMode,
                   /* local_Size */4, PrefetchBit_No);
  assert(g_system_ptr->getSequencer(m_processor->getProcNum())->isReady(request));
  g_system_ptr->getSequencer(m_processor->getProcNum())->makeRequest(request);

  if (requester && (m_requested_lines.find(line_addr) != m_requested_lines.end())) {
    m_primary_misses++;
    m_requested_lines[line_addr]->insertWaiter(requester);
  }
  return;
}

bool
RubyMemoryInterface::ready() const {
  CacheMsg nullRequest;
  return g_system_ptr->getSequencer(m_processor->getProcNum())->isReady(nullRequest);
}
