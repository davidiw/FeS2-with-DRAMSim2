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

#ifndef SYMBOL_H
#define SYMBOL_H

#include "global.h"
#include "Map.h"
#include "Location.h"

class Symbol {
public:
  // Constructors
  Symbol(string id, const Location& location, const Map<string, string>& pairs, const StateMachine* machine_ptr);

  // Destructor
  virtual ~Symbol() { }
  
  // Public Methods
  void error(string err_msg) const { m_location.error(err_msg); }
  void warning(string err_msg) const { m_location.warning(err_msg); }
  const Location& getLocation() const { return m_location; }

  const string& cIdent() const;
  const string& toString() const { return m_id; }
  const string& getIdent() const { return m_id; }
  const string& getShorthand() const { return lookupPair("short"); }
  const string& getDescription() const { return lookupPair("desc"); }

  void markUsed() { m_used = true; }
  bool wasUsed() { return m_used; }

  bool existPair(const string& key) const { return m_pairs.exist(key); }
  const string& lookupPair(const string& key) const;
  void addPair(const string& key, const string& value);

  const StateMachine* getMachine() const { return m_machine_ptr; }

  virtual void writeCFiles(string path) const {}
  virtual void writeHTMLFiles(string path) const {}
  virtual void print(ostream& out) const { out << "[Symbol: " << getIdent() << "]"; }

private:
  // Private Methods

  // Private copy constructor and assignment operator
  // Symbol(const Symbol& obj);
  // Symbol& operator=(const Symbol& obj);
  
  // Data Members (m_ prefix)
  string m_id;
  string m_c_id;
  Map<string, string> m_pairs;
  Location m_location;
  bool m_used;
  const StateMachine* m_machine_ptr;
};

// Output operator declaration
ostream& operator<<(ostream& out, const Symbol& obj);

// ******************* Definitions *******************

// Output operator definition
extern inline 
ostream& operator<<(ostream& out, const Symbol& obj)
{
  obj.print(out);
  out << flush;
  return out;
}

#endif //SYMBOL_H
