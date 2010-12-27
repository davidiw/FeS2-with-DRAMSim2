// ----------------------------------------------------------------------
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
// ----------------------------------------------------------------------

#include <simics/api.h>
#undef unlikely
#undef likely
#include <simics/arch/x86.h>
#include <simics/alloc.h>
#include <simics/utils.h>

#include "Processor.h"
#include "PyriteDriver.h"
#include "System.h"
#include "SubBlock.h"
#include "Profiler.h"
#include "AddressProfiler.h"
#include "Param.h"

#include "MemoryInterface.h"

// Simics includes

// Contructor
PyriteDriver::PyriteDriver()
{
  // setup processor objects
  m_mem_interface.setSize(g_param_ptr->NUM_NODES());
  for(int i=0; i < g_param_ptr->NUM_NODES(); i++){
    m_mem_interface[i] = NULL;
  }
}

// Destructor
PyriteDriver::~PyriteDriver()
{
  m_mem_interface.clear();
}

void 
PyriteDriver::addMemoryInterface(NodeID proc, MemoryInterface *mem_interface) {
  assert(m_mem_interface.size() >= proc);
  assert(m_mem_interface[proc] == NULL);
  assert(mem_interface != NULL);
  m_mem_interface[proc] = mem_interface;
}

/******************************************************************
 * void hitCallback(int cpuNumber)
 * Called by Sequencer when the data is ready in the cache
 ******************************************************************/
void PyriteDriver::hitCallback(NodeID proc, SubBlock& data)
{
  physical_address_t paddr = data.getAddress().getAddress();
  m_mem_interface[proc]->fillRequest(paddr);
}

void PyriteDriver::permissionChangeCallback(NodeID proc, const Address& addr,
                                            AccessPermission old_perm, AccessPermission new_perm) {
  assert(addr.isBlockAligned());
  g_processors_vec[proc]->cachePermissionChangeNotification(addr.getAddress(), old_perm, new_perm);
}
