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

#ifndef PERFECTCACHEMEMORY_H
#define PERFECTCACHEMEMORY_H

#include "Global.h"
#include "Map.h"
#include "AccessPermission.h"
#include "Address.h"
#include "interface.h"

template<class ENTRY>
class PerfectCacheLineState {
public:
  PerfectCacheLineState() { m_permission = AccessPermission_NUM; }
  AccessPermission m_permission;
  ENTRY m_entry;
};

template<class ENTRY>
class PerfectCacheMemory {
public:

  // Constructors
  PerfectCacheMemory(NodeID id);

  // Destructor
  //~PerfectCacheMemory();
  
  // Public Methods
  void printConfig(ostream& out) {}

  // perform a cache access and see if we hit or not.  Return true on
  // a hit.
  bool tryCacheAccess(const CacheMsg& msg, bool& block_stc, ENTRY*& entry);

  // tests to see if an address is present in the cache
  bool isTagPresent(const Address& address) const;

  // Returns true if there is:
  //   a) a tag match on this address or there is 
  //   b) an Invalid line in the same cache "way"
  bool cacheAvail(const Address& address) const;

  // find an Invalid entry and sets the tag appropriate for the address
  void allocate(const Address& address);

  void deallocate(const Address& address);

  // Returns with the physical address of the conflicting cache line
  Address cacheProbe(const Address& newAddress) const;

  // looks an address up in the cache
  ENTRY& lookup(const Address& address);
  const ENTRY& lookup(const Address& address) const;

  // Get/Set permission of cache block
  AccessPermission getPermission(const Address& address) const;
  void changePermission(const Address& address, AccessPermission new_perm);

  // Print cache contents
  void print(ostream& out) const;
private:
  // Private Methods

  // Private copy constructor and assignment operator
  PerfectCacheMemory(const PerfectCacheMemory& obj);
  PerfectCacheMemory& operator=(const PerfectCacheMemory& obj);
  
  // Data Members (m_prefix)
  Map<Address, PerfectCacheLineState<ENTRY> > m_map;
};

// Output operator declaration
//ostream& operator<<(ostream& out, const PerfectCacheMemory<ENTRY>& obj);

// ******************* Definitions *******************

// Output operator definition
template<class ENTRY>
extern inline 
ostream& operator<<(ostream& out, const PerfectCacheMemory<ENTRY>& obj)
{
  obj.print(out);
  out << flush;
  return out;
}


// ****************************************************************

template<class ENTRY>
extern inline 
PerfectCacheMemory<ENTRY>::PerfectCacheMemory(NodeID id)
{
}

// PUBLIC METHODS

template<class ENTRY>
extern inline 
bool PerfectCacheMemory<ENTRY>::tryCacheAccess(const CacheMsg& msg, bool& block_stc, ENTRY*& entry)
{
  ERROR_MSG("not implemented");
}

// tests to see if an address is present in the cache
template<class ENTRY>
extern inline 
bool PerfectCacheMemory<ENTRY>::isTagPresent(const Address& address) const
{
  return m_map.exist(line_address(address));
}

template<class ENTRY>
extern inline 
bool PerfectCacheMemory<ENTRY>::cacheAvail(const Address& address) const
{
  return true;
}

// find an Invalid or already allocated entry and sets the tag
// appropriate for the address
template<class ENTRY>
extern inline 
void PerfectCacheMemory<ENTRY>::allocate(const Address& address) 
{
  PerfectCacheLineState<ENTRY> line_state;
  line_state.m_permission = AccessPermission_Busy;
  line_state.m_entry = ENTRY();
  m_map.add(line_address(address), line_state);
}

// deallocate entry
template<class ENTRY>
extern inline
void PerfectCacheMemory<ENTRY>::deallocate(const Address& address)
{
  m_map.erase(line_address(address));
}

// Returns with the physical address of the conflicting cache line
template<class ENTRY>
extern inline 
Address PerfectCacheMemory<ENTRY>::cacheProbe(const Address& newAddress) const
{
  ERROR_MSG("cacheProbe called in perfect cache");
}

// looks an address up in the cache
template<class ENTRY>
extern inline 
ENTRY& PerfectCacheMemory<ENTRY>::lookup(const Address& address)
{
  return m_map.lookup(line_address(address)).m_entry;
}

// looks an address up in the cache
template<class ENTRY>
extern inline 
const ENTRY& PerfectCacheMemory<ENTRY>::lookup(const Address& address) const
{
  return m_map.lookup(line_address(address)).m_entry;
}

template<class ENTRY>
extern inline
AccessPermission PerfectCacheMemory<ENTRY>::getPermission(const Address& address) const
{
  return m_map.lookup(line_address(address)).m_permission;
}

template<class ENTRY>
extern inline 
void PerfectCacheMemory<ENTRY>::changePermission(const Address& address, AccessPermission new_perm)
{
  Address line_address = address;
  line_address.makeLineAddress();
  PerfectCacheLineState<ENTRY>& line_state = m_map.lookup(line_address);
  AccessPermission old_perm = line_state.m_permission;
  line_state.m_permission = new_perm;
}

template<class ENTRY>
extern inline 
void PerfectCacheMemory<ENTRY>::print(ostream& out) const
{ 
}

#endif //PERFECTCACHEMEMORY_H
