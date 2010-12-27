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

#include "MethodCallExprAST.h"
#include "ExternType.h"

MethodCallExprAST::MethodCallExprAST(ExprAST* obj_expr_ptr,
                                     string* proc_name_ptr, 
                                     Vector<ExprAST*>* expr_vec_ptr)
  : ExprAST()
{
  m_obj_expr_ptr = obj_expr_ptr;
  m_proc_name_ptr = proc_name_ptr;
  m_expr_vec_ptr = expr_vec_ptr;
}

MethodCallExprAST::~MethodCallExprAST()
{
  delete m_obj_expr_ptr;
  delete m_proc_name_ptr;
  int size = m_expr_vec_ptr->size();
  for(int i=0; i<size; i++) {
    delete (*m_expr_vec_ptr)[i];
  }
  delete m_expr_vec_ptr;
}

Type* MethodCallExprAST::generate(string& code) const
{
  string temp_code;
  temp_code += "((";
  Type* obj_base_type_ptr = m_obj_expr_ptr->generate(temp_code);
  temp_code += ").";

  ExternType* obj_type_ptr = dynamic_cast<ExternType*>(obj_base_type_ptr); // Ok
  if (obj_type_ptr == NULL) {
    error("Invalid method call: Variable " + *m_proc_name_ptr + " of type '" + obj_base_type_ptr->toString() + "' is not an object");
  }
    
  Vector <Type*> paramTypes;

  // generate code
  int actual_size = m_expr_vec_ptr->size();
  temp_code += (*m_proc_name_ptr) + "(";
  for(int i=0; i<actual_size; i++) {
    if (i != 0) {
      temp_code += ", ";
    }
    // Check the types of the parameter
    Type* actual_type_ptr = (*m_expr_vec_ptr)[i]->generate(temp_code);
    paramTypes.insertAtBottom(actual_type_ptr);
  }
  temp_code += "))";

  string methodId = obj_type_ptr->methodId(*m_proc_name_ptr, paramTypes);

  // Verify that this is a method of the object
  if (!obj_type_ptr->methodExist(methodId)) {
    error("Invalid method call: Type '" + obj_type_ptr->toString() + "' does not have a method '" + methodId + "'");
  }

  int expected_size = obj_type_ptr->methodParamType(methodId).size();
  if (actual_size != expected_size) {
    // Right number of parameters
    error("Wrong number of parameters for function name: '" + (*m_proc_name_ptr) + "'" +
          ", expected: " + int_to_string(expected_size) + ", actual: " + int_to_string(actual_size));
  }

  for(int i=0; i<actual_size; i++) {
    // Check the types of the parameter
    Type* actual_type_ptr = paramTypes[i];
    Type* expected_type_ptr = obj_type_ptr->methodParamType(methodId)[i];
    if (actual_type_ptr != expected_type_ptr) {
      (*m_expr_vec_ptr)[i]->error("Type mismatch: expected: " + expected_type_ptr->toString() + 
                                  " actual: " + actual_type_ptr->toString());
    }
  }
  
  Type* return_type = obj_type_ptr->methodReturnType(methodId);
  if (obj_type_ptr->methodPairs(methodId).exist("cast_hack")) {
    code += "(dynamic_cast<" + return_type->cIdent() + "&>(" + temp_code + "))";
  } else {
    code += temp_code;
  }

  // Return the return type of the method
  return return_type;
}

void MethodCallExprAST::findResources(Map<Var*, int>& resource_list) const
{
  
}

void MethodCallExprAST::print(ostream& out) const
{
  out << "[MethodCallExpr: " << *m_proc_name_ptr << *m_obj_expr_ptr << " " << *m_expr_vec_ptr << "]";
}
