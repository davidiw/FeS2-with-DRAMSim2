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
#include "Tester.h"
#include "EventQueue.h"
#include "SubBlock.h"
#include "Check.h"
#include "Param.h"
#include "Sequencer.h"

Tester::Tester()
{
  if (g_param_ptr->SIMICS()) {
    ERROR_MSG("SIMICS should not be defined.");
  }

  m_callback_counter = 0;

  // add the tester consumer to the global event queue
  g_eventQueue_ptr->scheduleEvent(this, 1);

  m_last_progress_vector.setSize(g_param_ptr->NUM_NODES());
  for (int i=0; i<m_last_progress_vector.size(); i++) {
    m_last_progress_vector[i] = 0;
  }
}

Tester::~Tester()
{
}

void Tester::hitCallback(NodeID proc, SubBlock& data)
{
  // Mark that we made progress
  m_last_progress_vector[proc] = g_eventQueue_ptr->getTime();
  m_callback_counter++;

  // This tells us our store has 'completed' or for a load gives us
  // back the data to make the check
  DEBUG_EXPR(TESTER_COMP, MedPrio, proc);
  DEBUG_EXPR(TESTER_COMP, MedPrio, data);
  Check* check_ptr = m_checkTable.getCheck(data.getAddress());
  assert(check_ptr != NULL);
  check_ptr->performCallback(proc, data);
}

void Tester::wakeup()
{ 
  if (m_callback_counter < g_param_ptr->TESTER_LENGTH()) {
    // Try to perform an action or check
    Check* check_ptr = m_checkTable.getRandomCheck();
    assert(check_ptr != NULL);
    check_ptr->initiate();
    
    checkForDeadlock();
    
    g_eventQueue_ptr->scheduleEvent(this, 2);
  }
}

void Tester::checkForDeadlock()
{
  int size = m_last_progress_vector.size();
  Time current_time = g_eventQueue_ptr->getTime();
  for (int processor=0; processor<size; processor++) {
    if ((current_time - m_last_progress_vector[processor]) > g_param_ptr->DEADLOCK_THRESHOLD()) {
      WARN_EXPR(current_time);
      WARN_EXPR(m_last_progress_vector[processor]);
      WARN_EXPR(current_time - m_last_progress_vector[processor]);
      WARN_EXPR(processor);
      Sequencer* seq_ptr = g_system_ptr->getSequencer(processor);
      assert(seq_ptr != NULL);
      WARN_EXPR(*seq_ptr);
      ERROR_MSG("Deadlock detected.");
    }
  }
}

void Tester::print(ostream& out) const
{
}

