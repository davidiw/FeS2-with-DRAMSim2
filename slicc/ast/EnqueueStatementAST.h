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

#ifndef ENQUEUESTATEMENTAST_H
#define ENQUEUESTATEMENTAST_H

#include "global.h"
#include "StatementAST.h"
#include "StatementListAST.h"
#include "TypeAST.h"

class VarExprAST;
class PairListAST;
class ExprAST;

class EnqueueStatementAST : public StatementAST {
public:
  // Constructors
  EnqueueStatementAST(VarExprAST* queue_name_ptr,
                      TypeAST* type_name_ptr,
                      ExprAST* expr_ptr,
                      PairListAST* pairs_ptr,
                      StatementListAST* statement_list_ast_ptr);

  // Destructor
  ~EnqueueStatementAST();
  
  // Public Methods
  void generate(string& code, Type* return_type_ptr) const;
  void findResources(Map<Var*, int>& resource_list) const;
  void print(ostream& out) const;
private:
  // Private Methods

  // Private copy constructor and assignment operator
  EnqueueStatementAST(const EnqueueStatementAST& obj);
  EnqueueStatementAST& operator=(const EnqueueStatementAST& obj);
  
  // Data Members (m_ prefix)
  VarExprAST* m_queue_name_ptr;
  TypeAST* m_type_name_ptr;
  ExprAST* m_expr_ptr;
  StatementListAST* m_statement_list_ast_ptr;
};

// Output operator declaration
ostream& operator<<(ostream& out, const EnqueueStatementAST& obj);

// ******************* Definitions *******************

// Output operator definition
extern inline 
ostream& operator<<(ostream& out, const EnqueueStatementAST& obj)
{
  obj.print(out);
  out << flush;
  return out;
}

#endif //ENQUEUESTATEMENTAST_H
