/* This file is part of the Ruby Multiprocessor Memory System
   Simulator, a component of the Multifacet GEMS (General
   Execution-driven Multiprocessor Simulator) software toolset
   originally developed at the University of Wisconsin-Madison.

   Ruby was originally developed primarily by Milo Martin and Daniel
   Sorin with contributions from Ross Dickson, Carl Mauer, and Manoj
   Plakal.

   Substantial further development of Multifacet GEMS at the
   University of Wisconsin was performed by Alaa Alameldeen, Brad
   Beckmann, Ross Dickson, Pacia Harper, Milo Martin, Michael Marty,
   Carl Mauer, Kevin Moore, Manoj Plakal, Daniel Sorin, Min Xu, and
   Luke Yen.

   Additional development was performed at the University of
   Pennsylvania by Milo Martin.

   --------------------------------------------------------------------

   Copyright (C) 1999-2005 by Mark D. Hill and David A. Wood for the
   Wisconsin Multifacet Project.  Contact: gems@cs.wisc.edu
   http://www.cs.wisc.edu/gems/

   This file is based upon a pre-release version of Multifacet GEMS
   from 2004 and may also contain additional modifications and code
   Copyright (C) 2004-2007 by Milo Martin for the Penn Architecture
   and Compilers Group.  Contact: acg@lists.seas.upenn.edu
   http://www.cis.upenn.edu/acg/

   --------------------------------------------------------------------

   Multifacet GEMS is free software; you can redistribute it and/or
   modify it under the terms of version 2 of the GNU General Public
   License as published by the Free Software Foundation.  This 
   software comes with ABSOLUTELY NO WARRANTY.  For details see the 
   file LICENSE included with the distribution.
*/
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

#include "Global.h"
#include "Sequencer.h"
#include "System.h"
#include "Profiler.h"
#include "CacheMemory.h"
#include "Tracer.h"
#include "Tester.h"
#include "SubBlock.h"
#include "CacheMsg.h"
#include "Map.h"
#include "MessageBuffer.h"

Sequencer::Sequencer(NodeID id, MessageBuffer* request_queue_ptr, CacheMemory* primary_data_cache_ptr, CacheMemory* primary_insn_cache_ptr) 
{
  m_id = id;
  assert(request_queue_ptr != NULL);
  assert(primary_data_cache_ptr != NULL);
  assert(primary_insn_cache_ptr != NULL);

  m_request_queue_ptr = request_queue_ptr;
  m_primary_data_cache_ptr = primary_data_cache_ptr;
  m_primary_insn_cache_ptr = primary_insn_cache_ptr;

  m_outstanding_count = 0;
  m_deadlock_check_scheduled = false;
  m_writeRequestTable_ptr = new Map<Address, CacheMsg>;
  m_readRequestTable_ptr = new Map<Address, CacheMsg>;
}

Sequencer::~Sequencer() 
{
  delete m_writeRequestTable_ptr;
  delete m_readRequestTable_ptr;
}

void Sequencer::wakeup() 
{
  // Check for deadlock of any of the requests
  Time current_time = g_eventQueue_ptr->getTime();
  int outstanding = 0;

  {
    Vector<Address> keys = m_readRequestTable_ptr->keys();
    for (int i=0; i<keys.size(); i++) {
      CacheMsg& request = m_readRequestTable_ptr->lookup(keys[i]);
      if (current_time - request.getTime() >= g_param_ptr->DEADLOCK_THRESHOLD()) {
        WARN_MSG("Possible Deadlock detected");
        WARN_EXPR(request);
        WARN_EXPR(m_id);
        WARN_EXPR(current_time);
        WARN_EXPR(current_time - request.getTime());
        ERROR_MSG("Aborted");
      }
    }
  }

  {
    Vector<Address> keys = m_writeRequestTable_ptr->keys();
    for (int i=0; i<keys.size(); i++) {
      CacheMsg& request = m_writeRequestTable_ptr->lookup(keys[i]);
      if (current_time - request.getTime() >= g_param_ptr->DEADLOCK_THRESHOLD()) {
        WARN_MSG("Possible Deadlock detected");
        WARN_EXPR(request);
        WARN_EXPR(m_id);
        WARN_EXPR(current_time);
        WARN_EXPR(request.getTime());
        WARN_EXPR(current_time - request.getTime());
        ERROR_MSG("Aborted");
      }
    }
  }

  assert(m_outstanding_count == (m_writeRequestTable_ptr->size() + m_readRequestTable_ptr->size()));

  if (m_outstanding_count > 0) { // If there are still outstanding requests, keep checking
    g_eventQueue_ptr->scheduleEvent(this, g_param_ptr->DEADLOCK_THRESHOLD());
  } else {
    m_deadlock_check_scheduled = false;
  }
}

// Insert the request on the correct request table.  Return true if
// the entry was already present.
bool Sequencer::insertRequest(const CacheMsg& request)
{
  assert(m_outstanding_count == (m_writeRequestTable_ptr->size() + m_readRequestTable_ptr->size()));
  
  // See if we should schedule a deadlock check
  if (m_deadlock_check_scheduled == false) {
    g_eventQueue_ptr->scheduleEvent(this, g_param_ptr->DEADLOCK_THRESHOLD());
    m_deadlock_check_scheduled = true;
  }

  if ((request.getType() == CacheRequestType_ST) || 
      (request.getType() == CacheRequestType_ATOMIC)) {
    if (m_writeRequestTable_ptr->exist(line_address(request.getAddress()))) {
      m_writeRequestTable_ptr->lookup(line_address(request.getAddress())) = request;
      return true;
    }
    m_writeRequestTable_ptr->allocate(line_address(request.getAddress()));
    m_writeRequestTable_ptr->lookup(line_address(request.getAddress())) = request;
  } else {
    if (m_readRequestTable_ptr->exist(line_address(request.getAddress()))) {
      m_readRequestTable_ptr->lookup(line_address(request.getAddress())) = request;
      return true;
    }
    m_readRequestTable_ptr->allocate(line_address(request.getAddress()));
    m_readRequestTable_ptr->lookup(line_address(request.getAddress())) = request;
  }

  m_outstanding_count++;
  g_system_ptr->getProfiler()->sequencerRequests(m_outstanding_count);

  assert(m_outstanding_count == (m_writeRequestTable_ptr->size() + m_readRequestTable_ptr->size()));
  return false;
}

void Sequencer::removeRequest(const CacheMsg& request)
{
  assert(m_outstanding_count == (m_writeRequestTable_ptr->size() + m_readRequestTable_ptr->size()));
  if ((request.getType() == CacheRequestType_ST) || 
      (request.getType() == CacheRequestType_ATOMIC)) {
    m_writeRequestTable_ptr->deallocate(line_address(request.getAddress()));
  } else {
    m_readRequestTable_ptr->deallocate(line_address(request.getAddress()));
  }
  m_outstanding_count--;
  assert(m_outstanding_count == (m_writeRequestTable_ptr->size() + m_readRequestTable_ptr->size()));
}

void Sequencer::writeCallback(const Address& address, DataBlock& data)
{
  assert(address == line_address(address));
  assert(m_writeRequestTable_ptr->exist(line_address(address)));

  CacheMsg request = m_writeRequestTable_ptr->lookup(address);
  removeRequest(request);

  assert((request.getType() == CacheRequestType_ST) || 
         (request.getType() == CacheRequestType_ATOMIC));

  hitCallback(request, data);
}

void Sequencer::readCallback(const Address& address, DataBlock& data)
{
  assert(address == line_address(address));
  assert(m_readRequestTable_ptr->exist(line_address(address)));

  CacheMsg request = m_readRequestTable_ptr->lookup(address);
  removeRequest(request);

  assert((request.getType() == CacheRequestType_LD) || 
         (request.getType() == CacheRequestType_IFETCH));

  hitCallback(request, data);
}

void Sequencer::hitCallback(const CacheMsg& request, DataBlock& data)
{
  int size = request.getSize();
  Address request_address = request.getAddress();
  Address request_line_address = line_address(request_address);
  CacheRequestType type = request.getType();
  Time issued_time = request.getTime();

  // Set this cache entry to the most recently used
  if (type == CacheRequestType_IFETCH) {
    if (m_primary_insn_cache_ptr->isTagPresent(request_line_address)) {
      m_primary_insn_cache_ptr->setMRU(request_line_address);
    }
  } else {
    if (m_primary_data_cache_ptr->isTagPresent(request_line_address)) {
      m_primary_data_cache_ptr->setMRU(request_line_address);
    }
  }

  // Profile the miss latency
  assert(g_eventQueue_ptr->getTime() >= issued_time);
  Time miss_latency = g_eventQueue_ptr->getTime() - issued_time;
  if (miss_latency != 0) {
    g_system_ptr->getProfiler()->missLatency(miss_latency, type);

    if (g_param_ptr->PROTOCOL_DEBUG_TRACE()) {
      g_system_ptr->getProfiler()->profileTransition("Seq", m_id, request.getAddress(), 
                                                     "", "Done", "", int_to_string(miss_latency)+" cycles");
    }
  }

  if (request.getPrefetch() == PrefetchBit_Yes) {
    return; // Ignore the prefetch
  }
  
  // Copy the correct bytes out of the cache line into the subblock
  SubBlock subblock(request_address, size);
  subblock.mergeFrom(data);  // copy the correct bytes from DataBlock in the SubBlock
  
  // Call into the Driver (Tester or Simics) and let it read and/or modify the sub-block
  g_system_ptr->getDriver()->hitCallback(m_id, subblock);
  
  // If the request was a Store or Atomic, apply the changes in the SubBlock to the DataBlock
  if ((type == CacheRequestType_ST) || (type == CacheRequestType_ATOMIC)) {
    subblock.mergeTo(data);    // copy the correct bytes from SubBlock into the DataBlock
  }
}

// Returns true if the sequencer already has a load or store outstanding
bool Sequencer::isReady(const CacheMsg& request) const 
{
  if (m_outstanding_count >= g_param_ptr->SEQUENCER_OUTSTANDING_REQUESTS()) {
    return false;
  }

  // This code allows reads to be performed even when we have a write
  // request outstanding for the line
  if ((request.getType() == CacheRequestType_ST) || (request.getType() == CacheRequestType_ATOMIC)) {
    if (m_writeRequestTable_ptr->exist(line_address(request.getAddress()))) {
      return false;
    }
  } else {
    if (m_readRequestTable_ptr->exist(line_address(request.getAddress()))) {
      return false;
    }
  }
  return true;
}

// Called by Driver (Simics or Tester).
void Sequencer::makeRequest(const CacheMsg& request) 
{
  assert(isReady(request));
  bool write = (request.getType() == CacheRequestType_ST) || (request.getType() == CacheRequestType_ATOMIC);

  bool hit = doRequest(request);
  
  // Issue prefetches, but don't let a prefetch trigger a prefetch
  if (!hit && request.getPrefetch() == PrefetchBit_No) {
    
    // We misses on an Ifetch, so we perform next line ahead instruction prefetching
    CacheMsg new_request = request;
    if (request.getType() == CacheRequestType_IFETCH) {
      Address next_line(line_address(request.getAddress()));
      int offset = 0;
      for (int i=0; i<g_param_ptr->INSTR_PREFETCH_NUM_LINES(); i++) {
        offset += (1 << g_param_ptr->DATA_BLOCK_BITS());
        new_request.getAddress() = Address(next_line.getAddress() + offset);
        new_request.getPrefetch() = PrefetchBit_Yes;
        if (isReady(new_request)) { 
          makeRequest(new_request);
        }
      }
    }
  }
}

bool Sequencer::doRequest(const CacheMsg& request) 
{
  // Check the fast path
  DataBlock* data_ptr;
  bool hit = tryCacheAccess(line_address(request.getAddress()), request.getType(), request.getProgramCounter(), request.getAccessMode(), request.getSize(), data_ptr);
  if (hit) {
    DEBUG_MSG(SEQUENCER_COMP, MedPrio, "Fast path hit");
    ASSERT(data_ptr != NULL);
    hitCallback(request, *data_ptr);
    return true;
  } 

  issueRequest(request);
  return false;
}

void Sequencer::issueRequest(const CacheMsg& request) 
{
  bool found = insertRequest(request);

  if (!found) {
    CacheMsg msg = request;
    msg.getAddress() = line_address(request.getAddress()); // Make line address

    // L1 misses are profiled here - L2 misses are profiled within the generated protocol code
    g_system_ptr->getProfiler()->addPrimaryStatSample(msg, m_id);

    if (g_param_ptr->PROTOCOL_DEBUG_TRACE()) {
      g_system_ptr->getProfiler()->profileTransition("Seq", m_id, msg.getAddress(),
                                                     "", "Begin", "", "");
    }

    if (g_system_ptr->getTracer()->traceEnabled()) {
      g_system_ptr->getTracer()->traceRequest(m_id, msg.getAddress(), msg.getProgramCounter(), 
                                              msg.getType(), g_eventQueue_ptr->getTime());
    }
    
    // Send the message to the cache controller
    int random_perturbation = 1+(random() % 3); // Average of 2
    Time latency = g_param_ptr->SEQUENCER_TO_CONTROLLER_LATENCY() + random_perturbation;
    m_request_queue_ptr->enqueue(msg, latency);

    DEBUG_MSG(SEQUENCER_COMP, HighPrio, "Issued request");
    DEBUG_EXPR(SEQUENCER_COMP, HighPrio, m_id);
    DEBUG_EXPR(SEQUENCER_COMP, HighPrio, msg);
  }
}

bool Sequencer::tryCacheAccess(const Address& addr, CacheRequestType type,
                               const Address& pc, AccessModeType access_mode,
                               int size, DataBlock*& data_ptr) 
{
  if (type == CacheRequestType_IFETCH) {
    return m_primary_insn_cache_ptr->tryCacheAccess(line_address(addr), type, data_ptr);
  } else {
    return m_primary_data_cache_ptr->tryCacheAccess(line_address(addr), type, data_ptr);
  }
}

void Sequencer::print(ostream& out) const
{
  out << "[Sequencer: " << m_id 
      << ", outstanding requests: " << m_outstanding_count 
      << ", read request table: " << *m_readRequestTable_ptr
      << ", write request table: " << *m_writeRequestTable_ptr
      << "]";
}
