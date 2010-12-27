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

#ifndef __PYRITE_DRIVER_H
#define __PYRITE_DRIVER_H

#include "Driver.h"

class MemoryInterface;

class PyriteDriver : public Driver {
public:
  // Constructors
  PyriteDriver();

  // Destructor
  virtual ~PyriteDriver();
  
  // Public Methods
  void addMemoryInterface(NodeID proc, MemoryInterface *mem_interface);
  
  virtual void hitCallback(NodeID proc, SubBlock& data);
  virtual integer_t getInstructionCount(int procID) const { return 1; }

  virtual void printStats(ostream& out) const {}
  virtual void clearStats() {}

  virtual void printConfig(ostream& out) const {}

  virtual void permissionChangeCallback(NodeID proc, const Address& addr,
                                        AccessPermission old_perm, AccessPermission new_perm);
protected:
  // accessible by subclasses

private:
  Vector<MemoryInterface *> m_mem_interface;
};

#endif // __PYRITE_DRIVER_H
