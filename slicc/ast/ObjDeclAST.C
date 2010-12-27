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


#include "ObjDeclAST.h"
#include "SymbolTable.h"

ObjDeclAST::ObjDeclAST(TypeAST* type_ptr, 
                       string* ident_ptr, 
                       PairListAST* pairs_ptr)
  : DeclAST(pairs_ptr)
{
  m_type_ptr = type_ptr;
  m_ident_ptr = ident_ptr;
}

ObjDeclAST::~ObjDeclAST()
{
  delete m_type_ptr;
  delete m_ident_ptr;
}

void ObjDeclAST::generate()
{
  //  if (!getPairs().exist("hack")) {
  //    warning("object declarations require a 'hack=' attribute");
  //  }

  string c_code;
  if (getPairs().exist("hack")) {
    warning("'hack=' is now deprecated");
  } 

  if (getPairs().exist("network")) {

    if (g_sym_table.getStateMachine() == NULL) {
      error("Network queues must be declared within a state machine.");
    }

    if (!getPairs().exist("virtual_network")) {
      error("Network queues require a 'virtual_network' attribute.");
    }
  }

  Type* type_ptr = m_type_ptr->lookupType();  
  if (type_ptr->isBuffer()) {
    if (!getPairs().exist("ordered")) {
      error("Buffer object declarations require an 'ordered' attribute.");
    }
  }

  if (getPairs().exist("ordered")) {
    string value = getPairs().lookup("ordered");
    if (value != "true" && value != "false") {
      error("The 'ordered' attribute must be 'true' or 'false'.");
    }
  }

  if (getPairs().exist("random")) {
    string value = getPairs().lookup("random");
    if (value != "true" && value != "false") {
      error("The 'random' attribute must be 'true' or 'false'.");
    }
  }

  string machine;
  if (g_sym_table.getStateMachine() != NULL) {
    machine = g_sym_table.getStateMachine()->getIdent() + "_";
  }

  c_code = "(*m_" + machine + *m_ident_ptr + "_ptr)";

  g_sym_table.newSym(new Var(*m_ident_ptr, getLocation(), type_ptr, c_code, 
                             getPairs(), g_sym_table.getStateMachine()));
}

void ObjDeclAST::print(ostream& out) const 
{ 
  out << "[ObjDecl: " << *m_ident_ptr << "]"; 
}
