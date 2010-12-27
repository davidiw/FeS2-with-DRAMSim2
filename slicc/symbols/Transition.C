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

#include "Transition.h"
#include "State.h"
#include "Event.h"
#include "Action.h"

Transition::Transition(string state, string event, string nextState,
                       const Vector<string>& actionList,
                       const Location& location,
                       const Map<string, string>& pairMap,
                       const StateMachine* machine_ptr)
  : Symbol(state + "|" + event, location, pairMap, machine_ptr)
{
  m_state = state;
  m_event = event;
  m_nextState = nextState;
  m_actionList = actionList;

  // Ptrs are undefined at this point
  m_statePtr = NULL;
  m_eventPtr = NULL;
  m_nextStatePtr = NULL;
  m_actionPtrsValid = false;
}

void Transition::checkIdents(const Vector<State*>& states, 
                             const Vector<Event*>& events, 
                             const Vector<Action*>& actions)
{
  m_statePtr = findIndex(states, m_state);
  m_eventPtr = findIndex(events, m_event);
  m_nextStatePtr = findIndex(states, m_nextState);

  for(int i=0; i < m_actionList.size(); i++) {
    Action* action_ptr = findIndex(actions, m_actionList[i]);
    int size = action_ptr->getResources().keys().size();
    for (int j=0; j < size; j++) {
      Var* var_ptr = action_ptr->getResources().keys()[j];
      int num = action_ptr->getResources().lookup(var_ptr);
      if (m_resources.exist(var_ptr)) {
        num += m_resources.lookup(var_ptr);
      }
      m_resources.add(var_ptr, num);
    }
    m_actionPtrs.insertAtBottom(action_ptr);
  }
  m_actionPtrsValid = true;
}

const string& Transition::getStateShorthand() const
{
  assert(m_statePtr != NULL);
  return m_statePtr->getShorthand();
}

const string& Transition::getEventShorthand() const
{
  assert(m_eventPtr != NULL);
  return m_eventPtr->getShorthand();
}

const string& Transition::getNextStateShorthand() const
{
  assert(m_nextStatePtr != NULL);
  return m_nextStatePtr->getShorthand();
}

string Transition::getActionShorthands() const
{
  assert(m_actionPtrsValid);
  string str;
  int numActions = m_actionPtrs.size();
  for (int i=0; i<numActions; i++) {
    str += m_actionPtrs[i]->getShorthand();
  }
  return str;
}

void Transition::print(ostream& out) const
{
  out << "[Transition: ";
  out << "(" << m_state;
  if (m_statePtr != NULL) {
    out << ":" << *m_statePtr;
  }
  out << ", " << m_event;
  if (m_eventPtr != NULL) {
    out << ":" << *m_eventPtr;
  }
  out << ") -> ";
  out << m_nextState;
  if (m_nextStatePtr != NULL) {
    out << ":" << *m_nextStatePtr;
  }
  out << ", ";
  out << m_actionList;
  out << "]";
}

Event* Transition::findIndex(const Vector<Event*>& vec, string ident)
{
  int size = vec.size();
  for(int i=0; i<size; i++) {
    if (ident == vec[i]->getIdent()) {
      return vec[i];
    }
  }
  error("Event not found: " + ident);
  return NULL;
}

State* Transition::findIndex(const Vector<State*>& vec, string ident)
{
  int size = vec.size();
  for(int i=0; i<size; i++) {
    if (ident == vec[i]->getIdent()) {
      return vec[i];
    }
  }
  error("State not found: " + ident);
  return NULL;
}

Action* Transition::findIndex(const Vector<Action*>& vec, string ident)
{
  int size = vec.size();
  for(int i=0; i<size; i++) {
    if (ident == vec[i]->getIdent()) {
      return vec[i];
    }
  }
  error("Action not found: " + ident);
  return NULL;
}
  
