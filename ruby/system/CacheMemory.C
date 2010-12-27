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

#include "CacheMemory.h"
#include "CacheEntryBase.h"
#include "Address.h"
#include "EventQueue.h"
#include "CacheRecorder.h"
#include "System.h"
#include "Driver.h"

// Output operator definition
ostream& operator<<(ostream& out, const CacheMemory& obj)
{
  obj.print(out);
  out << flush;
  return out;
}

CacheMemory::CacheMemory(NodeID id, const CacheEntryBase& entry, int numSetBits, int cacheAssoc, const string& description)
{
  m_id = id;
  m_description = description;
  m_cache_num_set_bits = numSetBits;
  m_cache_num_sets = 1 << numSetBits;
  m_cache_assoc = cacheAssoc;

  m_cache.setSize(m_cache_num_sets);
  for (int i = 0; i < m_cache_num_sets; i++) {
    m_cache[i].setSize(m_cache_assoc);
    for (int j=0; j < m_cache_assoc; j++) {
      m_cache[i][j] = entry.construct();
    }
  }
}

CacheMemory::~CacheMemory()
{
  for (int i = 0; i < m_cache_num_sets; i++) {
    for (int j=0; j < m_cache_assoc; j++) {
      delete m_cache[i][j];
    }
  }
}

void CacheMemory::printConfig(ostream& out)
{
  int data_block_bytes = 1 << g_param_ptr->DATA_BLOCK_BITS();
  out << "Cache config: " << m_description << endl;
  out << "  cache_associativity: " << m_cache_assoc << endl;
  out << "  num_cache_sets_bits: " << m_cache_num_set_bits << endl;
  const int cache_num_sets = 1 << m_cache_num_set_bits;
  out << "  num_cache_sets: " << cache_num_sets << endl;
  out << "  cache_set_size_bytes: " << cache_num_sets * data_block_bytes << endl;
  out << "  cache_set_size_Kbytes: " 
      << double(cache_num_sets * data_block_bytes) / (1<<10) << endl;
  out << "  cache_set_size_Mbytes: " 
      << double(cache_num_sets * data_block_bytes) / (1<<20) << endl;
  out << "  cache_size_bytes: " 
      << cache_num_sets * data_block_bytes * m_cache_assoc << endl;
  out << "  cache_size_Kbytes: " 
      << double(cache_num_sets * data_block_bytes * m_cache_assoc) / (1<<10) << endl;
  out << "  cache_size_Mbytes: " 
      << double(cache_num_sets * data_block_bytes * m_cache_assoc) / (1<<20) << endl;
}

// PRIVATE METHODS

// convert a Address to its location in the cache
Index CacheMemory::addressToCacheSet(const Address& address) const
{
  assert(address == line_address(address));
  Index temp = address.bitSelect(g_param_ptr->DATA_BLOCK_BITS(), 
                                 g_param_ptr->DATA_BLOCK_BITS()+m_cache_num_set_bits-1);
  assert(temp < m_cache_num_sets);
  assert(temp >= 0);
  return temp;
}

// Given a cache index: returns the index of the tag in a set.
// returns -1 if the tag is not found.
int CacheMemory::findTagInSet(Index cacheSet, const Address& tag) const
{
  assert(tag == line_address(tag));
  // search the set for the tags
  for (int i=0; i < m_cache_assoc; i++) {
    if ((m_cache[cacheSet][i]->getAddress() == tag) && 
        (m_cache[cacheSet][i]->getPermission() != AccessPermission_NotPresent)) {
      return i;
    }
  }
  return -1; // Not found
}

// PUBLIC METHODS
bool CacheMemory::tryCacheAccess(const Address& address, 
                                           CacheRequestType type, 
                                           DataBlock*& data_ptr)
{
  assert(address == line_address(address));
  DEBUG_EXPR(CACHE_COMP, HighPrio, address);
  Index cacheSet = addressToCacheSet(address);
  int loc = findTagInSet(cacheSet, address);
  if(loc != -1){ // Do we even have a tag match?
    CacheEntryBase& entry = *(m_cache[cacheSet][loc]);
    entry.getLastRef() = g_eventQueue_ptr->getTime();
    data_ptr = &(entry.getDataBlk());
    if(entry.getPermission() == AccessPermission_Read_Write) {
      return true;
    } 
    if ((entry.getPermission() == AccessPermission_Read_Only) && 
        (type == CacheRequestType_LD || type == CacheRequestType_IFETCH)) {
      return true;
    }
    // The line must not be accessible
  }
  data_ptr = NULL;
  return false;
}

// tests to see if an address is present in the cache
bool CacheMemory::isTagPresent(const Address& address) const
{
  assert(address == line_address(address));
  Index cacheSet = addressToCacheSet(address);
  int location = findTagInSet(cacheSet, address);
  
  if (location == -1) {
    // We didn't find the tag
    DEBUG_EXPR(CACHE_COMP, LowPrio, address);
    DEBUG_MSG(CACHE_COMP, LowPrio, "No tag match");
    return false;
  } 
  DEBUG_EXPR(CACHE_COMP, LowPrio, address);
  DEBUG_MSG(CACHE_COMP, LowPrio, "found");
  return true;
}

// Returns true if there is:
//   a) a tag match on this address or there is 
//   b) an unused line in the same cache "way"
bool CacheMemory::cacheAvail(const Address& address) const
{
  assert(address == line_address(address));

  Index cacheSet = addressToCacheSet(address);

  for (int i=0; i < m_cache_assoc; i++) {
    if (m_cache[cacheSet][i]->getAddress() == address) {
      // Already in the cache
      return true;
    }      

    if (m_cache[cacheSet][i]->getPermission() == AccessPermission_NotPresent) {
      // We found an empty entry
      return true;
    }
  }
  return false;
}

void CacheMemory::allocate(const Address& address) 
{
  assert(address == line_address(address));
  assert(!isTagPresent(address));
  assert(cacheAvail(address));
  DEBUG_EXPR(CACHE_COMP, HighPrio, address);

  // Find the first open slot
  Index cacheSet = addressToCacheSet(address);
  for (int i=0; i < m_cache_assoc; i++) {
    if (m_cache[cacheSet][i]->getPermission() == AccessPermission_NotPresent) {
      m_cache[cacheSet][i]->reset();  // Init entry
      m_cache[cacheSet][i]->getAddress() = address;
      m_cache[cacheSet][i]->getPermission() = AccessPermission_Invalid;
      m_cache[cacheSet][i]->getLastRef() = g_eventQueue_ptr->getTime();
      return;
    }
  }
  ERROR_MSG("Allocate didn't find an available entry");
}

void CacheMemory::deallocate(const Address& address)
{
  assert(address == line_address(address));
  assert(isTagPresent(address));
  DEBUG_EXPR(CACHE_COMP, HighPrio, address);
  lookup(address).getPermission() = AccessPermission_NotPresent;
}

// Returns with the physical address of the conflicting cache line
const Address& CacheMemory::cacheProbe(const Address& address) const
{
  assert(address == line_address(address));
  assert(!cacheAvail(address));

  // implements the replacement policy in a set associative caches
  Index cacheSet = addressToCacheSet(address);
  Time time, smallest_time;
  Index smallest_index;

  smallest_index = 0;
  smallest_time = m_cache[cacheSet][0]->getLastRef();

  for (int i=0; i < m_cache_assoc; i++) {
    time = m_cache[cacheSet][i]->getLastRef();
    assert(m_cache[cacheSet][i]->getPermission() != AccessPermission_NotPresent);

    if (time < smallest_time){
      smallest_index = i;
      smallest_time = time;
    }
  }

  DEBUG_EXPR(CACHE_COMP, MedPrio, cacheSet);
  DEBUG_EXPR(CACHE_COMP, MedPrio, smallest_index);
  DEBUG_EXPR(CACHE_COMP, MedPrio, *(m_cache[cacheSet][smallest_index]));
  DEBUG_EXPR(CACHE_COMP, MedPrio, *this);
  return m_cache[cacheSet][smallest_index]->getAddress();
}

// looks an address up in the cache
CacheEntryBase& CacheMemory::lookup(const Address& address)
{
  assert(address == line_address(address));
  Index cacheSet = addressToCacheSet(address);
  int loc = findTagInSet(cacheSet, address);
  assert(loc != -1);
  return *(m_cache[cacheSet][loc]);
}

// looks an address up in the cache
const CacheEntryBase& CacheMemory::lookup(const Address& address) const
{
  assert(address == line_address(address));
  Index cacheSet = addressToCacheSet(address);
  int loc = findTagInSet(cacheSet, address);
  assert(loc != -1);
  return *(m_cache[cacheSet][loc]);
}

AccessPermission CacheMemory::getPermission(const Address& address) const
{
  assert(address == line_address(address));
  return lookup(address).getPermission();
}

void CacheMemory::changePermission(const Address& address, AccessPermission new_perm)
{
  assert(address == line_address(address));
  assert((new_perm == AccessPermission_Read_Write) ||
         (new_perm == AccessPermission_Read_Only) ||
         (new_perm == AccessPermission_Invalid));
  g_system_ptr->getDriver()->permissionChangeCallback(m_id, address, getPermission(address), new_perm);
  lookup(address).getPermission() = new_perm;
  assert(getPermission(address) == new_perm);
}

// Sets the most recently used bit for a cache block
void CacheMemory::setMRU(const Address& address)
{
  lookup(address).getLastRef() = g_eventQueue_ptr->getTime();
}

void CacheMemory::recordCacheContents(CacheRecorder& tr, bool is_instruction_cache) const
{
  for (int i = 0; i < m_cache_num_sets; i++) {
    for (int j = 0; j < m_cache_assoc; j++) {
      AccessPermission perm = m_cache[i][j]->getPermission();
      CacheRequestType request_type = CacheRequestType_NULL;
      if (perm == AccessPermission_Read_Only) {
        if (is_instruction_cache) {
          request_type = CacheRequestType_IFETCH;
        } else {
          request_type = CacheRequestType_LD;
        }
      } else if (perm == AccessPermission_Read_Write) {
        request_type = CacheRequestType_ST;
      }

      if (request_type != CacheRequestType_NULL) {
        tr.addRecord(m_id, m_cache[i][j]->getAddress(), 
                     Address(0), request_type, m_cache[i][j]->getLastRef());
      }
    }
  }
}

void CacheMemory::print(ostream& out) const
{ 
  out << "Cache dump: " << m_description << endl;
  for (int i = 0; i < m_cache_num_sets; i++) {
    for (int j = 0; j < m_cache_assoc; j++) {
      out << "  Index: " << i 
          << " way: " << j 
          << " entry: " << *(m_cache[i][j]) << endl;
    }
  }
}
