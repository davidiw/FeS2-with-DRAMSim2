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

#ifndef __RUBY_MEMORY_INTERFACE
#define __RUBY_MEMORY_INTERFACE

//===-- RubyMemoryInterface.h - memory model adapter for Ruby ----*- C++ -*--=//
//
//! Adapts the MemoryModelInterface to what's expected by Ruby
//
//===----------------------------------------------------------------------===//

#include <map>
#include <set>
#include <list>
#include "Waiter.h"
#include "DynamicInst.h"
#include "MemoryInterface.h"
#include "Driver.h"
#include "Global.h"

class RubyMemoryInterface : public MemoryInterface
{
public:
  RubyMemoryInterface(Processor *processor);

  // MemoryInterface implementations
  virtual void request(Waddr addr, RequestType type, Waiter *requester);
  virtual bool ready() const;
};

#endif /* __RUBY_MEMORY_INTERFACE */
