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

#ifndef CACHEMEMORY_H
#define CACHEMEMORY_H

#include "Global.h"
#include "AccessPermission.h"
#include "CacheRequestType.h"
#include "Vector.h"
#include "NodeID.h"

class DataBlock;
class CacheEntryBase;
class Address;
class CacheRecorder;

class CacheMemory {
public:

  // Constructors
  CacheMemory(NodeID id, const CacheEntryBase& entry, int numSetBits, int cacheAssoc, const string& description);

  // Destructor
  ~CacheMemory();
  
  // Public Methods
  void printConfig(ostream& out);

  // perform a cache access and see if we hit or not.  Return true on a hit.
  bool tryCacheAccess(const Address& address, CacheRequestType type, DataBlock*& data_ptr);

  // tests to see if an address is present in the cache
  bool isTagPresent(const Address& address) const;

  // Returns true if there is:
  //   a) a tag match on this address or there is 
  //   b) an unused line in the same cache "way"
  bool cacheAvail(const Address& address) const;

  // find an unused entry and sets the tag appropriate for the address
  void allocate(const Address& address);

  // Explicitly free up this address
  void deallocate(const Address& address);

  // Returns with the physical address of the conflicting cache line
  const Address& cacheProbe(const Address& address) const;

  // looks an address up in the cache
  CacheEntryBase& lookup(const Address& address);
  const CacheEntryBase& lookup(const Address& address) const;

  // Get/Set permission of cache block
  AccessPermission getPermission(const Address& address) const;
  void changePermission(const Address& address, AccessPermission new_perm);

  // Hook for checkpointing the contents of the cache
  void recordCacheContents(CacheRecorder& tr, bool is_instruction_cache) const;

  // Set this address to most recently used
  void setMRU(const Address& address);

  // Print cache contents
  void print(ostream& out) const;
  void printData(ostream& out) const;

private:
  // Private Methods

  // convert a Address to its location in the cache
  Index addressToCacheSet(const Address& address) const;

  // Given a cache tag: returns the index of the tag in a set.
  // returns -1 if the tag is not found.
  int findTagInSet(Index line, const Address& tag) const;

  // Private copy constructor and assignment operator
  CacheMemory(const CacheMemory& obj);
  CacheMemory& operator=(const CacheMemory& obj);
  
  // Data Members (m_prefix)
  NodeID m_id;
  string m_description;

  // The first index is the # of cache lines.
  // The second index is the the amount associativity.
  Vector<Vector<CacheEntryBase*> > m_cache;

  int m_cache_num_sets;
  int m_cache_num_set_bits;
  int m_cache_assoc;
};

// Output operator declaration
ostream& operator<<(ostream& out, const CacheMemory& obj);

#endif //CACHEMEMORY_H
