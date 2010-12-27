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

#ifndef TRANSITION_H
#define TRANSITION_H

#include "global.h"
#include "Vector.h"
#include "Symbol.h"

class Transition : public Symbol {
public:
  // Constructors
  Transition(string state, string event, string nextState,
             const Vector<string>& actionList,
             const Location& location,
             const Map<string, string>& pairMap,
             const StateMachine* machine_ptr);
  // Destructor
  ~Transition() { }
  
  // Public Methods
  State* getStatePtr() const { assert(m_statePtr != NULL); return m_statePtr; }
  Event* getEventPtr() const { assert(m_eventPtr != NULL); return m_eventPtr; }
  State* getNextStatePtr() const { assert(m_nextStatePtr != NULL); return m_nextStatePtr; }

  //  int getStateIndex() const { assert(m_statePtr != NULL); return m_statePtr->getIndex(); }
  //  int getEventIndex() const { assert(m_eventPtr != NULL); return m_eventPtr->getIndex(); }
  //  int getNextStateIndex() const { assert(m_nextStatePtr != NULL); return m_nextStatePtr->getIndex(); }
  void checkIdents(const Vector<State*>& states, 
                   const Vector<Event*>& events, 
                   const Vector<Action*>& actions);

  const string& getStateShorthand() const;
  const string& getEventShorthand() const;
  const string& getNextStateShorthand() const;
  string getActionShorthands() const;
  const Vector<Action*>& getActions() const { assert(m_actionPtrsValid); return m_actionPtrs; }
  const Map<Var*, int>& getResources() const { assert(m_actionPtrsValid); return m_resources; }

  void print(ostream& out) const;

  // Default copy constructor and assignment operator
  // Transition(const Transition& obj);
  // Transition& operator=(const Transition& obj);
private:
  // Private Methods
  Event* findIndex(const Vector<Event*>& vec, string ident);
  State* findIndex(const Vector<State*>& vec, string ident);
  Action* findIndex(const Vector<Action*>& vec, string ident);
  
  // Data Members (m_ prefix)
  string m_state;
  string m_event;
  string m_nextState;

  State* m_statePtr;
  Event* m_eventPtr;
  State* m_nextStatePtr;

  Vector<string> m_actionList;
  Vector<Action*> m_actionPtrs;
  Map<Var*, int> m_resources;
  bool m_actionPtrsValid;
};

// Output operator declaration
ostream& operator<<(ostream& out, const Transition& obj);

// ******************* Definitions *******************

// Output operator definition
extern inline 
ostream& operator<<(ostream& out, const Transition& obj)
{
  obj.print(out);
  out << flush;
  return out;
}

#endif //TRANSITION_H
