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

#ifndef ADDRESSPROFILER_H
#define ADDRESSPROFILER_H

#include "Global.h"
#include "NodeID.h"
#include "Histogram.h"
#include "Address.h"
#include "CacheMsg.h"
#include "AccessType.h"

class AccessTraceForAddress;
class Set;
template <class KEY_TYPE, class VALUE_TYPE> class Map;

class AddressProfiler {
public:
  // Constructors
  AddressProfiler();

  // Destructor
  ~AddressProfiler();  
  
  // Public Methods
  void printStats(ostream& out) const;
  void clearStats();

  void addTraceSample(Address data_addr, Address pc_addr, CacheRequestType type, AccessModeType access_mode, NodeID id, bool sharing_miss);
  void profileRetry(const Address& data_addr, AccessType type, int count);
  void profileGetX(const Address& datablock, const Address& PC, const Set& owner, const Set& sharers, NodeID requestor);
  void profileGetS(const Address& datablock, const Address& PC, const Set& owner, const Set& sharers, NodeID requestor);

  void print(ostream& out) const;
private:
  // Private Methods

  // Private copy constructor and assignment operator
  AddressProfiler(const AddressProfiler& obj);
  AddressProfiler& operator=(const AddressProfiler& obj);
  
  // Data Members (m_ prefix)
  int64 m_sharing_miss_counter;

  Map<Address, AccessTraceForAddress>* m_dataAccessTrace;
  Map<Address, AccessTraceForAddress>* m_macroBlockAccessTrace;
  Map<Address, AccessTraceForAddress>* m_programCounterAccessTrace;
  Map<Address, AccessTraceForAddress>* m_retryProfileMap;
  Histogram m_retryProfileHisto;
  Histogram m_retryProfileHistoWrite;
  Histogram m_retryProfileHistoRead;
  Histogram m_getx_sharing_histogram;
  Histogram m_gets_sharing_histogram;
};

// Output operator declaration
ostream& operator<<(ostream& out, const AddressProfiler& obj);

// ******************* Definitions *******************

// Output operator definition
extern inline 
ostream& operator<<(ostream& out, const AddressProfiler& obj)
{
  obj.print(out);
  out << flush;
  return out;
}

#endif //ADDRESSPROFILER_H
