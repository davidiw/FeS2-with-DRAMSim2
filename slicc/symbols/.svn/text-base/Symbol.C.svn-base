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

#include "Symbol.h"
#include "StateMachine.h"

Symbol::Symbol(string id, const Location& location, const Map<string, string>& pairs, const StateMachine* machine_ptr)
{
  m_id = id; 
  m_location = location; 
  m_pairs = pairs;
  if (!existPair("short")) {
    addPair("short", m_id);
  }
  m_used = false; 
  m_machine_ptr = machine_ptr;
  
  if (m_machine_ptr != NULL) {
    m_c_id = machine_ptr->toString() + "_" + id;  // Append with machine name
  } else {
    m_c_id = id;
  }
}

const string& Symbol::cIdent() const
{
  if (existPair("external") || existPair("primitive")) { 
    return m_id;
  } else {
    return m_c_id;
  }
}

const string& Symbol::lookupPair(const string& key) const
{
  if (!existPair(key)) {
    error("Value for pair '" + key + "' missing.");
  }
  return m_pairs.lookup(key);
}

void Symbol::addPair(const string& key, const string& value)
{
  if (existPair(key)) {
    warning("Pair key '" + key + "' re-defined. new: '" + value + "' old: '" + lookupPair(key) + "'");
  }
  m_pairs.add(key, value);
}
