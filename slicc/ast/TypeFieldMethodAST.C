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

#include "TypeFieldMethodAST.h"
#include "ExternType.h"
#include "PairListAST.h"

TypeFieldMethodAST::TypeFieldMethodAST(TypeAST* return_type_ast_ptr, 
                                       string* ident_ptr, 
                                       Vector<TypeAST*>* type_ast_vec_ptr,
                                       PairListAST* pairs_ptr) 
  : AST(pairs_ptr->getPairs())
{
  m_return_type_ast_ptr = return_type_ast_ptr;
  m_ident_ptr = ident_ptr;
  m_type_ast_vec_ptr = type_ast_vec_ptr;
}

TypeFieldMethodAST::~TypeFieldMethodAST()
{
  delete m_return_type_ast_ptr;
  delete m_ident_ptr;

  int size = m_type_ast_vec_ptr->size();
  for(int i=0; i<size; i++) {
    delete (*m_type_ast_vec_ptr)[i];
  }
  delete m_type_ast_vec_ptr;
}

void TypeFieldMethodAST::generate(ExternType* extern_type_ptr)
{
  // Lookup return type
  Type* return_type_ptr = m_return_type_ast_ptr->lookupType();

  // Lookup parameter types
  Vector<Type*> type_vec;
  int size = m_type_ast_vec_ptr->size();
  for(int i=0; i<size; i++) {
    Type* param_type_ptr = (*m_type_ast_vec_ptr)[i]->lookupType();
    type_vec.insertAtBottom(param_type_ptr);
  }

  // Add method
  if (!extern_type_ptr->methodAdd(*m_ident_ptr, return_type_ptr, type_vec, getPairs())) {  // Return false on error
    error("Duplicate method: " + extern_type_ptr->toString() + ":" + *m_ident_ptr + "()");
  }  
}
