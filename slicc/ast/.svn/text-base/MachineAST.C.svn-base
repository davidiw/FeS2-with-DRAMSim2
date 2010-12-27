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

#include "MachineAST.h"
#include "SymbolTable.h"
#include "Enum.h"

MachineAST::MachineAST(string* ident_ptr, 
                       PairListAST* pairs_ptr,
                       DeclListAST* decl_list_ptr)

  : DeclAST(pairs_ptr)
{
  m_ident_ptr = ident_ptr;
  m_decl_list_ptr = decl_list_ptr;
}

MachineAST::~MachineAST()
{
  delete m_ident_ptr;
  delete m_decl_list_ptr;
}

void MachineAST::generate()
{
  StateMachine* machine_ptr = new StateMachine(*m_ident_ptr, getLocation(), getPairs());
  g_sym_table.newSym(machine_ptr);

  // Make a new frame
  g_sym_table.pushFrame();
  
  // Create a new machine
  g_sym_table.newCurrentMachine(machine_ptr);
  
  // Setup the "id" variable
  Type* type_ptr = g_sym_table.getType("NodeID");
  if (type_ptr == NULL) {
    error("Type 'NodeID' not declared.");
  }
  g_sym_table.newTempSym(new Var("id", getLocation(), type_ptr, "m_id", getPairs()));

  // Generate code for all the internal decls
  m_decl_list_ptr->generate();
  
  // Build the transition table
  machine_ptr->buildTable();
  
  // Pop the frame
  g_sym_table.popFrame();
}

void MachineAST::findMachines()
{
  // Add to MachineType enumeration
  Enum* enum_ptr = g_sym_table.getEnum("MachineType");
  assert(enum_ptr != NULL);
  if (!enum_ptr->enumAdd(*m_ident_ptr, getPairs())) {
    error("Duplicate machine name: " + enum_ptr->toString() + ":" + *m_ident_ptr);
  }

  // Generate code for all the internal decls
  m_decl_list_ptr->findMachines();
}

void MachineAST::print(ostream& out) const 
{ 
  out << "[Machine: " << *m_ident_ptr << "]"; 
}
