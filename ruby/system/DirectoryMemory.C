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

#include "System.h"
#include "Driver.h"
#include "DirectoryMemory.h"
#include "Address.h"
#include "Param.h"

DirectoryMemory::DirectoryMemory(NodeID id)
{
  m_id = id;
  m_size = int64(1) << memoryModuleBits();
  // allocates an array of directory entry pointers & sets them to NULL
  m_entries = new Directory_Entry*[m_size];  
  if (m_entries == NULL) {
    ERROR_MSG("Directory Memory: unable to allocate memory.");
  }

  for (int i=0; i < m_size; i++) {
    m_entries[i] = NULL;
  }

#if DRAMSIM
  m_mem_timer = g_param_ptr->SIMICS_RUBY_MULTIPLIER();
  /* pick a DRAM part to simulate */
  m_mem = new MemorySystem(0, "ram.ini", "system.ini", ".", "resultsfilename");

  /* create and register our callback functions */
  Callback_t *read_cb = new Callback<DirectoryMemory, void, uint, uint64_t, uint64_t>
    (this, &DirectoryMemory::read_complete);
  Callback_t *write_cb = new Callback<DirectoryMemory, void, uint, uint64_t, uint64_t>
    (this, &DirectoryMemory::write_complete);
  m_mem->RegisterCallbacks(read_cb, write_cb, NULL);

  m_pending_trans = new list<RequestMsg>();
  m_wakeup = true;
  g_eventQueue_ptr->scheduleEvent(this, m_mem_timer);
#endif
}

#if DRAMSIM
void DirectoryMemory::wakeup()
{
  if(m_pending_trans->empty()) {
    m_wakeup = false;
    return;
  }
  g_eventQueue_ptr->scheduleEvent(this, m_mem_timer);
  m_mem->update();
}
#endif

DirectoryMemory::~DirectoryMemory()
{
  // free up all the directory entries
  for (int i=0; i < m_size; i++) {
    if (m_entries[i] != NULL) {
      delete m_entries[i];
      m_entries[i] = NULL;
    }
  }

  // free up the array of directory entries
  delete[] m_entries;

#if DRAMSIM
  delete(m_pending_trans);
  delete(m_mem);
#endif
}

void DirectoryMemory::printConfig(ostream& out)
{
  out << "memory_bits: " << g_param_ptr->MEMORY_SIZE_BITS() << endl;
  int64 bytes = (int64(1) << g_param_ptr->MEMORY_SIZE_BITS());
  out << "memory_size_bytes: " << bytes << endl; 
  out << "memory_size_Kbytes: " << double(bytes) / (1<<10) << endl;
  out << "memory_size_Mbytes: " << double(bytes) / (1<<20) << endl;
  out << "memory_size_Gbytes: " << double(bytes) / (1<<30) << endl;

  int data_block_bytes = 1 << g_param_ptr->DATA_BLOCK_BITS();
  out << "module_bits: " <<  memoryModuleBits() << endl;
  out << "module_size_lines: " << (int64(1) << memoryModuleBits()) << endl;
  out << "module_size_bytes: " << (int64(1) << memoryModuleBits()) * data_block_bytes << endl;
  out << "module_size_Kbytes: " << double((int64(1) << memoryModuleBits()) * data_block_bytes) / (1<<10) << endl;
  out << "module_size_Mbytes: " << double((int64(1) << memoryModuleBits()) * data_block_bytes) / (1<<20) << endl;
}

// Public method
bool DirectoryMemory::isPresent(const Address& address)
{
  return (mapAddressToHomeNode(address) == m_id);
}

Directory_Entry& DirectoryMemory::lookup(const Address& address)
{
  assert(isPresent(address));
  Index index = memoryModuleIndex(address);

  if ((index < 0) || (index >= m_size)) {
    WARN_MSG("Directory Memory Assertion: accessing memory out of range");
    WARN_EXPR(address);
    WARN_EXPR(index);
    WARN_EXPR(m_size);
    WARN_EXPR(memoryModuleBits());
    WARN_EXPR(m_id); 
    ERROR_MSG("Aborting");
  }

  assert(index >= 0);
  assert(index < m_size);

  Directory_Entry* entry = m_entries[index];

  // allocate the directory entry on demand.
  if (entry == NULL) {
    entry = new Directory_Entry;
    m_entries[index] = entry;    
  }

  return (*entry);
}

void DirectoryMemory::print(ostream& out) const
{
  out << "Directory dump: " << endl;
  for (int i=0; i < m_size; i++) {
    if (m_entries[i] != NULL) {
      out << i << ": ";
      out << *m_entries[i] << endl;
    }
  }
}

// Determine the home node for a particular address.
NodeID DirectoryMemory::mapAddressToHomeNode(const Address& addr)
{
  // FIXME - this method currently interleaves cache blocks to home
  // nodes at a very fine granularity.  This approach seems fine for
  // very uniform memory access systems.  However, for more
  // non-uniform systems, we probably want to allocate chunks of
  // memory to a memory module at a larger granularity (e.g., an
  // entire page).  We _certainly_ don't want to interleave by the
  // highest-order memory bits, as this will likely lead to serious
  // load imbalance between the memory modules.

  NodeID id = addr.bitSelect(g_param_ptr->DATA_BLOCK_BITS(),
                             g_param_ptr->DATA_BLOCK_BITS()+g_param_ptr->NUM_NODES_BITS()-1);

  assert(id < g_param_ptr->NUM_NODES());
  assert(id >= 0);
  return id;
}

integer_t DirectoryMemory::memoryModuleIndex(const Address& addr)
{
  integer_t index = addr.bitSelect(g_param_ptr->DATA_BLOCK_BITS()+g_param_ptr->NUM_NODES_BITS(), ADDRESS_WIDTH);
  return index;
}

int DirectoryMemory::memoryModuleBits()
{
  return g_param_ptr->MEMORY_SIZE_BITS() - g_param_ptr->DATA_BLOCK_BITS() - g_param_ptr->NUM_NODES_BITS();
}

#if DRAMSIM
void DirectoryMemory::read_complete(uint id, uint64_t address, uint64_t clock_cycle)
{
  dram_complete(address, false);
}

void DirectoryMemory::write_complete(uint id, uint64_t address, uint64_t clock_cycle)
{
  dram_complete(address, true);
}

void DirectoryMemory::dram_complete(uint64_t address, bool write)
{
  // Find the entry in the pending queue
  list<RequestMsg>::iterator it;
  RequestMsg msg;
  for (it = m_pending_trans->begin(); it != m_pending_trans->end(); it++) {
    msg = *it;
    // FIXME: should check to make sure this is a read or a write
    if(msg.getAddress().getAddress() == address) {
      break;
    }
  }

  // None exists...
  if(it == m_pending_trans->end()) {
    return;
  }

  // Found one, remove it and send notification to the Requestor
  m_pending_trans->erase(it);
  Network* net = g_system_ptr->getNetwork();
  MessageBuffer* mb = net->getFromNetQueue(MachineType_Directory, msg.getRequestor(), false, 4);
  mb->enqueue(msg, 1);
}

void DirectoryMemory::read(const RequestMsg& inmsg)
{
  dram_operation(inmsg, DATA_READ);
}

void DirectoryMemory::write(const RequestMsg& inmsg)
{
  dram_operation(inmsg, DATA_WRITE);
}

void DirectoryMemory::dram_operation(const RequestMsg& inmsg, TransactionType ttype)
{
  if(m_wakeup == false) {
    m_wakeup = true;
    g_eventQueue_ptr->scheduleEvent(this, m_mem_timer);
  }

  physical_address_t addr = inmsg.getAddress().getAddress();
  Transaction tr = Transaction(ttype, addr, NULL);
  m_mem->addTransaction(tr);
  m_pending_trans->push_back(inmsg);
}
#endif
