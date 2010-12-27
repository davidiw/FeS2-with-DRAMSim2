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

#include "FuncCallExprAST.h"
#include "SymbolTable.h"

FuncCallExprAST::FuncCallExprAST(string* proc_name_ptr, 
                                 Vector<ExprAST*>* expr_vec_ptr)
  : ExprAST()
{
  m_proc_name_ptr = proc_name_ptr;
  m_expr_vec_ptr = expr_vec_ptr;
}

FuncCallExprAST::~FuncCallExprAST()
{
  delete m_proc_name_ptr;
  int size = m_expr_vec_ptr->size();
  for(int i=0; i<size; i++) {
    delete (*m_expr_vec_ptr)[i];
  }
  delete m_expr_vec_ptr;
}

Type* FuncCallExprAST::generate(string& code) const
{
  // DEBUG_EXPR is strange since it takes parameters of multiple types
  if (*m_proc_name_ptr == "DEBUG_EXPR") {
    // FIXME - check for number of parameters
    code += "DEBUG_SLICC(MedPrio, \"";
    code += (*m_expr_vec_ptr)[0]->getLocation().toString();
    code += ": \", ";
    (*m_expr_vec_ptr)[0]->generate(code);
    code += ");\n";
    Type* void_type_ptr = g_sym_table.getType("void");
    assert(void_type_ptr != NULL);
    return void_type_ptr;
  } 

  // Look up the function in the symbol table
  Vector<string> code_vec;
  Func* func_ptr = g_sym_table.getFunc(*m_proc_name_ptr);

  // Check the types and get the code for the parameters
  if (func_ptr == NULL) {
    error("Unrecognized function name: '" + *m_proc_name_ptr + "'");
  } else {
    int size = m_expr_vec_ptr->size();
    int expected_size = func_ptr->getParamTypes().size();
    if (size != expected_size) {
      error("Wrong number of parameters: actual: " + int_to_string(size) + ", expected: " + int_to_string(expected_size));
    }
    for(int i=0; i<size; i++) {

      // Check the types of the parameter
      string param_code;
      Type* actual_type_ptr = (*m_expr_vec_ptr)[i]->generate(param_code);
      Type* expected_type_ptr = func_ptr->getParamTypes()[i];
      if (actual_type_ptr != expected_type_ptr) {
        (*m_expr_vec_ptr)[i]->error("Type mismatch: expected: " + expected_type_ptr->toString() + 
                                    " actual: " + actual_type_ptr->toString());
      }
      code_vec.insertAtBottom(param_code);
    }
  }

  /* OK, the semantics of "trigger" here is that, ports in the machine have 
   * different priorities. We always check the first port for doable
   * transitions. If nothing/stalled, we pick one from the next port.
   * 
   * One thing we have to be careful as the SLICC protocol writter is :
   * If a port have two or more transitions can be picked from in one cycle,
   * they must be independent. Otherwise, if transition A and B mean to be
   * executed in sequential, and A get stalled, transition B can be issued
   * erroneously. In practice, in most case, there is only one transition
   * should be executed in one cycle for a given port. So as most of current
   * protocols.
   */
  if (*m_proc_name_ptr == "trigger") {
    code += indent_str() + "{\n";
    code += indent_str() + "  Address addr = ";
    code += code_vec[1];
    code += ";\n";
    code += indent_str() + "  TransitionResult result = doTransition(";
    code += code_vec[0];
    code += ", " + g_sym_table.getStateMachine()->toString() + "_getState(addr), addr);\n";
    code += indent_str() + "  if (result == TransitionResult_Valid) {\n";
    code += indent_str() + "    counter++;\n";
    code += indent_str() + "    continue; // Check the first port again\n";
    code += indent_str() + "  }\n";
    code += indent_str() + "  if (result == TransitionResult_ResourceStall) {\n";
    code += indent_str() + "    g_eventQueue_ptr->scheduleEvent(this, 1);\n";
    code += indent_str() + "    // Cannot do anything with this transition, go check next doable transition (mostly likely of next port)\n";
    code += indent_str() + "  }\n";
    code += indent_str() + "}\n";
  } else if (*m_proc_name_ptr == "error") {
    code += indent_str() + (*m_expr_vec_ptr)[0]->embedError(code_vec[0]) + "\n";
  } else if (*m_proc_name_ptr == "assert") {
    code += indent_str() + "if (ASSERT_FLAG && !(" + code_vec[0] + ")) {\n";
    code += indent_str() + "  " + (*m_expr_vec_ptr)[0]->embedError("\"assert failure\"") + "\n";
    code += indent_str() + "}\n";
  } else {
    // Normal function
    code += "(";

    // FIXME - make this work for function not in a state machine

    // if the func is internal to the node but not the machine then it can only be
    // accessed through the node pointer
    //    if (!func_ptr->existPair("external") && !func_ptr->isInternalMachineFunc()) {
    //      code += "m_node_ptr->";
    //    }

    code += func_ptr->cIdent() + "(";
    int size = code_vec.size();
    for(int i=0; i<size; i++) {
      if (i != 0) {
        code += ", ";
      }
      code += code_vec[i];
    }
    code += "))";
  }
  return func_ptr->getReturnType();
}

void FuncCallExprAST::print(ostream& out) const
{
  out << "[FuncCallExpr: " << *m_proc_name_ptr << " " << *m_expr_vec_ptr << "]";
}
