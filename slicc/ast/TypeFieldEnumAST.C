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

#include "TypeFieldEnumAST.h"
#include "State.h"
#include "Event.h"
#include "Enum.h"

TypeFieldEnumAST::TypeFieldEnumAST(string* field_id_ptr,
                                   PairListAST* pairs_ptr)
  : AST(pairs_ptr->getPairs())
{
  m_field_id_ptr = field_id_ptr;
}

TypeFieldEnumAST::~TypeFieldEnumAST()
{
  delete m_field_id_ptr;
}

void TypeFieldEnumAST::generate(Enum* enum_ptr)
{
  // Add enumeration
  if (!enum_ptr->enumAdd(*m_field_id_ptr, getPairs())) {
    error("Duplicate enumeration: " + enum_ptr->toString() + ":" + *m_field_id_ptr);
  }
  
  // Fill machine info
  StateMachine* machine_ptr = g_sym_table.getStateMachine();
  if (enum_ptr->toString() == "State") {
    if (machine_ptr == NULL) {
      error("State declaration not part of a machine.");
    }
    machine_ptr->addState(new State(*m_field_id_ptr, getLocation(), getPairs(), machine_ptr));
  }
  if (enum_ptr->toString() == "Event") {
    if (machine_ptr == NULL) {
      error("Event declaration not part of a machine.");
    }
    machine_ptr->addEvent(new Event(*m_field_id_ptr, getLocation(), getPairs(), machine_ptr));
  }
}

void TypeFieldEnumAST::print(ostream& out) const 
{ 
  out << "[TypeFieldEnum: " << *m_field_id_ptr << "]"; 
}
