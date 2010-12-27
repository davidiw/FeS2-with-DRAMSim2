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

#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include "global.h"
#include "Vector.h"
#include "Map.h"
#include "Symbol.h"

class StateMachine : public Symbol {
public:
  // Constructors
  StateMachine(string ident, const Location& location, const Map<string, string>& pairs);

  // Destructor
  ~StateMachine();
  
  // Public Methods

  // Add items to the state machine
  //  void setMachine(string ident, const Map<string, string>& pairs);
  void addState(State* state_ptr);
  void addEvent(Event* event_ptr);
  void addAction(Action* action_ptr);
  void addTransition(Transition* trans_ptr);
  void addInPort(Var* var) { m_in_ports.insertAtBottom(var); }

  // Accessors to vectors
  const State& getState(int index) const { return *m_states[index]; }
  const Event& getEvent(int index) const { return *m_events[index]; }
  const Action& getAction(int index) const { return *m_actions[index]; }
  const Transition& getTransition(int index) const { return *m_transitions[index]; }
  const Transition* getTransPtr(int stateIndex, int eventIndex) const;
  const Var& getInPort(int index) const { return *m_in_ports[index]; }

  // Accessors for size of vectors
  int numStates() const { return m_states.size(); }
  int numEvents() const { return m_events.size(); }
  int numActions() const { return m_actions.size(); }
  int numTransitions() const { return m_transitions.size(); }
  int numInPorts() const { return m_in_ports.size(); }

  void buildTable();  // Needs to be called before accessing the table

  void writeCFiles(string path) const;
  void writeHTMLFiles(string path) const;
  void print(ostream& out) const { out << "[StateMachine: " << toString() << "]" << endl; }
private:
  // Private Methods
  void checkForDuplicate(const Symbol& sym) const;

  int getStateIndex(State* state_ptr) const { return m_state_map.lookup(state_ptr); }
  int getEventIndex(Event* event_ptr) const { return m_event_map.lookup(event_ptr); }

  // Private copy constructor and assignment operator 
  StateMachine(const StateMachine& obj);
  StateMachine& operator=(const StateMachine& obj);
  
  // Data Members (m_ prefix)
  Vector<State*> m_states;
  Vector<Event*> m_events;
  Vector<Action*> m_actions;
  Vector<Transition*> m_transitions;

  Map<State*, int> m_state_map;
  Map<Event*, int> m_event_map;

  Vector<Var*> m_in_ports;

  // Table variables
  bool m_table_built;
  Vector<Vector<Transition*> > m_table;
};

// Output operator declaration
ostream& operator<<(ostream& out, const StateMachine& obj);

// ******************* Definitions *******************

// Output operator definition
extern inline 
ostream& operator<<(ostream& out, const StateMachine& obj)
{
  obj.print(out);
  out << flush;
  return out;
}

#endif //STATEMACHINE_H
