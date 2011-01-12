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

#ifndef DIRECTORYMEMORY_H
#define DIRECTORYMEMORY_H
#define DRAMSIM 1

#include "Global.h"
#include "Directory_Entry.h"
#include "MessageBuffer.h"
#include "RequestMsg.h"
#include "MachineType.h"
#include "Network.h"
#include <list>
#include <queue>
#include "MemorySystem.h"
using namespace std;

class Network; // network 
class MessageBuffer;
class Address;

#if DRAMSIM
class DirectoryMemory : public Consumer {
#else
class DirectoryMemory {
#endif
public:
  // Constructors
  DirectoryMemory(NodeID id);

  // Destructor
  ~DirectoryMemory();
  
  // Public Methods
  bool isPresent(const Address& address);
  Directory_Entry& lookup(const Address& address);

  void printConfig(ostream& out);
  void print(ostream& out) const;

  static NodeID mapAddressToHomeNode(const Address& addr);

#if DRAMSIM
  void wakeup();
  void read(const RequestMsg& inmsg);
  void write(const RequestMsg& inmsg);
#endif

private:
  // Private Methods
  static integer_t memoryModuleIndex(const Address& addr);
  static int memoryModuleBits();

  // Private copy constructor and assignment operator
  DirectoryMemory(const DirectoryMemory& obj);
  DirectoryMemory& operator=(const DirectoryMemory& obj);
  
#if DRAMSIM
  void read_complete(uint id, uint64_t address, uint64_t clock_cycle);
  void write_complete(uint id, uint64_t address, uint64_t clock_cycle);
  void dram_complete(uint64_t address, bool write);
  void dram_operation(const RequestMsg& inmsg, TransactionType ttype);
#endif

  // Data Members (m_ prefix)
  Directory_Entry **m_entries;
  NodeID m_id;
  int m_memory_module_bits;
  int m_size;  // # of memory module blocks for this directory

#if DRAMSIM
  MemorySystem *m_mem;
  list<RequestMsg> *m_pending_trans;
  queue<Transaction> *m_trans_queue;
  bool m_wakeup;
  int m_mem_timer;
#endif
};

// Output operator declaration
ostream& operator<<(ostream& out, const DirectoryMemory& obj);

// ******************* Definitions *******************

// Output operator definition
extern inline 
ostream& operator<<(ostream& out, const DirectoryMemory& obj)
{
  obj.print(out);
  out << flush;
  return out;
}

#endif //DIRECTORYMEMORY_H
