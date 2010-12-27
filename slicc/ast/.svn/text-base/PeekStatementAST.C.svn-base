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

#include "PeekStatementAST.h"
#include "SymbolTable.h"
#include "StatementListAST.h"
#include "TypeAST.h"
#include "VarExprAST.h"

PeekStatementAST::PeekStatementAST(VarExprAST* queue_name_ptr,
                                   TypeAST* type_ptr,
                                   StatementListAST* statementlist_ptr,
                                   string method)
  : StatementAST()
{
  m_queue_name_ptr = queue_name_ptr;
  m_type_ptr = type_ptr;
  m_statementlist_ptr = statementlist_ptr;
  m_method = method;
}

PeekStatementAST::~PeekStatementAST()
{
  delete m_queue_name_ptr;
  delete m_type_ptr;
  delete m_statementlist_ptr;
}

void PeekStatementAST::generate(string& code, Type* return_type_ptr) const
{
  code += indent_str() + "{\n";  // Start scope
  inc_indent();
  g_sym_table.pushFrame();

  Type* msg_type_ptr = m_type_ptr->lookupType();

  // Add new local var to symbol table
  g_sym_table.newTempSym(new Var("in_msg", getLocation(), msg_type_ptr, "(*in_msg_ptr)", getPairs()));

  // Check the queue type
  m_queue_name_ptr->assertType("InPort");

  // Declare the new "in_msg_ptr" variable
  code += indent_str() + "const " + msg_type_ptr->cIdent() + "* in_msg_ptr;\n";  // Declare message
  //  code += indent_str() + "in_msg_ptr = static_cast<const ";
  code += indent_str() + "in_msg_ptr = dynamic_cast<const ";
  code += msg_type_ptr->cIdent() + "*>(";
  code += "(" + m_queue_name_ptr->getVar()->getCode() + ")";
  code += ".";
  code += m_method;
  code += "());\n";

  code += indent_str() + "assert(in_msg_ptr != NULL);\n";        // Check the cast result
  m_statementlist_ptr->generate(code, return_type_ptr);                // The other statements
  dec_indent();
  g_sym_table.popFrame();
  code += indent_str() + "}\n";  // End scope
}

void PeekStatementAST::findResources(Map<Var*, int>& resource_list) const
{
  m_statementlist_ptr->findResources(resource_list);
}

void PeekStatementAST::print(ostream& out) const
{
  out << "[PeekStatementAST: " << m_method 
      << " queue_name: " << *m_queue_name_ptr
      << " type: " << m_type_ptr->toString()
      << " " << *m_statementlist_ptr
      << "]";
}
