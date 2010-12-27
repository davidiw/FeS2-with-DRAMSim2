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

#include "RequestGenerator.h"
#include "RequestGeneratorStatus.h"
#include "LockStatus.h"
#include "Sequencer.h"
#include "System.h"
#include "SubBlock.h"
#include "SyntheticDriver.h"
#include "CacheMsg.h"

RequestGenerator::RequestGenerator(NodeID node, SyntheticDriver& driver) :
  m_driver(driver)
{
  m_status = RequestGeneratorStatus_Thinking;
  m_last_transition = 0;
  m_node = node;
  pickAddress();
  m_counter = 0;

  g_eventQueue_ptr->scheduleEvent(this, 1+(random() % 200));
}

RequestGenerator::~RequestGenerator()
{
}

void RequestGenerator::wakeup()
{
  DEBUG_EXPR(TESTER_COMP, MedPrio, m_node);
  DEBUG_EXPR(TESTER_COMP, MedPrio, m_status);

  if (m_status == RequestGeneratorStatus_Thinking) {
    m_status = RequestGeneratorStatus_Test_Pending;
    m_last_transition = g_eventQueue_ptr->getTime();
    initiateTest();  // Test
  } else if (m_status == RequestGeneratorStatus_Holding) {
    m_status = RequestGeneratorStatus_Release_Pending;
    m_last_transition = g_eventQueue_ptr->getTime();
    initiateRelease();  // Release
  } else if (m_status == RequestGeneratorStatus_Before_Swap) {
    m_status = RequestGeneratorStatus_Swap_Pending;
    m_last_transition = g_eventQueue_ptr->getTime();
    initiateSwap();
  } else {
    WARN_EXPR(m_status);
    ERROR_MSG("Invalid status");
  }
}

void RequestGenerator::performCallback(NodeID proc, SubBlock& data)
{
  Address address = data.getAddress();
  assert(proc == m_node);
  assert(address == m_address);  

  DEBUG_EXPR(TESTER_COMP, LowPrio, proc);
  DEBUG_EXPR(TESTER_COMP, LowPrio, m_status);
  DEBUG_EXPR(TESTER_COMP, LowPrio, address);
  DEBUG_EXPR(TESTER_COMP, LowPrio, data);

  if (m_status == RequestGeneratorStatus_Test_Pending) { 
    //    m_driver.recordTestLatency(g_eventQueue_ptr->getTime() - m_last_transition);
    if (data.readByte() == LockStatus_Locked) {
      // Locked - keep spinning
      m_status = RequestGeneratorStatus_Thinking;
      m_last_transition = g_eventQueue_ptr->getTime();
      g_eventQueue_ptr->scheduleEvent(this, waitTime());
    } else {
      // Unlocked - try the swap
      m_driver.recordTestLatency(g_eventQueue_ptr->getTime() - m_last_transition);
      m_status = RequestGeneratorStatus_Before_Swap;
      m_last_transition = g_eventQueue_ptr->getTime();
      g_eventQueue_ptr->scheduleEvent(this, waitTime());
    }
  } else if (m_status == RequestGeneratorStatus_Swap_Pending) {
    m_driver.recordSwapLatency(g_eventQueue_ptr->getTime() - m_last_transition);
    if (data.readByte() == LockStatus_Locked) {
      // We failed to aquire the lock
      m_status = RequestGeneratorStatus_Thinking;
      m_last_transition = g_eventQueue_ptr->getTime();
      g_eventQueue_ptr->scheduleEvent(this, waitTime());
    } else {
      // We acquired the lock
      data.writeByte(LockStatus_Locked);
      m_status = RequestGeneratorStatus_Holding;
      m_last_transition = g_eventQueue_ptr->getTime();
      DEBUG_MSG(TESTER_COMP, HighPrio, "Acquired");
      DEBUG_EXPR(TESTER_COMP, HighPrio, proc);
      DEBUG_EXPR(TESTER_COMP, HighPrio, g_eventQueue_ptr->getTime());
      g_eventQueue_ptr->scheduleEvent(this, holdTime());
    }
  } else if (m_status == RequestGeneratorStatus_Release_Pending) {
    m_driver.recordReleaseLatency(g_eventQueue_ptr->getTime() - m_last_transition);
    // We're releasing the lock
    data.writeByte(LockStatus_Unlocked);

    m_counter++;
    if (m_counter < g_param_ptr->TESTER_LENGTH()) {
      m_status = RequestGeneratorStatus_Thinking;
      m_last_transition = g_eventQueue_ptr->getTime();
      pickAddress();
      g_eventQueue_ptr->scheduleEvent(this, thinkTime());
    } else {
      m_driver.reportDone();
      m_status = RequestGeneratorStatus_Done;
      m_last_transition = g_eventQueue_ptr->getTime();
    } 
  } else {
    WARN_EXPR(m_status);
    ERROR_MSG("Invalid status");
  }
}

int RequestGenerator::thinkTime() const
{
  return g_param_ptr->THINK_TIME();
}

int RequestGenerator::waitTime() const
{
  return g_param_ptr->WAIT_TIME();
}

int RequestGenerator::holdTime() const
{
  return g_param_ptr->HOLD_TIME();
}

void RequestGenerator::pickAddress()
{
  assert(m_status == RequestGeneratorStatus_Thinking);
  m_address = m_driver.pickAddress(m_node);
}

void RequestGenerator::initiateTest()
{
  DEBUG_MSG(TESTER_COMP, MedPrio, "initiating Test");
  sequencer()->makeRequest(CacheMsg(m_address, CacheRequestType_LD, Address(1), AccessModeType_UserMode, 1, PrefetchBit_No)); 
}

void RequestGenerator::initiateSwap()
{
  DEBUG_MSG(TESTER_COMP, MedPrio, "initiating Swap");
  sequencer()->makeRequest(CacheMsg(m_address, CacheRequestType_ATOMIC, Address(2), AccessModeType_UserMode, 1, PrefetchBit_No)); 
}

void RequestGenerator::initiateRelease()
{
  DEBUG_MSG(TESTER_COMP, MedPrio, "initiating Release");
  sequencer()->makeRequest(CacheMsg(m_address, CacheRequestType_ST, Address(3), AccessModeType_UserMode, 1, PrefetchBit_No)); 
}

Sequencer* RequestGenerator::sequencer() const
{
  return g_system_ptr->getSequencer(m_node);
}

void RequestGenerator::print(ostream& out) const
{
}

