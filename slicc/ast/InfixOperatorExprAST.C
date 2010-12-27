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

#include "InfixOperatorExprAST.h"

InfixOperatorExprAST::InfixOperatorExprAST(ExprAST* left_ptr, 
                                           string* op_ptr, 
                                           ExprAST* right_ptr)
  : ExprAST()
{
  m_left_ptr = left_ptr;
  m_op_ptr = op_ptr;
  m_right_ptr = right_ptr;
}

InfixOperatorExprAST::~InfixOperatorExprAST()
{
  delete m_left_ptr;
  delete m_op_ptr;
  delete m_right_ptr;
}

Type* InfixOperatorExprAST::generate(string& code) const
{
  code += "(";
  Type* left_type_ptr = m_left_ptr->generate(code);
  code += " " + *m_op_ptr + " ";
  Type* right_type_ptr = m_right_ptr->generate(code);
  code += ")";
  
  string inputs, output;
  // Figure out what the input and output types should be
  if ((*m_op_ptr == "==" || 
       *m_op_ptr == "!=")) {
    output = "bool";
    if (left_type_ptr != right_type_ptr) {
      error("Type mismatch: left & right operand of operator '" + *m_op_ptr + 
            "' must be the same type." + 
            "left: '" + left_type_ptr->toString() + 
            "', right: '" + right_type_ptr->toString() + "'");
    }
  } else {
    if ((*m_op_ptr == "&&" || 
         *m_op_ptr == "||")) {
      // boolean inputs and output
      inputs = "bool";
      output = "bool";
    } else if ((*m_op_ptr == "==" || 
                *m_op_ptr == "!=" || 
                *m_op_ptr == ">=" || 
                *m_op_ptr == "<=" || 
                *m_op_ptr == ">" || 
                *m_op_ptr == "<")) {
      // Integer inputs, boolean output
      inputs = "int";
      output = "bool";
    } else {
      // integer inputs and output
      inputs = "int";
      output = "int";    
    }
    
    Type* inputs_type = g_sym_table.getType(inputs);
    
    if (inputs_type != left_type_ptr) {
      m_left_ptr->error("Type mismatch: left operand of operator '" + *m_op_ptr + 
                        "' expects input type '" + inputs + "', actual was " + left_type_ptr->toString() + "'");
    }
    
    if (inputs_type != right_type_ptr) {
      m_right_ptr->error("Type mismatch: right operand of operator '" + *m_op_ptr + 
                         "' expects input type '" + inputs + "', actual was '" + right_type_ptr->toString() + "'");
    }
  }

  // All is well
  Type* output_type = g_sym_table.getType(output);
  return output_type;
}


void InfixOperatorExprAST::print(ostream& out) const
{
  out << "[InfixExpr: " << *m_left_ptr 
      << *m_op_ptr << *m_right_ptr << "]";
}
