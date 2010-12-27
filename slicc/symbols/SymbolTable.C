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

#include "SymbolTable.h"
#include "Type.h"
#include "Enum.h"
#include "Var.h"
#include "Func.h"
#include "ExternType.h"
#include "StateMachine.h"
  
SymbolTable g_sym_table;

SymbolTable::SymbolTable()
{
  m_sym_map_vec.setSize(1);
  m_depth = 0;

  {
    Map<string, string> pairs;
    newSym(new Enum("MachineType", Location(), pairs));
  }

  {
    Map<string, string> pairs;
    pairs.add("primitive", "yes");
    newSym(new ExternType("void", Location(), pairs));
  }
}

SymbolTable::~SymbolTable()
{
  int size = m_sym_vec.size();
  for(int i=0; i<size; i++) {
    delete m_sym_vec[i];
  }
}

void SymbolTable::newSym(Symbol* sym_ptr)
{
  registerSym(sym_ptr->toString(), sym_ptr);
  m_sym_vec.insertAtBottom(sym_ptr);  // Holder for the allocated Sym objects.
}

void SymbolTable::newTempSym(Symbol* sym_ptr)
{
  registerSym(sym_ptr->toString(), sym_ptr);
}

void SymbolTable::registerSym(string id, Symbol* sym_ptr)
{
  // Check for redeclaration (in the current frame only)
  if (m_sym_map_vec[m_depth].exist(id)) {
    sym_ptr->error("Symbol '" + id + "' redeclared in same scope.");
  }
  // FIXME - warn on masking of a declaration in a previous frame
  m_sym_map_vec[m_depth].add(id, sym_ptr);
}

Symbol* SymbolTable::getSym(string ident) const
{
  for (int i=m_depth; i>=0; i--) {
    if (m_sym_map_vec[i].exist(ident)) {
      return m_sym_map_vec[i].lookup(ident);
    }
  }
  return NULL;
}

void SymbolTable::newCurrentMachine(StateMachine* sym_ptr)
{
  registerSym("current_machine", sym_ptr);
}

Type* SymbolTable::getType(string ident) const
{
  return dynamic_cast<Type*>(getSym(ident)); 
}

Enum* SymbolTable::getEnum(string ident) const
{
  return dynamic_cast<Enum*>(getSym(ident)); 
}

Var* SymbolTable::getVar(string ident) const
{
  return dynamic_cast<Var*>(getSym(ident));
}

Func* SymbolTable::getFunc(string ident) const
{
  return dynamic_cast<Func*>(getSym(ident));
}

StateMachine* SymbolTable::getStateMachine(string ident) const
{
  return dynamic_cast<StateMachine*>(getSym(ident));
}

void SymbolTable::pushFrame()
{
  m_depth++;
  m_sym_map_vec.expand(1);
  m_sym_map_vec[m_depth].clear();
}

void SymbolTable::popFrame()
{
  m_depth--;
  assert(m_depth >= 0);
  m_sym_map_vec.expand(-1);
}

Vector<StateMachine*> SymbolTable::getStateMachines() const
{
  Vector<StateMachine*> machine_vec;
  int size = m_sym_vec.size();
  for(int i=0; i<size; i++) {
    StateMachine* type = dynamic_cast<StateMachine*>(m_sym_vec[i]); // Ok
    if (type != NULL) {
      machine_vec.insertAtBottom(type);
    }
  }
  return machine_vec;
}

void SymbolTable::print(ostream& out) const
{
  out << "[SymbolTable]";  // FIXME
}
