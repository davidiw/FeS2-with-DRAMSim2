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
#include "System.h"
#include "SyntheticDriver.h"
#include "EventQueue.h"
#include "RequestGenerator.h"
#include "SubBlock.h"
#include "Param.h"

SyntheticDriver::SyntheticDriver()
{
  if (g_param_ptr->SIMICS()) {
    ERROR_MSG("SIMICS should not be defined.");
  }

  m_finish_time = 0;
  m_done_counter = 0;

  m_last_progress_vector.setSize(g_param_ptr->NUM_NODES());
  for (int i=0; i<m_last_progress_vector.size(); i++) {
    m_last_progress_vector[i] = 0;
  }

  m_lock_vector.setSize(g_param_ptr->SYNTHETIC_LOCKS());
  for (int i=0; i<m_lock_vector.size(); i++) {
    m_lock_vector[i] = -1;  // No processor last held it
  }

  m_request_generator_vector.setSize(g_param_ptr->NUM_NODES());
  for (int i=0; i<m_request_generator_vector.size(); i++) {
    m_request_generator_vector[i] = new RequestGenerator(i, *this);
  }

  // add the tester consumer to the global event queue
  g_eventQueue_ptr->scheduleEvent(this, 1);
}

SyntheticDriver::~SyntheticDriver()
{
  for (int i=0; i<m_last_progress_vector.size(); i++) {
    delete m_request_generator_vector[i];
  }
}

void SyntheticDriver::hitCallback(NodeID proc, SubBlock& data)
{
  DEBUG_EXPR(TESTER_COMP, MedPrio, data);
  m_request_generator_vector[proc]->performCallback(proc, data);

  // Mark that we made progress
  m_last_progress_vector[proc] = g_eventQueue_ptr->getTime();
}

Address SyntheticDriver::pickAddress(NodeID node)
{
  // This methods picks a random lock that we were NOT that last
  // processor to acquire.  Why?  Without this change 2 and 4
  // processor runs, the odds of having the lock in your cache in
  // read/write state is 50% or 25%, respectively.  This effect can
  // make our 'throughput per processor' results look too strange.

  Address addr;
  // FIXME - make this a parameter of the workload
  bool done = false;
  int lock_number = 0;
  int counter = 0;
  while (1) {
    // Pick a random lock
    lock_number = random() % m_lock_vector.size();

    // Were we the last to acquire the lock?
    if (m_lock_vector[lock_number] != node) {
      break;
    }

    // Don't keep trying forever, since if there is only one lock, we're always the last to try to obtain the lock
    counter++;
    if (counter > 10) { 
      break;
    }
  }

  // We're going to acquire it soon, so we can update the last
  // processor to hold the lock at this time
  m_lock_vector[lock_number] = node;

  // One lock per cache line
  addr.setAddress(lock_number * (1 < g_param_ptr->DATA_BLOCK_BITS()));
  return addr;
}

void SyntheticDriver::reportDone()
{
  m_done_counter++;
  if (m_done_counter == g_param_ptr->NUM_NODES()) {
    m_finish_time = g_eventQueue_ptr->getTime();
  }
}

void SyntheticDriver::recordTestLatency(Time time)
{
  m_test_latency.add(time);
}

void SyntheticDriver::recordSwapLatency(Time time)
{
  m_swap_latency.add(time);
}

void SyntheticDriver::recordReleaseLatency(Time time)
{
  m_release_latency.add(time);
}

void SyntheticDriver::wakeup()
{ 
  //  checkForDeadlock();
  if (m_done_counter < g_param_ptr->NUM_NODES()) {
    g_eventQueue_ptr->scheduleEvent(this, g_param_ptr->DEADLOCK_THRESHOLD());
  }
}

void SyntheticDriver::checkForDeadlock()
{
  int size = m_last_progress_vector.size();
  Time current_time = g_eventQueue_ptr->getTime();
  for (int processor=0; processor<size; processor++) {
    if ((current_time - m_last_progress_vector[processor]) > g_param_ptr->DEADLOCK_THRESHOLD()) {
      WARN_EXPR(processor);
      Sequencer* seq_ptr = g_system_ptr->getSequencer(processor);
      assert(seq_ptr != NULL);
      //     if (seq_ptr->isRequestPending()) {
      //       WARN_EXPR(seq_ptr->pendingAddress());
      //      }
      WARN_EXPR(current_time);
      WARN_EXPR(m_last_progress_vector[processor]);
      WARN_EXPR(current_time - m_last_progress_vector[processor]);
      ERROR_MSG("Deadlock detected.");
    }
  }
}

void SyntheticDriver::printStats(ostream& out) const
{
  out << endl;
  out << heading("SyntheticDriver Stats");

  out << "synthetic_finish_time: " << m_finish_time << endl;
  out << "test_latency: " << m_test_latency << endl;
  out << "swap_latency: " << m_swap_latency << endl;
  out << "release_latency: " << m_release_latency << endl;
}

void SyntheticDriver::print(ostream& out) const
{
}
