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

#include "Profiler.h"
#include "CacheProfiler.h"
#include "AddressProfiler.h"
#include "System.h"
#include "Network.h"
#include "PrioHeap.h"
#include "CacheMsg.h"
#include "Driver.h"
#include "util.h"
#include "Map.h"

static double process_memory_total();
static double process_memory_resident();

Profiler::Profiler()
  : m_conflicting_histogram(-1)
{
  m_L1D_cache_profiler_ptr = new CacheProfiler("L1D_cache");
  m_L1I_cache_profiler_ptr = new CacheProfiler("L1I_cache");

  m_L2_cache_profiler_ptr = new CacheProfiler("L2_cache");

  m_address_profiler_ptr = new AddressProfiler;
  m_inst_profiler_ptr = NULL;
  if (g_param_ptr->PROFILE_ALL_INSTRUCTIONS()) {
    m_inst_profiler_ptr = new AddressProfiler;
  }

  m_conflicting_map_ptr = new Map<Address, Time>;

  m_real_time_start_time = time(NULL); // Not reset in clearStats()
  clearStats();

  m_periodic_output_file_ptr = NULL;
  if (g_param_ptr->PERIODIC_STATS_INTERVAL() > 0) {
    g_eventQueue_ptr->scheduleEvent(this, 1);

    if (g_param_ptr->PERIODIC_STATS_FILENAME() == "") {
      m_periodic_output_file_ptr = &cout;
      cout << "Recording periodic statistics to stdout every " 
           << g_param_ptr->PERIODIC_STATS_INTERVAL() << " Ruby cycles" << endl;
    } else {
      m_periodic_output_file_ptr = new ofstream(g_param_ptr->PERIODIC_STATS_FILENAME().c_str());
      cout << "Recording periodic statistics to file '" << g_param_ptr->PERIODIC_STATS_FILENAME() 
           << "' every " << g_param_ptr->PERIODIC_STATS_INTERVAL() << " Ruby cycles" << endl;
    }
  }
  //  m_stats_period = 1000000; // Default
}

Profiler::~Profiler()
{
  if (m_periodic_output_file_ptr != &cerr) {
    delete m_periodic_output_file_ptr;
  }
  delete m_address_profiler_ptr;
  delete m_L1D_cache_profiler_ptr;
  delete m_L1I_cache_profiler_ptr;
  delete m_L2_cache_profiler_ptr;
  delete m_conflicting_map_ptr;
}

void Profiler::wakeup()
{
  // FIXME - avoid the repeated code

  Vector<integer_t> perProcInstructionCount;
  perProcInstructionCount.setSize(g_param_ptr->NUM_NODES());

  for(int i=0; i < g_param_ptr->NUM_NODES(); i++) {
    perProcInstructionCount[i] = g_system_ptr->getDriver()->getInstructionCount(i) - m_instructions_executed_at_start[i] + 1;
    // The +1 allows us to avoid division by zero
  }

  integer_t total_l1_misses = m_perProcL1Misses.sum();
  integer_t total_l2_misses = m_perProcL2Misses.sum();
  integer_t instruction_executed = perProcInstructionCount.sum();

  (*m_periodic_output_file_ptr) << "ruby_cycles: " << g_eventQueue_ptr->getTime()-m_ruby_start << endl;
  (*m_periodic_output_file_ptr) << "total_l1_misses: " << total_l1_misses << " " << m_perProcL1Misses << endl;
  (*m_periodic_output_file_ptr) << "total_l2_misses: " << total_l2_misses << " " << m_perProcL2Misses << endl;
  (*m_periodic_output_file_ptr) << "instruction_executed: " << instruction_executed << " " << perProcInstructionCount << endl;
  (*m_periodic_output_file_ptr) << "mbytes_resident: " << process_memory_resident() << endl;
  (*m_periodic_output_file_ptr) << "mbytes_total: " << process_memory_total() << endl;
  if (process_memory_total() > 0) {
    (*m_periodic_output_file_ptr) << "resident_ratio: " << process_memory_resident()/process_memory_total() << endl;
  }

  g_eventQueue_ptr->scheduleEvent(this, g_param_ptr->PERIODIC_STATS_INTERVAL());
}

void Profiler::printStats(ostream& out) const
{
  const time_t T = time(NULL);
  tm *localTime = localtime(&T);
  char buf[100];
  strftime(buf, 100, "%b/%d/%Y %H:%M:%S", localTime);

  out << "Real time: " << buf << endl;
  out << endl;
  out << heading("Profiler Stats");

  time_t real_time_current = time(NULL);
  double seconds = difftime(real_time_current, m_real_time_start_time);
  double minutes = seconds/60.0;
  double hours = minutes/60.0;
  double days = hours/24.0;
  out << "Elapsed_time_in_seconds: " << seconds << endl;
  out << "Elapsed_time_in_minutes: " << minutes << endl;
  out << "Elapsed_time_in_hours: " << hours << endl;
  out << "Elapsed_time_in_days: " << days << endl;
  out << endl;

  out << "Ruby_current_time: " << g_eventQueue_ptr->getTime() << endl;
  out << "Ruby_start_time: " << m_ruby_start << endl;
  Time ruby_cycles = g_eventQueue_ptr->getTime()-m_ruby_start;
  out << "Ruby_cycles: " << ruby_cycles << endl;
  out << endl;

  out << "mbytes_resident: " << process_memory_resident() << endl;
  out << "mbytes_total: " << process_memory_total() << endl;
  if (process_memory_total() > 0) {
    out << "resident_ratio: " << process_memory_resident()/process_memory_total() << endl;
  }
  out << endl;

  m_L1D_cache_profiler_ptr->printStats(out);
  m_L1I_cache_profiler_ptr->printStats(out);
  m_L2_cache_profiler_ptr->printStats(out);

  if(m_num_BA_broadcasts + m_num_BA_unicasts != 0){
    out << endl;
    out << "Broadcast_percent: " << (float)m_num_BA_broadcasts/(m_num_BA_broadcasts+m_num_BA_unicasts) << endl;
  }

  out << endl;

  Vector<integer_t> perProcInstructionCount;
  Vector<double> perProcCPI;
  Vector<double> perProcMissesPerInsn;

  perProcInstructionCount.setSize(g_param_ptr->NUM_NODES());
  perProcCPI.setSize(g_param_ptr->NUM_NODES());
  perProcMissesPerInsn.setSize(g_param_ptr->NUM_NODES());


  for(int i=0; i < g_param_ptr->NUM_NODES(); i++) {
    perProcInstructionCount[i] = g_system_ptr->getDriver()->getInstructionCount(i) - m_instructions_executed_at_start[i] + 1;
    // The +1 allows us to avoid division by zero
    perProcCPI[i] = double(ruby_cycles)/perProcInstructionCount[i];
    perProcMissesPerInsn[i] = 1000.0 * (double(m_perProcL1Misses[i]) / double(perProcInstructionCount[i]));
  }

  integer_t total_misses = m_perProcL1Misses.sum(); // L2 misses is a proper subset of L1 misses
  integer_t instruction_executed = perProcInstructionCount.sum();

  out << "Total_misses: " << total_misses << endl;
  out << "total_l1_misses: " << m_perProcL1Misses.sum() << " " << m_perProcL1Misses << endl;
  out << "total_l2_misses: " <<  m_perProcL2Misses.sum() << " " << m_perProcL2Misses << endl;
  out << endl;
  out << "instruction_executed: " << instruction_executed << " " << perProcInstructionCount << endl;
  out << "cycles_per_instruction: " << (g_param_ptr->NUM_NODES()*double(ruby_cycles))/double(instruction_executed) << " " << perProcCPI << endl;
  out << "misses_per_thousand_instructions: " << 1000.0 * (double(total_misses) / double(instruction_executed)) << " " << perProcMissesPerInsn << endl;
  out << endl;

  out << endl;
  out << "busy_controllers: " << m_perProcBusyController << endl;
  out << "TBE_usage: " << m_tbeProfile << endl;
  out << "sequencer_requests_outstanding: " << m_sequencer_requests << endl;
  out << endl;

  out << "miss_latency: " << m_allMissLatencyHistogram << endl;
  for(int i=0; i<m_missLatencyHistograms.size(); i++) {
    if (m_missLatencyHistograms[i].size() > 0) {
      out << "miss_latency_" << CacheRequestType(i) << ": " << m_missLatencyHistograms[i] << endl;
    }
  }
  out << "gets_mask_prediction_count: " << m_gets_mask_prediction << endl;
  out << "getx_mask_prediction_count: " << m_getx_mask_prediction << endl;
  out << endl;

  if (m_all_sharing_histogram.size() > 0) { 
    out << "all_sharing: " << m_all_sharing_histogram << endl;
    out << "read_sharing: " << m_read_sharing_histogram << endl;
    out << "write_sharing: " << m_write_sharing_histogram << endl;
    
    out << "all_sharing_percent: "; m_all_sharing_histogram.printPercent(out); out << endl;
    out << "read_sharing_percent: "; m_read_sharing_histogram.printPercent(out); out << endl;
    out << "write_sharing_percent: "; m_write_sharing_histogram.printPercent(out); out << endl;

    int64 total_miss = m_cache_to_cache +  m_memory_to_cache;
    out << "all_misses: " << total_miss << endl;
    out << "cache_to_cache_misses: " << m_cache_to_cache << endl;
    out << "memory_to_cache_misses: " << m_memory_to_cache << endl;
    out << "cache_to_cache_percent: " << 100.0 * (double(m_cache_to_cache) / double(total_miss)) << endl;
    out << "memory_to_cache_percent: " << 100.0 * (double(m_memory_to_cache) / double(total_miss)) << endl;
    out << endl;
  }

  if (m_conflicting_histogram.size() > 0) {
    out << "conflicting_histogram: " << m_conflicting_histogram << endl;
    out << "conflicting_histogram_percent: "; m_conflicting_histogram.printPercent(out); out << endl;
    out << endl;
  }

  if (m_outstanding_requests.size() > 0) {
    out << "outstanding_requests: "; m_outstanding_requests.printPercent(out); out << endl;
    if (m_outstanding_persistent_requests.size() > 0) {
      out << "outstanding_persistent_requests: "; m_outstanding_persistent_requests.printPercent(out); out << endl;
    }
    out << endl;
  }
  
  if (!g_param_ptr->PROFILE_ALL_INSTRUCTIONS()) {
    m_address_profiler_ptr->printStats(out);
  }
  
  if (g_param_ptr->PROFILE_ALL_INSTRUCTIONS()) {
    m_inst_profiler_ptr->printStats(out);
  }

  printResourceUsage(out);
}
  
void Profiler::printResourceUsage(ostream& out) const
{
  out << heading("Resource Usage") << endl;
  
  integer_t pagesize = getpagesize(); // page size in bytes
  out << "page_size: " << pagesize << endl;
  
  rusage usage;
  getrusage (RUSAGE_SELF, &usage);

  out << "user_time: " << usage.ru_utime.tv_sec << endl;
  out << "system_time: " << usage.ru_stime.tv_sec << endl;
  // Unfortunately, Linux does not set the following fields
  //  out << "maximum_resident_set_size: " << usage.ru_maxrss * pagesize << endl;
  //  out << "integral_shared_memory_size: " << usage.ru_ixrss * pagesize << endl;
  //  out << "integral_unshared_data_size: " << usage.ru_idrss * pagesize << endl;
  //  out << "integral_unshared_stack_size: " << usage.ru_isrss * pagesize << endl;
  out << "page_reclaims: " << usage.ru_minflt << endl;
  out << "page_faults: " << usage.ru_majflt << endl;
  out << "swaps: " << usage.ru_nswap << endl;
  out << "block_inputs: " << usage.ru_inblock << endl;
  out << "block_outputs: " << usage.ru_oublock << endl;
}

void Profiler::clearStats()
{
  m_num_BA_unicasts = 0;
  m_num_BA_broadcasts = 0;

  m_ruby_start = g_eventQueue_ptr->getTime();
  
  m_instructions_executed_at_start.setSize(g_param_ptr->NUM_NODES());
  for (int i=0; i < g_param_ptr->NUM_NODES(); i++) {
    if (g_system_ptr == NULL) {
      m_instructions_executed_at_start[i] = 0;
    } else {
      m_instructions_executed_at_start[i] = g_system_ptr->getDriver()->getInstructionCount(i);
    }
  }

  m_perProcL1Misses.setSize(g_param_ptr->NUM_NODES());
  m_perProcL2Misses.setSize(g_param_ptr->NUM_NODES());
  m_perProcBusyController.setSize(g_param_ptr->NUM_NODES());

  for(int i=0; i < g_param_ptr->NUM_NODES(); i++) {
    m_perProcL1Misses[i] = 0;
    m_perProcL2Misses[i] = 0;
    m_perProcBusyController[i] = 0;
  }

  m_gets_mask_prediction.clear();
  m_getx_mask_prediction.clear();

  m_missLatencyHistograms.setSize(CacheRequestType_NUM);
  for(int i=0; i<m_missLatencyHistograms.size(); i++) {
    m_missLatencyHistograms[i].clear(200);
  }
  m_allMissLatencyHistogram.clear(200);

  m_tbeProfile.clear();
  m_sequencer_requests.clear();
  m_read_sharing_histogram.clear();
  m_write_sharing_histogram.clear();
  m_all_sharing_histogram.clear();
  m_cache_to_cache = 0;
  m_memory_to_cache = 0;

  if (m_address_profiler_ptr != NULL) {
    m_address_profiler_ptr->clearStats();
  }

  if (m_inst_profiler_ptr != NULL) {
    m_address_profiler_ptr->clearStats();
  }

  // Conflicting requests
  m_conflicting_map_ptr->clear();
  m_conflicting_histogram.clear();

  m_outstanding_requests.clear();
  m_outstanding_persistent_requests.clear();

  m_L1D_cache_profiler_ptr->clearStats();
  m_L1I_cache_profiler_ptr->clearStats();
  m_L2_cache_profiler_ptr->clearStats();
}

void Profiler::profileConflictingRequests(const Address& addr)
{
  assert(addr == line_address(addr));
  Time last_time = m_ruby_start;
  if (m_conflicting_map_ptr->exist(addr)) {
    Time last_time = m_conflicting_map_ptr->lookup(addr);
  }
  Time current_time = g_eventQueue_ptr->getTime();
  assert (current_time - last_time > 0);
  m_conflicting_histogram.add(current_time - last_time);
  m_conflicting_map_ptr->add(addr, current_time);
}

void Profiler::addPrimaryStatSample(const CacheMsg& msg, NodeID id)
{
  m_perProcL1Misses[id]++;
  if (msg.getType() == CacheRequestType_IFETCH) {
    m_L1I_cache_profiler_ptr->addStatSample(msg.getType(), msg.getAccessMode(), msg.getSize(), msg.getPrefetch());
  } else {
    m_L1D_cache_profiler_ptr->addStatSample(msg.getType(), msg.getAccessMode(), msg.getSize(), msg.getPrefetch());
  }
}

void Profiler::addSecondaryStatSample(CacheRequestType request_type, AccessModeType type, int msgSize, PrefetchBit pfBit, NodeID id)
{
  m_perProcL2Misses[id]++;
  m_L2_cache_profiler_ptr->addStatSample(request_type, type, msgSize, pfBit);
}

void Profiler::addAddressTraceSample(const CacheMsg& msg, NodeID id)
{
  if (msg.getType() != CacheRequestType_IFETCH) {

    // Note: The following line should be commented out if you want to
    // use the special profiling that is part of the GS320 protocol

    m_address_profiler_ptr->addTraceSample(msg.getAddress(), msg.getProgramCounter(), msg.getType(), msg.getAccessMode(), id, false);
  }
}

void Profiler::profileSharing(const Address& addr, AccessType type, NodeID requestor, const Set& sharers, const Set& owner)
{
  Set set_contacted(owner);
  if (type == AccessType_Write) {
    set_contacted.addSet(sharers);
  }
  set_contacted.remove(requestor);
  int number_contacted = set_contacted.count();
  
  if (type == AccessType_Write) {
    m_write_sharing_histogram.add(number_contacted);
  } else {
    m_read_sharing_histogram.add(number_contacted);
  }
  m_all_sharing_histogram.add(number_contacted);

  if (number_contacted == 0) {
    m_memory_to_cache++;
  } else {
    m_cache_to_cache++;
  }

}

int64 Profiler::getTotalInstructionsExecuted() const
{
  int64 sum = 1;     // Starting at 1 allows us to avoid division by zero
  for(int i=0; i < g_param_ptr->NUM_NODES(); i++) {
    sum += (g_system_ptr->getDriver()->getInstructionCount(i) - m_instructions_executed_at_start[i]);
  }
  return sum;
}

void Profiler::controllerBusy(NodeID node)
{
  m_perProcBusyController[node]++;
}

void Profiler::missLatency(Time t, CacheRequestType type)
{
  m_allMissLatencyHistogram.add(t);
  m_missLatencyHistograms[type].add(t);
}

void Profiler::profileTransition(const string& component, NodeID id, Address addr, 
                                 const string& state, const string& event, 
                                 const string& next_state, const string& note)
{
  const int EVENT_SPACES = 20;
  const int ID_SPACES = 3;
  const int TIME_SPACES = 7;
  const int COMP_SPACES = 10;
  const int STATE_SPACES = 6;

  if ((g_debug_ptr->getDebugTime() > 0) && 
      (g_eventQueue_ptr->getTime() >= g_debug_ptr->getDebugTime())) {
    cout.flags(ios::right);
    cout << setw(ID_SPACES) << id << " ";
    cout << setw(TIME_SPACES) << g_eventQueue_ptr->getTime() << " ";
    cout << setw(COMP_SPACES) << component;
    cout << setw(EVENT_SPACES) << event << " ";
    for (int i=0; i < g_param_ptr->NUM_NODES(); i++) {
     
      //  Commented code from old profileTransistion(L1) function
      //       string mark = " ";
      //       Node *n = dynamic_cast<Node*>(g_system_ptr->getNode(i));
      //       if (n->m_L1Cache_TBEs_ptr->isPresent(addr)) {
      //        if (n->m_L1Cache_TBEs_ptr->lookup(addr).getIssueCount() >= 3) {
      //          mark = "*";
      //        } else {
      //          mark = ":";
      //        }
      //       } else if (n->m_L1Cache_LockdownTable_ptr->isPresent(addr)) {
      //        mark = "-";
      //       }
      
      if (i == id) {
        cout.flags(ios::right);
        cout << setw(STATE_SPACES) << state;
        cout << ">";
        cout.flags(ios::left);
        cout << setw(STATE_SPACES) << next_state;
      } else {
        cout << setw(STATE_SPACES) << " " << " " << setw(STATE_SPACES) << " ";
      }
    }
    cout << " " << addr << " " << note;
  
    //  Commented code from old profileTransistion(L1) function
    // Node *n = dynamic_cast<Node*>(g_system_ptr->getNode(id));
    // if (n->m_L1Cache_TBEs_ptr->isPresent(addr)) {
    //   cout << ", NumPendingMsg: " << n->m_L1Cache_TBEs_ptr->lookup(addr).getNumPendingMsgs();
    // }

    //  Commented code from old profileTransistion(Directory) function
    // Node *n = dynamic_cast<Node*>(g_system_ptr->getNode(id));
    // cout << ", Sharers: " << n->m_Directory_directory_ptr->lookup(addr).getSharers();
    // cout << ", Owner: " << n->m_Directory_directory_ptr->lookup(addr).getOwner();
    // cout << ", Data: " << n->m_Directory_directory_ptr->lookup(addr).getDataBlk();
    cout << endl;
  }
}

// Helper function
static double process_memory_total()
{
  const double MULTIPLIER = 4096.0/(1024.0*1024.0); // 4kB page size, 1024*1024 bytes per MB, 
  ifstream proc_file;
  proc_file.open("/proc/self/statm");
  int total_size_in_pages = 0;
  int res_size_in_pages = 0;
  proc_file >> total_size_in_pages;
  proc_file >> res_size_in_pages;
  return double(total_size_in_pages)*MULTIPLIER; // size in megabytes
}

static double process_memory_resident()
{
  const double MULTIPLIER = 4096.0/(1024.0*1024.0); // 4kB page size, 1024*1024 bytes per MB, 
  ifstream proc_file;
  proc_file.open("/proc/self/statm");
  int total_size_in_pages = 0;
  int res_size_in_pages = 0;
  proc_file >> total_size_in_pages;
  proc_file >> res_size_in_pages;
  return double(res_size_in_pages)*MULTIPLIER; // size in megabytes
}

void Profiler::profileGetXMaskPrediction(const Set& pred_set)
{
  m_getx_mask_prediction.add(pred_set.count());
}

void Profiler::profileGetSMaskPrediction(const Set& pred_set)
{
  m_gets_mask_prediction.add(pred_set.count());
}
