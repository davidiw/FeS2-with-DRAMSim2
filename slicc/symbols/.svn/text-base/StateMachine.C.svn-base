/* This file is part of SLICC (Specification Language for Implementing
   Cache Coherence), a component of the Multifacet GEMS (General
   Execution-driven Multiprocessor Simulator) software toolset
   originally developed at the University of Wisconsin-Madison.

   SLICC was originally developed by Milo Martin with substantial
   contributions from Daniel Sorin.

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

#include "StateMachine.h"
#include "Action.h"
#include "Event.h"
#include "State.h"
#include "Transition.h"
#include "Var.h"
#include "Func.h"
#include "SymbolTable.h"
#include "Vector.h"
#include "machine_gen.h"
#include "gen.h"
#include "html_gen.h"

StateMachine::StateMachine(string ident, const Location& location, const Map<string, string>& pairs)
  : Symbol(ident, location, pairs, NULL)
{ 
  m_table_built = false; 
}

StateMachine::~StateMachine()
{
  // FIXME
  // assert(0);
}

void StateMachine::addState(State* state_ptr)
{
  assert(m_table_built == false);
  m_state_map.add(state_ptr, m_states.size());
  m_states.insertAtBottom(state_ptr);
}

void StateMachine::addEvent(Event* event_ptr)
{
  assert(m_table_built == false);
  m_event_map.add(event_ptr, m_events.size());
  m_events.insertAtBottom(event_ptr);
}

void StateMachine::addAction(Action* action_ptr)
{
  assert(m_table_built == false);

  // Check for duplicate action
  int size = m_actions.size();
  for(int i=0; i<size; i++) {
    if (m_actions[i]->getIdent() == action_ptr->getIdent()) {
      m_actions[i]->warning("Duplicate action definition: " + m_actions[i]->getIdent());
      action_ptr->error("Duplicate action definition: " + action_ptr->getIdent());
    }
    if (m_actions[i]->getShorthand() == action_ptr->getShorthand()) {
      m_actions[i]->warning("Duplicate action shorthand: " + m_actions[i]->getIdent());
      m_actions[i]->warning("    shorthand = " + m_actions[i]->getShorthand());
      action_ptr->warning("Duplicate action shorthand: " + action_ptr->getIdent());
      action_ptr->error("    shorthand = " + action_ptr->getShorthand());
    }
  }

  m_actions.insertAtBottom(action_ptr);
}

void StateMachine::addTransition(Transition* trans_ptr)
{
  assert(m_table_built == false);
  trans_ptr->checkIdents(m_states, m_events, m_actions);
  m_transitions.insertAtBottom(trans_ptr);
}

void StateMachine::buildTable()
{
  assert(m_table_built == false);
  int numStates = m_states.size();
  int numEvents = m_events.size();
  int numTransitions = m_transitions.size();
  int stateIndex, eventIndex;

  for(stateIndex=0; stateIndex < numStates; stateIndex++) {
    m_table.insertAtBottom(Vector<Transition*>());
    for(eventIndex=0; eventIndex < numEvents; eventIndex++) {
      m_table[stateIndex].insertAtBottom(NULL);
    }
  }

  for(int i=0; i<numTransitions; i++) {
    Transition* trans_ptr = m_transitions[i];

    // Track which actions we touch so we know if we use them all --
    // really this should be done for all symbols as part of the
    // symbol table, then only trigger it for Actions, States, Events,
    // etc.

    Vector<Action*> actions = trans_ptr->getActions();
    for(int actionIndex=0; actionIndex < actions.size(); actionIndex++) {
      actions[actionIndex]->markUsed();
    }

    stateIndex = getStateIndex(trans_ptr->getStatePtr());
    eventIndex = getEventIndex(trans_ptr->getEventPtr());
    if (m_table[stateIndex][eventIndex] != NULL) {
      m_table[stateIndex][eventIndex]->warning("Duplicate transition: " + m_table[stateIndex][eventIndex]->toString());
      trans_ptr->error("Duplicate transition: " + trans_ptr->toString());
    }
    m_table[stateIndex][eventIndex] = trans_ptr;
  }

  // Look at all actions to make sure we used them all
  for(int actionIndex=0; actionIndex < m_actions.size(); actionIndex++) {
    Action* action_ptr = m_actions[actionIndex];
    if (!action_ptr->wasUsed()) {
      string error_msg = "Unused action: " +  action_ptr->getIdent();
      if (action_ptr->existPair("desc")) {
        error_msg += ", "  + action_ptr->getDescription();
      }
      action_ptr->warning(error_msg);
    }
  }

  m_table_built = true;
}

const Transition* StateMachine::getTransPtr(int stateIndex, int eventIndex) const
{
  return m_table[stateIndex][eventIndex]; 
}

void StateMachine::writeCFiles(string path) const
{
  write_C_files_machine(path, *this);
}

void StateMachine::writeHTMLFiles(string path) const
{
  write_HTML_files_machine(path, *this);
}

