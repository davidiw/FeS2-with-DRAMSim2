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

#include <cmath>
#include <simics/api.h>
#undef unlikely
#undef likely
#include <simics/arch/x86.h>
#include <simics/alloc.h>
#include <simics/utils.h>

#include "pyrite.h"
#include "globals.h"
#include "terminal-colors.h"

#include "TraceDecoder.h"
#include "EventLog.h"
#include "Debug.h"
#include "params.h"

#include "Box.h"
#include "Map.h"
#include "Vector.h"
#include "Processor.h"
#include "DoubleWord.h"

#include "System.h"
#include "init.h"
#include "EventQueue.h"
#include "Param.h"
#include "Profiler.h"

using namespace std;
extern Map<Waddr, string> g_code_map;

static int print_interval = 100000;
bool g_break_simulation = false;
bool g_functional_only = false;

/* to get the correct enums */
#define TARGET_X86
#define SIM_FAST_ACCESS_FUNCTIONS

extern "C" {

  //have to declare module initialization function with C linkage so that Simics can find it
  void init_local(void);

}

void rubyInit();

static int s_advance_counter = 0;

static void processors_step_cycle(void){
  if (g_params.getPrintIntermediateStats()) {
    if ((g_stats.getTotalCycles() % print_interval) == 0){
      print();
    }
    if (g_stats.getTotalCycles() == 10 * print_interval){
      print_interval *= 2;
    }

    if ((g_stats.getTotalCycles() % 1000000) == 0){
      print();
    }
  }

  if (SIM_number_processors() == 1) { 
    g_processors_vec[0]->stepCycle();
  } else {
    for (int i = 0; i < SIM_number_processors(); i++){
      conf_object_t* cpu = SIM_get_processor(i);
      assert(!SIM_processor_enabled(cpu));
      SIM_enable_processor(cpu);
      CHECK_SIM_EXCEPTION();
      g_processors_vec[i]->stepCycle();
      SIM_disable_processor(cpu);
      CHECK_SIM_EXCEPTION();
    }
  }

  // g_simics_driver.endOfCycle();
  g_stats.incrementTotalCycles();
  
  if (g_params.getRuby()) {
    // advance ruby time
    s_advance_counter++;
    if (s_advance_counter == g_param_ptr->SIMICS_RUBY_MULTIPLIER()) {
      Time time = g_eventQueue_ptr->getTime() + 1;
      g_eventQueue_ptr->triggerEvents(time);
      s_advance_counter = 0;
    }
  }
}

static void init_processors(void){
  assert(g_params.getNumProcessors() == SIM_number_processors());
  assert(g_params.getRuby() || (SIM_number_processors() == 1));
  for (int i = 0; i < SIM_number_processors(); i++){
    Processor* curr = new Processor(i);
    curr->init();
    g_processors_vec.insertAtBottom(curr);
  }
  DoubleWord::setAddressMask(~((Waddr)(1 << g_params.getMemoryBlockBits()) - 1));
}

static void step_n_cycles(int n){
  if (!g_params.getPyriteIsControlling()){
    g_params.setPyriteIsControlling(true);
    init_processors();
  }

  for (int i = 0; i < n; i++){
    processors_step_cycle();
  }
}

static void init(void){
  if (g_params.getRuby()) {
    printf("RUBY = 1\n");
    rubyInit();
  } else {
    printf("RUBY = 0\n");
  }

  g_stats.init();
  if (g_params.getPyriteIsControlling()){
    init_processors();
  }
  CHECK_SIM_EXCEPTION();
}

void rubyInit() {
  if (!g_param_ptr) {
    cout << "Creating parameter object..." << endl;
    g_param_ptr = new Param();
    cout << "Creating parameter object done" << endl;
  }
  
  // Hardcode specific parameters
  g_param_ptr->set_SIMICS(true);
  g_param_ptr->set_NUM_NODES(SIM_number_processors());
  g_param_ptr->set_SEQUENCER_OUTSTANDING_REQUESTS(16 + /* fudge room */4);
  g_param_ptr->set_RANDOMIZATION(false);

  g_param_ptr->set_NULL_LATENCY(1);
  g_param_ptr->set_ISSUE_LATENCY(2);

  g_param_ptr->set_SEQUENCER_TO_CONTROLLER_LATENCY(4+4);  // to make L2 hit time 17?
  g_param_ptr->set_CACHE_LATENCY(1);
  g_param_ptr->set_DIRECTORY_LATENCY(1);
  g_param_ptr->set_MEMORY_LATENCY(35);  // fixme
  g_param_ptr->set_NETWORK_LINK_LATENCY(1);

  g_param_ptr->set_MEMORY_SIZE_BITS(g_params.getMemorySizeBits());

  g_param_ptr->set_L1_CACHE_ASSOC(g_params.getL1DataAssoc());
  g_param_ptr->set_L1_CACHE_NUM_SETS_BITS(g_params.getL1DataNumSetBits());
  g_param_ptr->set_L2_CACHE_ASSOC(g_params.getL2Assoc());
  g_param_ptr->set_L2_CACHE_NUM_SETS_BITS(g_params.getL2NumSetBits());

  g_param_ptr->set_DATA_BLOCK_BITS(g_params.getMemoryBlockBits());

  init_simulator();
  
  g_system_ptr->printConfig(cout);
  cout << "Testing clear stats...";
  g_system_ptr->clearStats();
  cout << "Done." << endl;
}

void run(void){
  if (!g_params.getPyriteIsControlling()){
    g_params.setPyriteIsControlling(true);
    init_processors();
  }

  for(;;){
    if (g_functional_only) {
      if (SIM_number_processors() != 1) { 
        for (int i = 0; i < SIM_number_processors(); i++){
          conf_object_t* cpu = SIM_get_processor(i);
          assert(!SIM_processor_enabled(cpu));
          SIM_enable_processor(cpu);
        }
      }
      SIM_continue(0);
      if (SIM_number_processors() != 1) { 
        for (int i = 0; i < SIM_number_processors(); i++){
          conf_object_t* cpu = SIM_get_processor(i);
          assert(SIM_processor_enabled(cpu));
          SIM_disable_processor(cpu);
        }
      }
    }

    processors_step_cycle();

    if (g_break_simulation){
      g_break_simulation = false;
      break;
    }
  }

  if (SIM_number_processors() != 1) { 
    for (int i = 0; i < SIM_number_processors(); i++){
      conf_object_t* cpu = SIM_get_processor(i);
      assert(!SIM_processor_enabled(cpu));
      SIM_enable_processor(cpu);
    }
  } 
}

static void print_register_state(void){
  //TODO: how to convert register numbers to names? how to print out 80
  //registers?
  //cout << "Eflags: " << hex << g_register_values[REG_flags] << dec << endl;
}

static void print_processor_state(void){
  for (int i = 0; i < SIM_number_processors(); i++){
    if (g_params.getRuby()) {
      // merge ruby miss stats into global stats
      g_stats.setL1Misses(i, g_system_ptr->getProfiler()->getNumL1Misses(i));
      g_stats.setL2Misses(i, g_system_ptr->getProfiler()->getNumL2Misses(i));
    }
    g_processors_vec[i]->print();
  }
  print_register_state();
}

void print(void){
  print_processor_state();
  
  g_stats.dumpStats(cout);
  ofstream statsFile;
  statsFile.open(g_params.getStatsFilePath().c_str());
  g_stats.dumpStats(statsFile);
  statsFile.close();

  g_eventLog.print(cout);

  if (strcmp(g_params.getDisasmFilePath().c_str(), "/dev/null") != 0) {
    ofstream disasmFile;
    disasmFile.open(g_params.getDisasmFilePath().c_str());
    Vector<Waddr> keys = g_code_map.keys();
    keys.sortVector();
    for (int i = 0 ; i < keys.size() ; ++ i) {
      disasmFile << g_code_map.lookup(keys[i]);
    }
    disasmFile.close();
  }
}

/************* Simics setup functions ************/

static set_error_t set_run_parameter(void *dont_care, conf_object_t *obj, attr_value_t *val, attr_value_t *idx){
  attr_kind_t idx_kind = idx->kind;
  assert(idx_kind == Sim_Val_Nil);

  run();

  return Sim_Set_Ok;
}

static set_error_t set_init_parameter(void *dont_care, conf_object_t *obj, attr_value_t *val, attr_value_t *idx){
  attr_kind_t idx_kind = idx->kind;
  assert(idx_kind == Sim_Val_Nil);

  init();

  return Sim_Set_Ok;
}

static set_error_t set_step_cycle_parameter(void *dont_care, conf_object_t *obj, attr_value_t *val, attr_value_t *idx){

  attr_kind_t val_kind = val->kind;
  attr_kind_t idx_kind = idx->kind;
  
  assert(val_kind == Sim_Val_Integer);
  int num_cycles = val->u.integer;
  assert(idx_kind == Sim_Val_Nil);
   
  step_n_cycles(num_cycles);

  return Sim_Set_Ok;
}

static set_error_t set_print_parameter(void *dont_care, conf_object_t *obj, attr_value_t *val, attr_value_t *idx){
  attr_kind_t idx_kind = idx->kind;
  assert(idx_kind == Sim_Val_Nil);
   
  print();

  return Sim_Set_Ok;

}

static set_error_t set_switch_to_warmup(void *dont_care, conf_object_t *obj, attr_value_t *val, attr_value_t *idx){
  if (g_functional_only) {
    g_functional_only = false;
    SIM_break_cycle( SIM_current_processor(), 1 );
  }

  printf("SWITCH TO WARMUP\n");
  g_stats.clearStats();
  CHECK_SIM_EXCEPTION();
  g_stats.initStats();
  for (int i = 0; i < SIM_number_processors(); i++) {
    g_processors_vec[i]->reset();
    g_processors_vec[i]->warmup();  
  }
  if (g_params.getRuby()) {
    g_eventQueue_ptr->triggerAllEvents();
    g_system_ptr->clearStats();
  }
  return Sim_Set_Ok;
}

static set_error_t set_switch_to_timing(void *dont_care, conf_object_t *obj, attr_value_t *val, attr_value_t *idx){
  if (g_functional_only) {
    g_functional_only = false;
    SIM_break_cycle( SIM_current_processor(), 1 );
  }

  printf("SWITCH TO TIMING\n");
  g_stats.clearStats();
  CHECK_SIM_EXCEPTION();
  g_stats.initStats();
  for (int i = 0; i < SIM_number_processors(); i++) {
    g_processors_vec[i]->reset();
    g_processors_vec[i]->dataCollect();  
  }
  if (g_params.getRuby()) {
    g_eventQueue_ptr->triggerAllEvents();
    g_system_ptr->clearStats();
  }
  return Sim_Set_Ok;
}

static set_error_t set_switch_to_functional(void *dont_care, conf_object_t *obj, attr_value_t *val, attr_value_t *idx){
  printf("SWITCH TO FUNCTIONAL\n");
  g_functional_only = true;
  return Sim_Set_Ok;
}

static set_error_t set_break_simulation(void *dont_care, conf_object_t *obj, attr_value_t *val, attr_value_t *idx){
  if (g_functional_only) {
    g_functional_only = false;
    SIM_break_cycle( SIM_current_processor(), 1 );
  }

  g_break_simulation = true;
  return Sim_Set_Ok;
}

static unsigned g_marker_count = 0;

const int OFFSET_SIZE = 4;

static conf_object_t * pyrite_new_instance(parse_object_t *pa){

  pyrite_object_t *pyrite = MM_ZALLOC(1, pyrite_object_t);
  SIM_object_constructor(&pyrite->obj, pa);
  pyrite->count = 0;
  pyrite->stall_time = 0;
  pyrite->next_timing_model = NULL;
  memset(&pyrite->next_timing_iface, 0, sizeof(pyrite->next_timing_iface));
  pyrite->trace_file = NULL;
  pyrite->trace_file_name = NULL;
  
  return &pyrite->obj;

}

// auto-generated code for Simics wrappers
#include "params_wrappers.inc"
#include "stats_wrappers.inc"

void init_local(void){

  class_data_t class_data;
  conf_class_t *myClass;

  cout << "Initializing pyrite module" << endl;
 
  /* initialize and register the pyrite class */
  memset(&class_data, 0, sizeof class_data);
  class_data.new_instance = pyrite_new_instance;

  myClass = SIM_register_class("pyrite", &class_data);
  CHECK_SIM_EXCEPTION();

  /* initialize attributes */
  SIM_register_typed_attribute(myClass, "step_cycle",
                               NULL, NULL,
                               set_step_cycle_parameter, NULL,
                               Sim_Attr_Pseudo,
                               "i", NULL,
                               "");
  
  SIM_register_typed_attribute(myClass, "run",
                               NULL, NULL,
                               set_run_parameter, NULL,
                               Sim_Attr_Pseudo,
                               "i", NULL,
                               "");
  
  SIM_register_typed_attribute(myClass, "switch_to_warmup",
                               NULL, NULL,
                               set_switch_to_warmup, NULL,
                               Sim_Attr_Pseudo,
                               "i", NULL,
                               "");
  
  SIM_register_typed_attribute(myClass, "switch_to_timing",
                               NULL, NULL,
                               set_switch_to_timing, NULL,
                               Sim_Attr_Pseudo,
                               "i", NULL,
                               "");
  
  SIM_register_typed_attribute(myClass, "switch_to_functional",
                               NULL, NULL,
                               set_switch_to_functional, NULL,
                               Sim_Attr_Pseudo,
                               "i", NULL,
                               "");
  
  SIM_register_typed_attribute(myClass, "break_simulation",
                               NULL, NULL,
                               set_break_simulation, NULL,
                               Sim_Attr_Pseudo,
                               "i", NULL,
                               "");
  
  SIM_register_typed_attribute(myClass, "init",
                               NULL, NULL,
                               set_init_parameter, NULL,
                               Sim_Attr_Pseudo,
                               "i", NULL,
                               "");

  SIM_register_typed_attribute(myClass, "print_state",
                               NULL, NULL,
                               set_print_parameter, NULL,
                               Sim_Attr_Pseudo,
                               "i", NULL,
                               "");

  // initialize auto-generated attributes
#include "stats_attributes.inc"
#include "params_attributes.inc"
}

DLL_EXPORT void fini_local(void){ }
