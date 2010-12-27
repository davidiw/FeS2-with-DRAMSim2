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

#include "ExternTypeDeclAST.h"
#include "SymbolTable.h"
#include "Struct.h"
#include "ExternType.h"

ExternTypeDeclAST::ExternTypeDeclAST(TypeAST* type_ast_ptr, 
                                     PairListAST* pairs_ptr,
                                     Vector<TypeFieldMethodAST*>* field_vec_ptr)
  : DeclAST(pairs_ptr)
{
  m_type_ast_ptr = type_ast_ptr;
  m_field_vec_ptr = field_vec_ptr;
}

ExternTypeDeclAST::~ExternTypeDeclAST()
{
  delete m_type_ast_ptr;
  if (m_field_vec_ptr != NULL) {
    int size = m_field_vec_ptr->size();
    for(int i=0; i<size; i++) {
      delete (*m_field_vec_ptr)[i];
    }
    delete m_field_vec_ptr;
  }
}

void ExternTypeDeclAST::generate()
{
  string machine_name;
  string id = m_type_ast_ptr->toString();

  // Make the new type
  ExternType* new_type_ptr = new ExternType(id, getLocation(), getPairs(), 
                                            g_sym_table.getStateMachine());
  g_sym_table.newSym(new_type_ptr);

  // Add all of the fields of the type to it
  if (m_field_vec_ptr != NULL) {
    int size = m_field_vec_ptr->size();
    for(int i=0; i<size; i++) {
      (*m_field_vec_ptr)[i]->generate(new_type_ptr);
    }
  }
}

void ExternTypeDeclAST::print(ostream& out) const 
{ 
  out << "[TypeDecl: " << m_type_ast_ptr->toString() << "]"; 
}
