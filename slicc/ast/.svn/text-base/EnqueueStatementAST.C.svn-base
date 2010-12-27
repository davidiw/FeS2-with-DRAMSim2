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

#include "EnqueueStatementAST.h"
#include "SymbolTable.h"
#include "VarExprAST.h"
#include "PairListAST.h"
#include "ExprAST.h"

EnqueueStatementAST::EnqueueStatementAST(VarExprAST* queue_name_ptr,
                                         TypeAST* type_name_ptr,
                                         ExprAST* expr_ptr,
                                         PairListAST* pairs_ptr,
                                         StatementListAST* statement_list_ast_ptr)
  : StatementAST(pairs_ptr->getPairs())
{
  m_queue_name_ptr = queue_name_ptr;
  m_type_name_ptr = type_name_ptr;
  m_expr_ptr = expr_ptr;
  m_statement_list_ast_ptr = statement_list_ast_ptr;
}

EnqueueStatementAST::~EnqueueStatementAST()
{
  delete m_queue_name_ptr;
  delete m_type_name_ptr;
  delete m_expr_ptr;
  delete m_statement_list_ast_ptr;
}
  
void EnqueueStatementAST::generate(string& code, Type* return_type_ptr) const
{
  code += indent_str() + "{\n";  // Start scope
  inc_indent();
  g_sym_table.pushFrame();
  
  Type* msg_type_ptr = m_type_name_ptr->lookupType();

  // Add new local var to symbol table
  g_sym_table.newTempSym(new Var("out_msg", getLocation(), msg_type_ptr, "out_msg", getPairs()));

  code += indent_str() + msg_type_ptr->cIdent() + " out_msg;\n";  // Declare message
  m_statement_list_ast_ptr->generate(code, NULL);                // The other statements

  code += indent_str();

  m_queue_name_ptr->assertType("OutPort");
  code += "(" + m_queue_name_ptr->getVar()->getCode() + ")";
  code += ".enqueue(out_msg, ";

  Type* expr_type_ptr = m_expr_ptr->generate(code);

  if (expr_type_ptr != g_sym_table.getType("int")) {
    m_expr_ptr->error("Latency in enqueue statement must be of type 'int'");
  }

  if (getPairs().exist("latency")) {
    error("latency pair is now depricated");
  }

  code += ");\n";

  dec_indent();
  g_sym_table.popFrame();
  code += indent_str() + "}\n";  // End scope
}

void EnqueueStatementAST::findResources(Map<Var*, int>& resource_list) const
{
  Var* var_ptr = m_queue_name_ptr->getVar();
  int res_count = 0;
  if (resource_list.exist(var_ptr)) {
    res_count = resource_list.lookup(var_ptr);
  }
  resource_list.add(var_ptr, res_count+1);
}

void EnqueueStatementAST::print(ostream& out) const
{
  out << "[EnqueueStatementAst: " << *m_queue_name_ptr << " " 
      << m_type_name_ptr->toString() << " " << *m_statement_list_ast_ptr << "]";
}
