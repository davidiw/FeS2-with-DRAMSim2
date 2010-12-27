/* This file is part of the Ruby Multiprocessor Memory System
   Simulator, a component of the Multifacet GEMS (General
   Execution-driven Multiprocessor Simulator) software toolset
   originally developed at the University of Wisconsin-Madison.

   Ruby was originally developed primarily by Milo Martin and Daniel
   Sorin with contributions from Ross Dickson, Carl Mauer, and Manoj
   Plakal.

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

#include "NetDest.h"

NetDest::NetDest()  
{ 
  m_sets.setSize(MachineType_NUM);
}

void NetDest::add(MachineType machine, NodeID node)
{
  m_sets[machine].add(node);
}

void NetDest::addSet(MachineType machine, const Set& set)
{
  m_sets[machine].addSet(set);
}

void NetDest::addNetDest(const NetDest& netdest)
{
  for (int i=0; i<m_sets.size(); i++) {
    m_sets[i].addSet(netdest.m_sets[i]);
  }
}

void NetDest::remove(MachineType machine, NodeID node)
{
  m_sets[machine].remove(node);
}

void NetDest::removeSet(MachineType machine, const Set& set)
{
  m_sets[machine].removeSet(set);
}

void NetDest::removeNetDest(const NetDest& netdest)
{
  assert(m_sets.size() == netdest.m_sets.size());
  for (int i=0; i<m_sets.size(); i++) {
    m_sets[i].removeSet(netdest.m_sets[i]);
  }
}

bool NetDest::isElement(MachineType machine, NodeID element) const
{
  return m_sets[machine].isElement(element);
}

void NetDest::broadcast()
{
  for (int i=0; i<m_sets.size(); i++) {
    m_sets[i].broadcast();
  }
}

void NetDest::broadcast(MachineType machine)
{
  m_sets[machine].broadcast();
}

void NetDest::clear()
{
  for (int i=0; i<m_sets.size(); i++) {
    m_sets[i].clear();
  }
}

int NetDest::count() const
{
  int counter = 0;
  for (int i=0; i<m_sets.size(); i++) {
    counter += m_sets[i].count();
  }
  return counter;
}

// returns the logical OR of "this" set and orNetDest 
NetDest NetDest::OR(const NetDest& orNetDest) const
{
  assert(m_sets.size() == orNetDest.m_sets.size());
  NetDest result;
  for (int i=0; i<m_sets.size(); i++) {  
    result.m_sets[i] = m_sets[i].OR(orNetDest.m_sets[i]);
  }
  return result;
}

// returns the logical AND of "this" set and orNetDest 
NetDest NetDest::AND(const NetDest& andNetDest) const
{
  assert(m_sets.size() == andNetDest.m_sets.size());
  NetDest result;
  for (int i=0; i<m_sets.size(); i++) {  
    result.m_sets[i] = m_sets[i].AND(andNetDest.m_sets[i]);
  }
  return result;
}

// Returns true if the intersection of the two sets is non-empty
bool NetDest::intersectionIsNotEmpty(const NetDest& other_netDest) const
{
  assert(m_sets.size() == other_netDest.m_sets.size());
  for (int i=0; i<m_sets.size(); i++) {      
    if (m_sets[i].intersectionIsNotEmpty(other_netDest.m_sets[i])) {
      return true;
    }
  }
  return false;
}

// Returns true if the intersection of the two sets is empty
bool NetDest::intersectionIsEmpty(const NetDest& other_netDest) const
{
  assert(m_sets.size() == other_netDest.m_sets.size());
  for (int i=0; i<m_sets.size(); i++) {      
    if (!m_sets[i].intersectionIsEmpty(other_netDest.m_sets[i])) {
      return false;
    }
  }
  return true;
}

void NetDest::print(ostream& out) const
{
  out << "[NetDest ";
  for (int i=0; i<m_sets.size(); i++) {
    out << m_sets[i] << " ";
  }
  out << "]";
}

