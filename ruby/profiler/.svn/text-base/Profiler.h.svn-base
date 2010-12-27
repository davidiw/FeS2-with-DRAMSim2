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

#ifndef PROFILER_H
#define PROFILER_H

#include "Global.h"
#include "MachineType.h"
#include "Histogram.h"
#include "Consumer.h"
#include "AccessModeType.h"
#include "AccessType.h"
#include "NodeID.h"
#include "PrefetchBit.h"
#include "Address.h"
#include "Set.h"
#include "CacheRequestType.h"

class CacheMsg;
class CacheProfiler;
class AddressProfiler;
template <class KEY_TYPE, class VALUE_TYPE> class Map;

class Profiler : public Consumer {
public:
  // Constructors
  Profiler();

  // Destructor
  ~Profiler();  
  
  // Public Methods
  void wakeup();

  void printStats(ostream& out) const;
  void printTraceStats(ostream& out) const;
  void clearStats();
  void printConfig(ostream& out) const {}
  void printResourceUsage(ostream& out) const;

  AddressProfiler* getAddressProfiler() { return m_address_profiler_ptr; }

  AddressProfiler* getInstructionProfiler() { return m_inst_profiler_ptr; }

  void addPrimaryStatSample(const CacheMsg& msg, NodeID id);
  void addSecondaryStatSample(CacheRequestType request_type, AccessModeType type, int msgSize, PrefetchBit pfBit, NodeID id);
  void addAddressTraceSample(const CacheMsg& msg, NodeID id);

  void profileSharing(const Address& addr, AccessType type, NodeID requestor, const Set& sharers, const Set& owner);

  void profileConflictingRequests(const Address& addr);
  void profileOutstandingRequest(int outstanding) { m_outstanding_requests.add(outstanding); }
  void profileOutstandingPersistentRequest(int outstanding) { m_outstanding_persistent_requests.add(outstanding); }

  void countBAUnicast() { m_num_BA_unicasts++; }
  void countBABroadcast() { m_num_BA_broadcasts++; }

  void controllerBusy(NodeID node);
  void missLatency(Time t, CacheRequestType type);

  void tbeUsageSample(int num) { m_tbeProfile.add(num); }
  void sequencerRequests(int num) { m_sequencer_requests.add(num); }
  
  void profileGetXMaskPrediction(const Set& pred_set);
  void profileGetSMaskPrediction(const Set& pred_set);
  void profileTrainingMask(const Set& pred_set);
  void profileTransition(const string& component, NodeID id, Address addr, 
                         const string& state, const string& event, 
                         const string& next_state, const string& note);

  int64 getTotalInstructionsExecuted() const;

  integer_t getNumL1Misses(NodeID id) { return m_perProcL1Misses[id]; }
  integer_t getNumL2Misses(NodeID id) { return m_perProcL2Misses[id]; }
 
  void print(ostream& out) const {}
  static Profiler* create() { return new Profiler; }
private:
  // Private Methods

  // Private copy constructor and assignment operator
  Profiler(const Profiler& obj);
  Profiler& operator=(const Profiler& obj);
  
  // Data Members (m_ prefix)
  CacheProfiler* m_L1D_cache_profiler_ptr;
  CacheProfiler* m_L1I_cache_profiler_ptr;
  CacheProfiler* m_L2_cache_profiler_ptr;
  AddressProfiler* m_address_profiler_ptr;
  AddressProfiler* m_inst_profiler_ptr;

  Vector<int64> m_instructions_executed_at_start;
  
  ostream* m_periodic_output_file_ptr;

  Time m_ruby_start;
  time_t m_real_time_start_time;

  int m_num_BA_unicasts;
  int m_num_BA_broadcasts;

  Vector<integer_t> m_perProcL1Misses;
  Vector<integer_t> m_perProcL2Misses;
  Vector<integer_t> m_perProcBusyController;
  Histogram m_tbeProfile;
  Histogram m_sequencer_requests;
  Histogram m_read_sharing_histogram;
  Histogram m_write_sharing_histogram;
  Histogram m_all_sharing_histogram;
  int64 m_cache_to_cache;
  int64 m_memory_to_cache;

  Vector<Histogram> m_missLatencyHistograms;
  Histogram m_allMissLatencyHistogram;

  Histogram m_gets_mask_prediction;
  Histogram m_getx_mask_prediction;

  // For profiling possibly conflicting requests
  Map<Address, Time>* m_conflicting_map_ptr;
  Histogram m_conflicting_histogram;

  Histogram m_outstanding_requests;
  Histogram m_outstanding_persistent_requests;
};

// Output operator declaration
ostream& operator<<(ostream& out, const Profiler& obj);

// ******************* Definitions *******************

// Output operator definition
extern inline 
ostream& operator<<(ostream& out, const Profiler& obj)
{
  obj.print(out);
  out << flush;
  return out;
}

#endif //PROFILER_H
