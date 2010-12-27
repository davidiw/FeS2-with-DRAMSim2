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

#include "TransitionDeclAST.h"
#include "Transition.h"

TransitionDeclAST::TransitionDeclAST(Vector<string>* state_list_ptr, 
                                     Vector<string>* event_list_ptr, 
                                     string* next_state_ptr,
                                     PairListAST* pairs_ptr,
                                     Vector<string>* action_list_ptr)
  : DeclAST(pairs_ptr)
{
  m_state_list_ptr = state_list_ptr;
  m_event_list_ptr = event_list_ptr;
  m_next_state_ptr = next_state_ptr;
  m_action_list_ptr = action_list_ptr;
}

TransitionDeclAST::~TransitionDeclAST()
{
  delete m_state_list_ptr;
  delete m_event_list_ptr;
  delete m_next_state_ptr;
  delete m_action_list_ptr;
}

void TransitionDeclAST::generate()
{
  Vector<string>& states = *m_state_list_ptr;
  Vector<string>& events = *m_event_list_ptr;

  StateMachine* machine_ptr = g_sym_table.getStateMachine();
  if (machine_ptr == NULL) {
    error("Transition declaration not part of a machine.");
  } else if (m_next_state_ptr == NULL) {
    for (int i=0; i<states.size(); i++) {
      for (int j=0; j<events.size(); j++) {
        machine_ptr->addTransition(new Transition(states[i], events[j], states[i], *m_action_list_ptr, getLocation(), getPairs(), machine_ptr));
      }
    }
  } else {
    for (int i=0; i<states.size(); i++) {
      for (int j=0; j<events.size(); j++) {
        machine_ptr->addTransition(new Transition(states[i], events[j], *m_next_state_ptr, *m_action_list_ptr, getLocation(), getPairs(), machine_ptr));
      }
    }
  }
}

void TransitionDeclAST::print(ostream& out) const 
{ 
  out << "[TransitionDecl: ]"; 
}
