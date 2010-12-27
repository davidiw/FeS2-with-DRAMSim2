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

#include "Check.h"
#include "Sequencer.h"
#include "System.h"
#include "SubBlock.h"
#include "CacheMsg.h"

Check::Check(const Address& address, const Address& pc)
{
  m_status = TesterStatus_Idle;

  pickValue();
  pickInitiatingNode();
  changeAddress(address);
  m_pc = pc;
  m_access_mode = AccessModeType(random() % AccessModeType_NUM);
  m_store_count = 0;
}

void Check::initiate()
{
  DEBUG_MSG(TESTER_COMP, MedPrio, "initiating");
  DEBUG_EXPR(TESTER_COMP, MedPrio, *this);

  if ((random() & 0xf) == 0) { // 1 in 16 chance
    initiatePrefetch(); // Prefetch from random processor
  }

  if(m_status == TesterStatus_Idle) {
    initiateAction();
  } else if(m_status == TesterStatus_Ready) {
    initiateCheck();
  } else {
    // Pending - do nothing
    DEBUG_MSG(TESTER_COMP, MedPrio, "initiating action/check - failed: action/check is pending\n");
  }
}

void Check::initiatePrefetch(Sequencer* targetSequencer_ptr)
{
  DEBUG_MSG(TESTER_COMP, MedPrio, "initiating prefetch");
  
  CacheRequestType type;
  if ((random() & 0x7) != 0) { // 1 in 8 chance
    if ((random() & 0x1) == 0) { // 50% chance
      type = CacheRequestType_LD;
    } else {
      type = CacheRequestType_IFETCH;
    }
  } else {
    type = CacheRequestType_ST;
  }
  assert(targetSequencer_ptr != NULL);
  CacheMsg request(m_address, type, m_pc, m_access_mode, 0, PrefetchBit_Yes);
  if (targetSequencer_ptr->isReady(request)) {
    targetSequencer_ptr->makeRequest(request);
  }
}

void Check::initiatePrefetch()
{
  // Any sequencer can issue a prefetch for this address
  Sequencer* targetSequencer_ptr = g_system_ptr->getSequencer(random() % g_param_ptr->NUM_NODES());
  assert(targetSequencer_ptr != NULL); 
  initiatePrefetch(targetSequencer_ptr);
}

void Check::initiateAction()
{
  DEBUG_MSG(TESTER_COMP, MedPrio, "initiating Action");
  assert(m_status == TesterStatus_Idle);
  
  CacheRequestType type = CacheRequestType_ST;
  if ((random() & 0x1) == 0) { // 50% chance
    type = CacheRequestType_ATOMIC;
  }

  CacheMsg request(Address(m_address.getAddress()+m_store_count), type, m_pc, m_access_mode, 1, PrefetchBit_No);
  Sequencer* sequencer_ptr = initiatingSequencer();
  if (sequencer_ptr->isReady(request) == false) {
    DEBUG_MSG(TESTER_COMP, MedPrio, "failed to initiate action - sequencer not ready\n");
  } else {
    DEBUG_MSG(TESTER_COMP, MedPrio, "initiating action - successful\n");
    DEBUG_EXPR(TESTER_COMP, MedPrio, m_status);
    m_status = TesterStatus_Action_Pending;
    sequencer_ptr->makeRequest(request);
  }
  DEBUG_EXPR(TESTER_COMP, MedPrio, m_status);
}

void Check::initiateCheck()
{
  DEBUG_MSG(TESTER_COMP, MedPrio, "initiating Check");
  assert(m_status == TesterStatus_Ready);
  
  CacheRequestType type = CacheRequestType_LD;
  if ((random() & 0x1) == 0) { // 50% chance
    type = CacheRequestType_IFETCH;
  }

  CacheMsg request(m_address, type, m_pc, m_access_mode, CHECK_SIZE, PrefetchBit_No);
  Sequencer* sequencer_ptr = initiatingSequencer();
  if (sequencer_ptr->isReady(request) == false) {
    DEBUG_MSG(TESTER_COMP, MedPrio, "failed to initiate check - sequencer not ready\n");
  } else {
    DEBUG_MSG(TESTER_COMP, MedPrio, "initiating check - successful\n");
    DEBUG_MSG(TESTER_COMP, MedPrio, m_status);
    m_status = TesterStatus_Check_Pending;
    sequencer_ptr->makeRequest(request);
  }
  DEBUG_MSG(TESTER_COMP, MedPrio, m_status);
}

void Check::performCallback(NodeID proc, SubBlock& data)
{
  Address address = data.getAddress();
  //  assert(getAddress() == address);  // This isn't exactly right since we now have multi-byte checks
  assert(getAddress().getLineAddress() == address.getLineAddress());
  
  DEBUG_MSG(TESTER_COMP, MedPrio, "Callback");
  DEBUG_EXPR(TESTER_COMP, MedPrio, *this);

  if (m_status == TesterStatus_Action_Pending) {
    DEBUG_MSG(TESTER_COMP, MedPrio, "Action callback");
    // Perform store
    data.setByte(0, m_value+m_store_count); // We store one byte at a time
    m_store_count++;

    if (m_store_count == CHECK_SIZE) {
      m_status = TesterStatus_Ready;
    } else {
      m_status = TesterStatus_Idle;
    }
  } else if (m_status == TesterStatus_Check_Pending) {
    DEBUG_MSG(TESTER_COMP, MedPrio, "Check callback");
    // Perform load/check
    for(int byte_number=0; byte_number<CHECK_SIZE; byte_number++) {
      if (uint8(m_value+byte_number) != data.getByte(byte_number) && (g_param_ptr->DATA_BLOCK() == true)) {
        WARN_EXPR(proc);
        WARN_EXPR(address);
        WARN_EXPR(data);
        WARN_EXPR(byte_number);
        WARN_EXPR((int)m_value+byte_number);
        WARN_EXPR((int)data.getByte(byte_number));
        WARN_EXPR(*this);
        WARN_EXPR(g_eventQueue_ptr->getTime());
        ERROR_MSG("Action/check failure");
      }
    } 
    DEBUG_MSG(TESTER_COMP, HighPrio, "Action/check success:");
    DEBUG_EXPR(TESTER_COMP, HighPrio, *this);
    DEBUG_EXPR(TESTER_COMP, MedPrio, data);

    m_status = TesterStatus_Idle;
    pickValue();

  } else {
    WARN_EXPR(*this);
    WARN_EXPR(proc);
    WARN_EXPR(data);
    WARN_EXPR(m_status);
    WARN_EXPR(g_eventQueue_ptr->getTime());
    ERROR_MSG("Unexpected TesterStatus");
  }
  
  DEBUG_EXPR(TESTER_COMP, MedPrio, proc);
  DEBUG_EXPR(TESTER_COMP, MedPrio, data);
  DEBUG_EXPR(TESTER_COMP, MedPrio, getAddress().getLineAddress());
  DEBUG_MSG(TESTER_COMP, MedPrio, "Callback done");
  DEBUG_EXPR(TESTER_COMP, MedPrio, *this);
}

void Check::changeAddress(const Address& address)
{
  assert((m_status == TesterStatus_Idle) || (m_status == TesterStatus_Ready));
  m_status = TesterStatus_Idle;
  m_address = address;
  m_store_count = 0;
}

Sequencer* Check::initiatingSequencer() const
{
  return g_system_ptr->getSequencer(m_initiatingNode);
  //  return g_system_ptr->getSequencer(random() % g_param_ptr->NUM_NODES());
}

void Check::pickValue()
{
  assert(m_status == TesterStatus_Idle);
  m_status = TesterStatus_Idle;
  //  DEBUG_MSG(TESTER_COMP, MedPrio, m_status);
  DEBUG_MSG(TESTER_COMP, MedPrio, *this);
  m_value = random() & 0xff; // One byte
  //  DEBUG_MSG(TESTER_COMP, MedPrio, m_value);
  DEBUG_MSG(TESTER_COMP, MedPrio, *this);
  m_store_count = 0;
}

void Check::pickInitiatingNode()
{
  assert((m_status == TesterStatus_Idle) || (m_status == TesterStatus_Ready));
  m_status = TesterStatus_Idle; 
  DEBUG_MSG(TESTER_COMP, MedPrio, m_status);
  m_initiatingNode = (random() % g_param_ptr->NUM_NODES());
  DEBUG_MSG(TESTER_COMP, MedPrio, m_initiatingNode);
  m_store_count = 0;
}

void Check::print(ostream& out) const
{
  out << "[" 
      << m_address << ", value: " 
      << (int) m_value << ", status: " 
      << m_status << ", initiating node: " 
      << m_initiatingNode << ", store_count: "
      << m_store_count
      << "]" << flush;
}
