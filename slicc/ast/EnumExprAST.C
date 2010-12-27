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

#include "EnumExprAST.h"
#include "Enum.h"

EnumExprAST::EnumExprAST(TypeAST* type_ast_ptr,
                         string* value_ptr)
  : ExprAST()
{
  assert(value_ptr != NULL);
  assert(type_ast_ptr != NULL);
  m_type_ast_ptr = type_ast_ptr;
  m_value_ptr = value_ptr;
}

EnumExprAST::~EnumExprAST()
{
  delete m_type_ast_ptr;
  delete m_value_ptr;
}
  
Type* EnumExprAST::generate(string& code) const
{
  Type* type_ptr = m_type_ast_ptr->lookupType();
  Enum* enum_ptr = dynamic_cast<Enum*>(type_ptr);  // Ok
  if (enum_ptr == NULL) {
    error("Type '" + m_type_ast_ptr->toString() + "' is not an enumeration");
  }
  
  code += enum_ptr->cIdent() + "_" + (*m_value_ptr);
  
  // Make sure the enumeration value exists
  if (!enum_ptr->enumExist(*m_value_ptr)) {
    error("Type '" + m_type_ast_ptr->toString() + "' does not have enumeration '" + *m_value_ptr + "'");
  }
  
  // Return the proper type
  return enum_ptr;
}

void EnumExprAST::print(ostream& out) const
{
  string str;
  str += m_type_ast_ptr->toString()+":"+(*m_value_ptr);
  out << "[EnumExpr: " << str << "]";
}
