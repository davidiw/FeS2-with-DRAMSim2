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

#include "ReturnStatementAST.h"

ReturnStatementAST::ReturnStatementAST(ExprAST* expr_ptr)
  : StatementAST()
{
  m_expr_ptr = expr_ptr;
}

ReturnStatementAST::~ReturnStatementAST()
{
  delete m_expr_ptr;
}

void ReturnStatementAST::generate(string& code, Type* return_type_ptr) const
{
  code += indent_str();
  code += "return ";
  Type* actual_type_ptr = m_expr_ptr->generate(code);
  code += ";\n";

  // Is return valid here?
  if (return_type_ptr == NULL) {
    error("Invalid 'return' statement");
  }

  // The return type must match the return_type_ptr
  if (return_type_ptr != actual_type_ptr) {
    m_expr_ptr->error("Return type mismatch, expected return type is '" + return_type_ptr->toString() + 
                      "', actual is '" + actual_type_ptr->toString() + "'");
  }
}

void ReturnStatementAST::findResources(Map<Var*, int>& resource_list) const
{
  m_expr_ptr->findResources(resource_list);
}

void ReturnStatementAST::print(ostream& out) const
{
  out << "[ReturnStatementAST: " << *m_expr_ptr << "]";
}
