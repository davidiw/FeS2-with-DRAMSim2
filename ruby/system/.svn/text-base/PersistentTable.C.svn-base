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

#include "PersistentTable.h"
#include "Set.h"
#include "Map.h"
#include "Address.h"
#include "util.h"

class PersistentTableEntry {
public:
  Set m_starving;
  Set m_marked;
  Set m_request_to_write;
};

PersistentTable::PersistentTable(NodeID id)
{
  m_id = id;
  m_map_ptr = new Map<Address, PersistentTableEntry>;
}

PersistentTable::~PersistentTable()
{
  delete m_map_ptr;
  m_map_ptr = NULL;
}  

void PersistentTable::persistentRequestLock(const Address& address, NodeID locker, AccessType type)
{
  assert(address == line_address(address));
  if (!m_map_ptr->exist(address)) {
    // Allocate if not present
    PersistentTableEntry entry;
    entry.m_starving.add(locker);
    if (type == AccessType_Write) {
      entry.m_request_to_write.add(locker);
    }
    m_map_ptr->add(address, entry);
  } else {
    PersistentTableEntry& entry = m_map_ptr->lookup(address);
    assert(!(entry.m_starving.isElement(locker))); // Make sure we're not already in the locked set

    entry.m_starving.add(locker);
    if (type == AccessType_Write) {
      entry.m_request_to_write.add(locker);
    }
    assert(entry.m_marked.isSubset(entry.m_starving));
  }
}

void PersistentTable::persistentRequestUnlock(const Address& address, NodeID unlocker)
{
  assert(address == line_address(address));
  assert(m_map_ptr->exist(address));
  PersistentTableEntry& entry = m_map_ptr->lookup(address);
  assert(entry.m_starving.isElement(unlocker)); // Make sure we're in the locked set
  assert(entry.m_marked.isSubset(entry.m_starving));
  entry.m_starving.remove(unlocker);
  entry.m_marked.remove(unlocker);
  entry.m_request_to_write.remove(unlocker);
  assert(entry.m_marked.isSubset(entry.m_starving));

  // Deallocate if empty
  if (entry.m_starving.isEmpty()) {
    assert(entry.m_marked.isEmpty());
    m_map_ptr->erase(address);
  }
}

bool PersistentTable::okToIssueStarving(const Address& address) const
{
  assert(address == line_address(address));
  if (!m_map_ptr->exist(address)) {
    return true; // No entry present
  } else if (m_map_ptr->lookup(address).m_starving.isElement(m_id)) {
    return false; // We can't issue another lockdown until are previous unlock has occurred
  } else {
    return (m_map_ptr->lookup(address).m_marked.isEmpty());
  }
}

NodeID PersistentTable::findSmallest(const Address& address) const
{
  assert(address == line_address(address));
  assert(m_map_ptr->exist(address));
  const PersistentTableEntry& entry = m_map_ptr->lookup(address);
  return entry.m_starving.smallestElement();
}

AccessType PersistentTable::typeOfSmallest(const Address& address) const
{
  assert(address == line_address(address));
  assert(m_map_ptr->exist(address));
  const PersistentTableEntry& entry = m_map_ptr->lookup(address);
  if (entry.m_request_to_write.isElement(entry.m_starving.smallestElement())) {
    return AccessType_Write;
  } else {
    return AccessType_Read;
  }
}

void PersistentTable::markEntries(const Address& address)
{
  assert(address == line_address(address));
  if (m_map_ptr->exist(address)) {
    PersistentTableEntry& entry = m_map_ptr->lookup(address);
    assert(entry.m_marked.isEmpty());  // None should be marked
    entry.m_marked = entry.m_starving; // Mark all the nodes currently in the table
  }
}

bool PersistentTable::isLocked(const Address& address) const
{
  assert(address == line_address(address));
  // If an entry is present, it must be locked
  return (m_map_ptr->exist(address));
}
