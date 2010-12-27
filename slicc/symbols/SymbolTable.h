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

#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include "global.h"
#include "Map.h"
#include "Vector.h"

extern SymbolTable g_sym_table;

class SymbolTable {
public:
  // Constructors
  SymbolTable();

  // Destructor
  ~SymbolTable();
  
  // Public Methods
  void newSym(Symbol* sym_ptr);
  void newTempSym(Symbol* sym_ptr);

  Symbol* getSym(string id) const;
  Type* getType(string ident) const;
  Enum* getEnum(string ident) const;
  Var* getVar(string ident) const;
  Func* getFunc(string ident) const;

  const Vector<Symbol*> getAllSymbols() const { return m_sym_vec; }
  Vector<StateMachine*> getStateMachines() const;

  void newCurrentMachine(StateMachine* machine_ptr);
  StateMachine* getStateMachine(string ident) const;
  StateMachine* getStateMachine() const { return getStateMachine("current_machine"); }

  void pushFrame();
  void popFrame();

  void print(ostream& out) const;
private:
  // Private Methods
  void registerGlobalSym(string id, Symbol* sym_ptr);
  void registerSym(string id, Symbol* sym_ptr);

  // Private copy constructor and assignment operator
  SymbolTable(const SymbolTable& obj);
  SymbolTable& operator=(const SymbolTable& obj);
  
  // Data Members (m_ prefix)
  Vector<Symbol*> m_sym_vec;
  Vector<Map<string, Symbol*> > m_sym_map_vec;
  int m_depth;
};

// Output operator declaration
ostream& operator<<(ostream& out, const SymbolTable& obj);

// ******************* Definitions *******************

// Output operator definition
extern inline 
ostream& operator<<(ostream& out, const SymbolTable& obj)
{
  obj.print(out);
  out << flush;
  return out;
}

#endif //SYMBOLTABLE_H
