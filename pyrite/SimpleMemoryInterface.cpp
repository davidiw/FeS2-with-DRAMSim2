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

#include "Processor.h"
#include "SimpleMemoryInterface.h"
#include "cache.h"
#include "Address.h"
#include "stats.h"
#include "params.h"

extern unsigned g_L1D_latency;
extern unsigned g_L2_latency;
extern unsigned g_mem_latency;

cache_t *g_l2_cache = 0;

// l1 data cache l1 block miss handler function, returns the latency of a block access
unsigned int l1d_access_fun(enum mem_cmd cmd /* access cmd, Read or Write */,
                            md_addr_t baddr /* block address to access */,
                            int bsize /* size of block to access */,
                            struct cache_blk_t *blk /* ptr to block in upper level */,
                            tick_t now /* time of access */) {
  g_stats.incrementL1Misses(0); /* hardcoded to proc 0 stats */
  unsigned int lat;
  cmd = Read;  // currently treat writes like reads

  if (g_l2_cache) {
    /* access next level of data cache hierarchy */
    lat = cache_access(g_l2_cache, cmd, baddr, NULL, bsize,
                       /* now */now, /* pudata */NULL, /* repl addr */NULL);
    if (cmd == Read)
      return lat;
    else {
      /* FIXME: unlimited write buffers */
      return 0;
    }
  } else {
    /* access main memory */
    if (cmd == Read) {
      return g_mem_latency;
    } else {
      /* FIXME: unlimited write buffers */
      return 0;
    }
  }
}

// l2 cache block miss handler function, returns the latency of a block access
unsigned int l2_access_fun(enum mem_cmd cmd /* access cmd, Read or Write */,
                           md_addr_t baddr /* block address to access */,
                           int bsize /* size of block to access */,
                           struct cache_blk_t *blk /* ptr to block in upper level */,
                           tick_t now /* time of access */) {
  g_stats.incrementL2Misses(0); /* hardcoded to proc 0 stats */
  /* this is a miss to the lowest level, so access main memory */
  cmd = Read;  // currently treat writes like reads
  if (cmd == Read) {
    return g_mem_latency;
  } else {
    /* FIXME: unlimited write buffers */
    return 0;
  }
}

void
SimpleMissWaiter::wakeup() {
  m_mem_interface->fillRequest(m_line_addr);
  delete this;
}

void
SimpleMissWaiter::release() {
  m_mem_interface->releaseRequest(m_line_addr);
  delete this;
}

SimpleMemoryInterface::SimpleMemoryInterface(Processor *processor) : MemoryInterface(processor) {
  assert(!g_l2_cache);
  m_l1_dcache = cache_create("l1d", (1 << g_params.getL1DataNumSetBits()), 64, 0, 0, g_params.getL1DataAssoc(), LRU, l1d_access_fun, g_L1D_latency);
  g_l2_cache = cache_create("l2", (1 << g_params.getL2NumSetBits()), 64, 0, 0, g_params.getL2Assoc(),  LRU, l2_access_fun, g_L2_latency);
}

void
SimpleMemoryInterface::reset() {
  cache_mark_all_ready(m_l1_dcache);
  cache_mark_all_ready(g_l2_cache);
  MemoryInterface::reset();
}

void
SimpleMemoryInterface::request(Waddr addr, RequestType type, Waiter *requester) {
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

  unsigned int latency = cache_access(m_l1_dcache, (type == REQUEST_READ) ? Read : Write,
                                      line_addr, NULL, 1, m_processor->getCurrentCycle(), NULL, NULL);
  if (latency > 0) {
    m_primary_misses++;
    m_requested_lines.insert(std::make_pair(line_addr, new MissedLineWaitList(type)));
    if (requester) m_requested_lines[line_addr]->insertWaiter(requester);
    m_processor->getEventsQueue().insert(new SimpleMissWaiter(this, line_addr), latency);
  }
}

void
SimpleMemoryInterface::releaseRequest(Waddr request_addr) {
  if (m_requested_lines.find(request_addr) != m_requested_lines.end()) {
    MissedLineWaitList *requested_line_waitlist = m_requested_lines[request_addr];
    m_requested_lines.erase(request_addr);
    requested_line_waitlist->releaseAll();
    delete requested_line_waitlist;
  }
}

void
SimpleMemoryInterface::initHitLatencies() {
  m_l1_dcache->hit_latency = g_L1D_latency;
  g_l2_cache->hit_latency = g_L2_latency;
}
