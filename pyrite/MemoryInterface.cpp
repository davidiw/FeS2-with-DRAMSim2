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

#include "MemoryInterface.h"

void
MemoryRequestRetry::wakeup() {
  Waiter *requester = 0;
  if (m_requester_waiter.waiting()) {
    requester = m_requester_waiter.getNextWaiter();
    m_requester_waiter.detach();
    assert(m_requester_waiter.detached() && requester->detached());
  }
  m_mem_interface->request(m_request_addr, m_request_type, requester);
  if (requester && requester->detached()) requester->wakeup();
  delete this;
}

void
MemoryRequestRetry::release() {
  if (m_requester_waiter.waiting()) m_requester_waiter.detach();
  delete this;
}

void
MemoryInterface::reset() {
  for (std::map<Waddr, MissedLineWaitList *>::iterator iter = m_requested_lines.begin();
       iter != m_requested_lines.end(); ++iter) {
    MissedLineWaitList *requested_line_waitlist = iter->second;
    requested_line_waitlist->releaseAll();
    delete requested_line_waitlist;
  }
  m_requested_lines.clear();
}

void
MemoryInterface::fillRequest(Waddr fill_addr) {
  if (m_requested_lines.find(fill_addr) != m_requested_lines.end()) {
    MissedLineWaitList *requested_line_waitlist = m_requested_lines[fill_addr];
    m_requested_lines.erase(fill_addr);
    requested_line_waitlist->wakeupAll();
    delete requested_line_waitlist;
  }
}

void 
MemoryInterface::releaseAllWaiters() {
  for(std::map<Waddr, MissedLineWaitList *>::const_iterator iter = m_requested_lines.begin();
      iter != m_requested_lines.end(); ++iter) {
    iter->second->releaseAll();
  }
}
