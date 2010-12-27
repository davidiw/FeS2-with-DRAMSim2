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

#include <simics/api.h>
#undef unlikely
#undef likely
#include <simics/arch/x86.h>
#include <simics/alloc.h>
#include <simics/utils.h>
#include <sstream>

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
#include "AccessType.h"
#include "Scheduler.h"
#include "Event.h"
#include "System.h"
#include "Address.h"
#include "RubyMemoryInterface.h"
#include "SimpleMemoryInterface.h"
#include "DynamicInst.h"

#define PAGE_BYTES 4096
#define PAGE_OFFSET(x) ((x) & (PAGE_BYTES - 1))

Vector<Processor*> g_processors_vec;
EventLog g_eventLog; // accessed via the LOG_EVENT macro

Map<Waddr, string> g_code_map;

unsigned g_x86_fetch_width = 0;
unsigned g_rename_width = 0;
unsigned g_execute_width = 0;
unsigned g_memory_issue_width = 0;
unsigned g_store_issue_width = 0;
unsigned g_retire_width = 0;
unsigned g_L1D_latency = 0;  // the L1 hit latency is already accounted for in the FU latency
unsigned g_L2_latency = 0;
unsigned g_mem_latency = 0;
unsigned g_load_latency = 0;
unsigned g_fpipe_depth = 0; // cfg.getLong(proc, "frontEndStages");  
unsigned g_cpipe_depth = 0; // cfg.getLong(proc, "coreStages");  
unsigned g_rpipe_depth = 0; // cfg.getLong(proc, "coreStages");  
unsigned g_front_end_max = 0;
unsigned g_rob_size = 0;

int g_simics_reg_id[ARCHREG_COUNT];

string get_architectural_exception_name(integer_t arch_except_number){
    
  string ret;
  char* name = (char*)
    SIM_get_exception_name(SIM_get_object("cpu0"),
                           arch_except_number);
    
  //Check for unknown architectural exception
  sim_exception_t simics_exception= SIM_clear_exception();
  if (simics_exception == SimExc_General){
    ret = "Unknown exception";
  } else {
    assert(simics_exception == SimExc_No_Exception);
    assert(name != 0);
    ret = string(name);
  }

  return ret;
}

void exception_callback(void* callback_data, 
                        conf_object_t* trigger_obj, 
                        integer_t exception_number){
  
  conf_object_t* cpu = SIM_current_processor();
  int current_processor_number = SIM_get_current_proc_no();
  int cycle_count = SIM_cycle_count(cpu);
  CHECK_SIM_EXCEPTION();
  g_stats.incrementExceptions(current_processor_number, exception_number);
  LOG_EVENT(DEBUG_EXCEPTION, Box<TransOp>(), "Exception " << exception_number << "(" << get_architectural_exception_name(exception_number) << ") in cycle " << cycle_count);
  g_processors_vec[current_processor_number]->setExceptionOccurred(true);
}

void interrupt_callback(void* callback_data, 
                        conf_object_t* trigger_obj, 
                        integer_t interrupt_number){
  conf_object_t* cpu = SIM_current_processor();
  int cycle_count = SIM_cycle_count(cpu);
  int current_processor_number = SIM_get_current_proc_no();
  CHECK_SIM_EXCEPTION();
  g_stats.incrementInterrupts(current_processor_number);
  LOG_EVENT(DEBUG_INTERRUPT, Box<TransOp>(), "Interrupt " << interrupt_number << " in cycle " << cycle_count);
  g_processors_vec[current_processor_number]->setInterruptOccurred(true);
}

Processor::Processor(int processor_number)
  : Context(), m_physical_file(), m_front_end_map(m_physical_file),
    m_retire_map(m_physical_file), fetch_pipe(), core_pipe(), retire_pipe(),
    m_predictors(), record_handler(0), record_factory(0) {
  // m_buf_size must be a power of 2 (otherwise mappedIndex() will not work properly)
  m_buf_size = 1 << 7; /* 128 uop instruction window */
  m_inst_buffer = new DynamicInst[m_buf_size];

  m_processor_number = processor_number;
  m_cpu = SIM_get_processor(m_processor_number);
  CHECK_SIM_EXCEPTION();

  attr_value_t attr = SIM_get_attribute(m_cpu, "architecture");
  const string arch = attr.u.string;
  if (arch == "x86-64") {
    m_is64bit = true;
  }
  else {
    assert(arch == "x86");
    m_is64bit = false;
  }

  if (m_processor_number == 0) {
    // initialize mapping of ptlsim register ids to simics register numbers
    if (m_is64bit) {
      g_simics_reg_id[REG_flags] = SIM_get_register_number(m_cpu, "rflags");
      g_simics_reg_id[REG_rip] = SIM_get_register_number(m_cpu, "rip");
      g_simics_reg_id[REG_rax] = SIM_get_register_number(m_cpu, "rax");
      g_simics_reg_id[REG_rbx] = SIM_get_register_number(m_cpu, "rbx");
      g_simics_reg_id[REG_rcx] = SIM_get_register_number(m_cpu, "rcx");
      g_simics_reg_id[REG_rdx] = SIM_get_register_number(m_cpu, "rdx");
      g_simics_reg_id[REG_rsp] = SIM_get_register_number(m_cpu, "rsp");
      g_simics_reg_id[REG_rbp] = SIM_get_register_number(m_cpu, "rbp");
      g_simics_reg_id[REG_rsi] = SIM_get_register_number(m_cpu, "rsi");
      g_simics_reg_id[REG_rdi] = SIM_get_register_number(m_cpu, "rdi");
      g_simics_reg_id[REG_r8 ] = SIM_get_register_number(m_cpu, "r8" );
      g_simics_reg_id[REG_r9 ] = SIM_get_register_number(m_cpu, "r9" );
      g_simics_reg_id[REG_r10] = SIM_get_register_number(m_cpu, "r10");
      g_simics_reg_id[REG_r11] = SIM_get_register_number(m_cpu, "r11");
      g_simics_reg_id[REG_r12] = SIM_get_register_number(m_cpu, "r12");
      g_simics_reg_id[REG_r13] = SIM_get_register_number(m_cpu, "r13");
      g_simics_reg_id[REG_r14] = SIM_get_register_number(m_cpu, "r14");
      g_simics_reg_id[REG_r15] = SIM_get_register_number(m_cpu, "r15");
    } else { /* 32-bit */
      g_simics_reg_id[REG_flags] = SIM_get_register_number(m_cpu, "eflags");
      g_simics_reg_id[REG_rip] = SIM_get_register_number(m_cpu, "eip");
      g_simics_reg_id[REG_rax] = SIM_get_register_number(m_cpu, "eax");
      g_simics_reg_id[REG_rbx] = SIM_get_register_number(m_cpu, "ebx");
      g_simics_reg_id[REG_rcx] = SIM_get_register_number(m_cpu, "ecx");
      g_simics_reg_id[REG_rdx] = SIM_get_register_number(m_cpu, "edx");
      g_simics_reg_id[REG_rsp] = SIM_get_register_number(m_cpu, "esp");
      g_simics_reg_id[REG_rbp] = SIM_get_register_number(m_cpu, "ebp");
      g_simics_reg_id[REG_rsi] = SIM_get_register_number(m_cpu, "esi");
      g_simics_reg_id[REG_rdi] = SIM_get_register_number(m_cpu, "edi");
    }
    CHECK_SIM_EXCEPTION();
  }

  m_decoder = new TraceDecoder(*this, readSimicsRegister(REG_rip));

  if (g_params.getRuby()) {
    m_mem_interface = new RubyMemoryInterface(this);
  } else {
    m_mem_interface = new SimpleMemoryInterface(this);
  }

  if (g_params.getInitUsingWarmup()) {
    printf("Init using warmup config\n");
    warmup();
  } else {
    dataCollect();
  }
  sim_cycle = 0;
  m_events_queue.addCycleable(&fetch_pipe);
  m_events_queue.addCycleable(&core_pipe);
  m_events_queue.addCycleable(&retire_pipe);

  QPointer fhead = fetch_pipe.readHead();
  fetch_pipe.configure(g_fpipe_depth, fhead);

  QPointer chead = core_pipe.readHead();
  core_pipe.configure(g_cpipe_depth, chead);

  QPointer rhead = retire_pipe.readHead();
  retire_pipe.configure(g_rpipe_depth, rhead); // fixme: is this still used?

  m_q_head = m_q_tail = 0;
  m_q_rename  = 0;
  m_q_execute = 0;
  m_q_retire  = 0;
  m_q_oldest_bad  = (QPointer)-1;

  m_interrupt_occurred  = false;
  m_exception_occurred  = false;
  m_crack_unaligned_memops = false;
  m_committing = false;
  m_reset_while_committing = false;

  m_scheduler = new OutorderScheduler(this, m_mem_interface);
}

void Processor::init(void){
  if (g_params.getPyriteIsControlling()){
    
    assert(SIM_initial_configuration_ok());
    initializeUopBuf();
    initializeProcessorState();
    initializeRamRanges();
    
    if (m_processor_number == 0) {
      /* register callbacks */
      SIM_hap_add_callback("Core_Asynchronous_Trap",
                           (obj_hap_func_t) interrupt_callback, 
                           NULL);
      CHECK_SIM_EXCEPTION();
      SIM_hap_add_callback("Core_Exception",
                           (obj_hap_func_t) exception_callback, 
                           NULL);
      CHECK_SIM_EXCEPTION();
    }

    if (SIM_number_processors() != 1) { 
      SIM_disable_processor(m_cpu);
      CHECK_SIM_EXCEPTION();
    }
  }
}

bool Processor::initializeRamRanges() {
  std::map<conf_object_t *, RangeType> ram_object_ranges;

  attr_value_t mem_space_attr = SIM_get_attribute(m_cpu, "physical_memory");
  assert(mem_space_attr.kind == Sim_Val_Object);
  conf_object_t *mem_space = mem_space_attr.u.object;

  while (mem_space) {
    attr_value_t mem_map_attr = SIM_get_attribute(mem_space, "map");
    assert(mem_map_attr.kind == Sim_Val_List);

    for (integer_t i = 0; i < mem_map_attr.u.list.size; ++i) {
      attr_value_t map_entry_attr = mem_map_attr.u.list.vector[i];
      assert(map_entry_attr.kind == Sim_Val_List);
      assert(map_entry_attr.u.list.vector[0].kind == Sim_Val_Integer);
      assert(map_entry_attr.u.list.vector[1].kind == Sim_Val_Object);
      assert(map_entry_attr.u.list.vector[4].kind == Sim_Val_Integer);
      conf_object_t *map_obj = map_entry_attr.u.list.vector[1].u.object;
      W64 base = map_entry_attr.u.list.vector[0].u.integer;
      W64 length = map_entry_attr.u.list.vector[4].u.integer;

      if (strcmp(SIM_get_class_name(map_obj->class_data), "ram") == 0) {
        if (ram_object_ranges.find(map_obj) == ram_object_ranges.end()) {
          ram_object_ranges[map_obj] = std::make_pair(base, base + length - 1);
        } else {
          if (base < ram_object_ranges[map_obj].first) {
            ram_object_ranges[map_obj].first = base;
          }
          if ((base + length - 1) >ram_object_ranges[map_obj].second) {
            ram_object_ranges[map_obj].second = base + length - 1;
          }
        }
      }
    }

    SIM_free_attribute(mem_map_attr);

    attr_value_t default_attr = SIM_get_attribute(mem_space, "default_target");
    if (default_attr.kind == Sim_Val_List) {
      assert(default_attr.u.list.vector[0].kind == Sim_Val_Object);
      mem_space = default_attr.u.list.vector[0].u.object;
      SIM_free_attribute(default_attr);
    } else {
      mem_space = 0;
    }
  }

  for (std::map<conf_object_t *, RangeType>::iterator i = ram_object_ranges.begin();
       i != ram_object_ranges.end(); ++i) {
    m_ram_ranges.insertAtBottom(i->second);
  }
}

bool Processor::validRamAddress(W64 physAddr) {
  for (int i = 0; i < m_ram_ranges.size(); ++i) {
    if ((physAddr >= m_ram_ranges[i].first) && (physAddr <= m_ram_ranges[i].second)) {
      return true;
    }
  }
  return false;
}

bool Processor::addressHasTranslation(W64 virtAddr, data_or_instr_t dataOrInstr){
  SIM_logical_to_physical(m_cpu, dataOrInstr, virtAddr);
  if (SIM_clear_exception() != SimExc_No_Exception){
    return false;
  }
  return true;
}

bool Processor::translateAddress(W64 virtAddr, W64 &physAddr, data_or_instr_t dataOrInstr) {
  if (!addressHasTranslation(virtAddr, dataOrInstr)) {
    return false;
  }
  physAddr = SIM_logical_to_physical(m_cpu, dataOrInstr, virtAddr);
  return validRamAddress(physAddr);
}

unsigned long long Processor::readFromSimicsMemory(W64 phys_addr,
                                                   W64 num_bytes){
  unsigned long long data = SIM_read_phys_memory(m_cpu, phys_addr, num_bytes);
  CHECK_SIM_EXCEPTION();
  return data;
}

bool Processor::procInKernelMode(){
  int priv_level = SIM_processor_privilege_level(m_cpu);
  CHECK_SIM_EXCEPTION();
  bool kernel_mode = (priv_level == 0);
  return kernel_mode;
}

W64 Processor::readSimicsRegister(int ptl_reg_id){
  // get simics register number
  int reg_number = g_simics_reg_id[ptl_reg_id];

  unsigned long long value = SIM_read_register(m_cpu, reg_number);
  CHECK_SIM_EXCEPTION();
  return value;
}

W64 Processor::readSimicsRegisterByName(const char *reg_name){
  // get simics register number
  int reg_number = SIM_get_register_number(m_cpu, reg_name);
  CHECK_SIM_EXCEPTION();

  unsigned long long value = SIM_read_register(m_cpu, reg_number);
  CHECK_SIM_EXCEPTION();
  return value;
}

void Processor::stepSimicsCycles(int num_cycles){
  assert(SIM_processor_enabled(m_cpu));
  CHECK_SIM_EXCEPTION();
  g_stats.setTotalX86Instructions(m_processor_number, g_stats.getTotalX86Instructions(m_processor_number) + 
                                  num_cycles);
  SIM_break_cycle(m_cpu, num_cycles);
  SIM_continue(0);
  assert(SIM_current_processor() == m_cpu);
}

/*********** Utility functions for our state ***********/

void Processor::resetUopBuf(void){
  //  printf("reseting pipeline (cycle: %d)\n", g_cycles);

  // m_q_tail can be zero, so specify bounds carefully
  for (QPointer q = m_q_head; q > m_q_tail; --q) {
    getDynamicInst(q - 1)->squash();
    getDynamicInst(q - 1)->reset();
  }

  m_q_head =    m_q_tail;
  m_q_rename =  m_q_tail;
  m_q_execute = m_q_tail;
  m_q_retire =  m_q_tail;
  m_q_oldest_bad  = (QPointer)-1;

  if (record_factory) { record_factory->AddEventForNextRecord(inst_record_t::PIPELINE_RESET); }
  if (m_mem_interface != NULL) {
    m_mem_interface->releaseAllWaiters(); 
  }

  if (m_committing) {
    m_reset_while_committing = true;
  }
}

void Processor::initializeUopBuf(){
  resetUopBuf();
}

/*********** processor core functions *************/
bool Processor::validateRegister(int ptl_reg_id){
  unsigned long long our_value = 0;
  if(ptl_reg_id == REG_rip) {
    our_value = m_retire_rip;
  } else {
    our_value = m_physical_file.getValue(m_retire_map.getMapping(ptl_reg_id));
  }
  unsigned long long simics_value = readSimicsRegister(ptl_reg_id);
  if (our_value != simics_value){
    LOG_EVENT(DEBUG_VALIDATION, Box<TransOp>(),
              "Values disagree for register " << ptl_reg_id << hex
              << ": our value: 0x" << our_value << ", simics value: 0x"
              << simics_value << dec);
    return false;
  }
  return true;
}

bool Processor::validateFlags(W64 flagmask){
  unsigned long long our_of = m_physical_file.getFlags(m_retire_map.getMapping(REG_of));
  unsigned long long our_cf = m_physical_file.getFlags(m_retire_map.getMapping(REG_cf));
  unsigned long long our_zapsf = m_physical_file.getFlags(m_retire_map.getMapping(REG_zf));
  unsigned long long our_value = (our_of & FLAG_OF) | (our_cf & FLAG_CF) | (our_zapsf & FLAG_ZAPS);
  unsigned long long simics_value = readSimicsRegister(REG_flags);
  bool cf_valid = (((our_value & FLAG_CF) == (simics_value & FLAG_CF)) ||
                   (!(flagmask & FLAG_CF)));
  bool zf_valid = (((our_value & FLAG_ZF) == (simics_value & FLAG_ZF)) ||
                   (!(flagmask & FLAG_ZF)));
  bool pf_valid = (((our_value & FLAG_PF) == (simics_value & FLAG_PF)) ||
                   (!(flagmask & FLAG_PF)));
  bool sf_valid = (((our_value & FLAG_SF) == (simics_value & FLAG_SF)) ||
                   (!(flagmask & FLAG_SF)));
  bool of_valid = (((our_value & FLAG_OF) == (simics_value & FLAG_OF)) ||
                   (!(flagmask & FLAG_OF)));
  bool ret = cf_valid && zf_valid && pf_valid && sf_valid && of_valid;
  if (!ret){
    LOG_EVENT(DEBUG_VALIDATION, Box<TransOp>(),
              "Values disagree for register eflags: " << hex
              << " our value: 0x" << our_value << ", simics value: 0x"
              << simics_value << dec);
  }
  return ret;
}

bool Processor::validateArchitecturalRegisters(W64 flagmask){
  // architectural registers only
  bool eip_valid = validateRegister(REG_rip);
  bool eax_valid = validateRegister(REG_rax);
  bool ebx_valid = validateRegister(REG_rbx);
  bool ecx_valid = validateRegister(REG_rcx);
  bool edx_valid = validateRegister(REG_rdx);
  bool esp_valid = validateRegister(REG_rsp);
  bool ebp_valid = validateRegister(REG_rbp);
  bool esi_valid = validateRegister(REG_rsi);
  bool edi_valid = validateRegister(REG_rdi);
  bool eflags_valid = validateFlags(flagmask);

  bool ret = eip_valid && eax_valid && ebx_valid && ecx_valid && edx_valid && 
    esp_valid && ebp_valid && esi_valid && edi_valid && eflags_valid;

  if (m_is64bit) {
    bool r8_valid = validateRegister(REG_r8);
    bool r9_valid = validateRegister(REG_r9);
    bool r10_valid = validateRegister(REG_r10);
    bool r11_valid = validateRegister(REG_r11);
    bool r12_valid = validateRegister(REG_r12);
    bool r13_valid = validateRegister(REG_r13);
    bool r14_valid = validateRegister(REG_r14);
    bool r15_valid = validateRegister(REG_r15);
    ret = ret && r8_valid && r9_valid && r10_valid && r11_valid && r12_valid &&
      r13_valid && r14_valid && r15_valid;
  }

  // TODO: add xmm registers
  // TODO: add fp registers

  return ret;
}

bool Processor::validateStore(const DoubleWord *store_operand){
  W64 simics_value = readFromSimicsMemory(store_operand->addr(), 8);
  bool valid = store_operand->compareValue(simics_value);

  if (!valid) {
    LOG_EVENT(DEBUG_VALIDATION, Box<TransOp>(), 
              "Store values didn't match for addr 0x" << hex << store_operand->addr()
              << " our value: 0x" << store_operand->value()
              << ", simics value: 0x" << simics_value
              << ", set bytes: 0x" << (int)store_operand->bytemask() << dec);
  }
  return valid;
}

bool Processor::validatePyriteState(W64 flagmask){
  bool ret = true;
  ret = ret && validateArchitecturalRegisters(flagmask);
  return ret;
}
   
void Processor::resetPyriteRegister(LogicalName ptl_reg_id){
  assert(ptl_reg_id != REG_rip);
  PhysName preg = m_front_end_map.getNewMapping(ptl_reg_id);
  if (ptl_reg_id == REG_flags) {
    m_physical_file.setValue(preg, 0);
    m_physical_file.setFlags(preg, readSimicsRegister(ptl_reg_id));
  } else {
    m_physical_file.setValue(preg, readSimicsRegister(ptl_reg_id));
    m_physical_file.setFlags(preg, 0);
  }
}

void Processor::resetPyriteArchitecturalRegisters() {
  m_events_queue.clear();
  m_front_end_map.reset();
  m_retire_map.reset();
  m_physical_file.reset();

  // setup a mapping for REG_zero
  PhysName zero_preg = m_front_end_map.getNewMapping(REG_zero);
  m_physical_file.setValue(zero_preg, 0);

  m_fetch_rip = readSimicsRegister(REG_rip);
  m_retire_rip = readSimicsRegister(REG_rip);
  resetPyriteRegister(REG_rax);
  resetPyriteRegister(REG_rbx);
  resetPyriteRegister(REG_rcx);
  resetPyriteRegister(REG_rdx);
  resetPyriteRegister(REG_rsp);
  resetPyriteRegister(REG_rbp);
  resetPyriteRegister(REG_rsi);
  resetPyriteRegister(REG_rdi);
  resetPyriteRegister(REG_flags);

  if (m_is64bit) {
    resetPyriteRegister(REG_r8);
    resetPyriteRegister(REG_r9);
    resetPyriteRegister(REG_r10);
    resetPyriteRegister(REG_r11);
    resetPyriteRegister(REG_r12);
    resetPyriteRegister(REG_r13);
    resetPyriteRegister(REG_r14);
    resetPyriteRegister(REG_r15);
  }

  // we rename subsets of the flags register independently, but
  // they initially map to the same physical register
  PhysName flags_preg = m_front_end_map.getMapping(REG_flags);
  m_front_end_map.setMapping(REG_of, flags_preg);
  m_front_end_map.setMapping(REG_cf, flags_preg);
  // this is redundant, because REG_zf and REG_flags are treated equivalently
  m_front_end_map.setMapping(REG_zf, flags_preg);

  // xmm registers
  attr_value_t xmm_regs = SIM_get_attribute(m_cpu, "xmm");
  assert(xmm_regs.kind == Sim_Val_List);
  for (int i = 0; i < xmm_regs.u.list.size; i++){
    attr_value_t xmm_i = xmm_regs.u.list.vector[i];
    assert(xmm_i.kind == Sim_Val_List);
    assert(xmm_i.u.list.size == 2);
    attr_value_t xmm_i_low = xmm_i.u.list.vector[0];
    attr_value_t xmm_i_high = xmm_i.u.list.vector[1];
    assert(xmm_i_low.kind == Sim_Val_Integer);
    assert(xmm_i_high.kind == Sim_Val_Integer);
    W64 xmm_i_low_value = xmm_i_low.u.integer;
    W64 xmm_i_high_value = xmm_i_high.u.integer;
    // TODO: fix the manipulation below to make it more clear/less fragile
    // xmm registers start at 16, and there is a consecutive low and high for
    // each
    PhysName xmm_preg_low = m_front_end_map.getNewMapping(16 + (2 * i));
    PhysName xmm_preg_high = m_front_end_map.getNewMapping(16 + (2 * i) + 1);
    m_physical_file.setValue(xmm_preg_low, xmm_i_low_value);
    m_physical_file.setValue(xmm_preg_high, xmm_i_high_value);
//    writePyriteRegister(16 + (2 * i), xmm_i_low_value);
//    writePyriteRegister(16 + (2 * i) + 1, xmm_i_high_value);
    //cout << "xmm " << i << ": " << "[" << xmm_i.u.list.vector[0].u.integer << ", " << xmm_i.u.list.vector[1].u.integer << endl;
  }
  SIM_free_attribute(xmm_regs);

  // map temporary registers
  PhysName preg = 0;
  preg = m_front_end_map.getNewMapping(REG_temp0);
  m_physical_file.setValue(preg, 0);
  preg = m_front_end_map.getNewMapping(REG_temp1);
  m_physical_file.setValue(preg, 0);
  preg = m_front_end_map.getNewMapping(REG_temp2);
  m_physical_file.setValue(preg, 0);
  preg = m_front_end_map.getNewMapping(REG_temp3);
  m_physical_file.setValue(preg, 0);
  preg = m_front_end_map.getNewMapping(REG_temp4);
  m_physical_file.setValue(preg, 0);
  preg = m_front_end_map.getNewMapping(REG_temp5);
  m_physical_file.setValue(preg, 0);
  preg = m_front_end_map.getNewMapping(REG_temp6);
  m_physical_file.setValue(preg, 0);
  preg = m_front_end_map.getNewMapping(REG_temp7);
  m_physical_file.setValue(preg, 0);
  preg = m_front_end_map.getNewMapping(REG_temp8);
  m_physical_file.setValue(preg, 0);
  preg = m_front_end_map.getNewMapping(REG_temp9);
  m_physical_file.setValue(preg, 0);
  preg = m_front_end_map.getNewMapping(REG_temp10);
  m_physical_file.setValue(preg, 0);

  // map internal registers
  preg = m_front_end_map.getNewMapping(REG_ctx);
  Context *ctxt = this;
  m_physical_file.setValue(preg, (Waddr)ctxt);

  // setup x87 state
  preg = m_front_end_map.getNewMapping(REG_fpstack);
  m_physical_file.setValue(preg, (Waddr)ctxt->fpstack);
  preg = m_front_end_map.getNewMapping(REG_fpsw);
  attr_value_t fpu_status = SIM_get_attribute(m_cpu, "fpu_status");
  assert( fpu_status.kind == Sim_Val_Integer );
  m_physical_file.setValue(preg, fpu_status.u.integer);
  preg = m_front_end_map.getNewMapping(REG_fptos);
  m_physical_file.setValue(preg, (fpu_status.u.integer >> 8) & 0x38);
  preg = m_front_end_map.getNewMapping(REG_fptags);
  attr_value_t fpu_tag = SIM_get_attribute(m_cpu, "fpu_tag");
  assert( fpu_tag.kind == Sim_Val_Integer );
  m_physical_file.setValue(preg, fpu_tag.u.integer);
  attr_value_t fpu_control = SIM_get_attribute(m_cpu, "fpu_control");
  assert( fpu_control.kind == Sim_Val_Integer );
  fpcw = fpu_status.u.integer;
  SIM_free_attribute(fpu_status);
  SIM_free_attribute(fpu_tag);
  SIM_free_attribute(fpu_control);

  // setup floating point registers
  attr_value_t fpu = SIM_get_attribute(m_cpu, "fpu_regs");
  assert(fpu.kind == Sim_Val_List && fpu.u.list.size == 8);
  for (int i = 0 ; i < 8 ; ++ i) {
    assert(fpu.u.list.vector[i].u.list.vector[0].kind == Sim_Val_Integer);
    assert(fpu.u.list.vector[i].kind == Sim_Val_List);
    assert(fpu.u.list.vector[i].u.list.size == 11);

    // Simics represents each fp register with 11 bytes.
    // The first byte keeps track of if the register is empty (1) or not (0).
    // We only model 64-bit regs, so only grab the lower 8 bytes.

    fpstack[i] = 0;
    for (int j = 8 ; j > 0 ; -- j) {
      assert(fpu.u.list.vector[i].u.list.vector[j].kind == Sim_Val_Integer);
      unsigned fbyte = fpu.u.list.vector[i].u.list.vector[j].u.integer;
      assert(fbyte < 256);
      fpstack[i] = (fpstack[i] << 8) | fbyte;
    }
  }
  SIM_free_attribute(fpu);

  //setup the segment descriptors
  seg[SEGID_ES].selector = readSimicsRegisterByName("es");
  seg[SEGID_ES].base = readSimicsRegisterByName("es_base");
  seg[SEGID_CS].selector = readSimicsRegisterByName("cs");
  seg[SEGID_CS].base = readSimicsRegisterByName("cs_base");
  seg[SEGID_SS].selector = readSimicsRegisterByName("ss");
  seg[SEGID_SS].base = readSimicsRegisterByName("ss_base");
  seg[SEGID_DS].selector = readSimicsRegisterByName("ds");
  seg[SEGID_DS].base = readSimicsRegisterByName("ds_base");
  seg[SEGID_FS].selector = readSimicsRegisterByName("fs");
  seg[SEGID_FS].base = readSimicsRegisterByName("fs_base");
  seg[SEGID_GS].selector = readSimicsRegisterByName("gs");
  seg[SEGID_GS].base = readSimicsRegisterByName("gs_base");

  //setup the internal eflags
  internal_eflags = readSimicsRegister(REG_flags) & ~(FLAG_OF | FLAG_CF | FLAG_ZAPS);
  kernel_mode = procInKernelMode();
  
  m_retire_map = m_front_end_map;
}

void Processor::clearTransientState(void){
  m_interrupt_occurred = false;
  m_exception_occurred = false;
}

void Processor::resetPyriteState(void){
  resetPyriteArchitecturalRegisters();
  m_predictors.squash(m_q_tail);
  m_lsq.reset();
  g_stats.incrementResets(m_processor_number);
  clearTransientState();
  fetch_pipe.configure(g_fpipe_depth, m_q_tail);
  core_pipe.configure(g_cpipe_depth, m_q_tail);
  retire_pipe.configure(g_rpipe_depth, m_q_tail);
  sim_cycle = getCurrentCycle();
  m_crack_unaligned_memops = false;
}

void Processor::initializeProcessorState(void){
  resetPyriteArchitecturalRegisters();
}

bool Processor::fetchMoreBytes(Waddr &fetch_address, W8 *fetch_buffer, unsigned &num_bytes) {
  W64 phys_addr;
  // printf("A: %d(%x) NB: %d\n", (int)fetch_address, (int)fetch_address, num_bytes);

  if (!translateAddress(fetch_address, phys_addr, Sim_DI_Instruction)) {
    logNoTranslationForInstructionEvent();
    return false;
  }

  unsigned avail = PAGE_SIZE - PAGE_OFFSET(fetch_address);
  size_t bytes_grabbed = (avail < 8) ? avail : 8;
  W64 insn_contents = SIM_read_phys_memory(m_cpu, phys_addr, bytes_grabbed);
  if (SIM_clear_exception() != SimExc_No_Exception) {
    logNoTranslationForInstructionEvent();
    return false;
  }
  *(W64 *)&fetch_buffer[num_bytes] = insn_contents;
  num_bytes += bytes_grabbed;
  fetch_address += bytes_grabbed;
  return true;
}

bool Processor::decodeCurrentX86Instruction(bool &taken_branch){
  unsigned long long insn_address = m_fetch_rip;
  W8 fetch_buffer[32];
  unsigned num_bytes = 0;
  Waddr fetch_address = insn_address;
  unsigned instruction_size = 0;
  TransOp trans_op_buf[MAX_TRANSOPS_PER_USER_INSN];

  while(1) {
    if (!fetchMoreBytes(fetch_address, fetch_buffer, num_bytes)) {
      return false;
    }

    try{
      // prime decoder
      m_decoder->reset();
      m_decoder->use64 = m_is64bit;
      m_decoder->rip = insn_address;
      m_decoder->ripstart = insn_address;
      m_decoder->split_basic_block_at_locks_and_fences = false;
      m_decoder->split_invalid_basic_blocks = false;
      m_decoder->split_unaligned_memops = m_crack_unaligned_memops;
  
      instruction_size = m_decoder->translate(fetch_buffer, num_bytes, trans_op_buf);
      break;
    } catch (NotEnoughBytesException& e) {
      // still not enough bytes--back to top of loop
      assert(num_bytes <= 24);
      continue;
    } catch (InvalidOpcodeException& e) {
      logInvalidOpcodeEvent(e);
      return false;
    } catch (UnimplementedOpcodeException& e){
      logUnimplementedOpcodeEvent(e, m_fetch_rip);
      return false;
    }
  }

  // figure out how many uops the x86 op decoded into
  unsigned num_uops = m_decoder->transbufcount;
  assert(num_uops <= MAX_TRANSOPS_PER_USER_INSN);

  // lookup implementation functions for each uop
  for (int i = 0; i < num_uops; i++) {
    TransOp &curr = trans_op_buf[i];
    assert(!isbranch(curr.opcode) || (i == (num_uops - 1)));
    int sfra = 0; /** @todo: sfra == store forwarding register address? */
    bool except = 0; /** @todo except == an exception occurred? */    

    DynamicInst *dyn_curr = getDynamicInst(m_q_head + i);
    dyn_curr->init(this, curr, insn_address,
                   get_synthcode_for_uop(curr.opcode, curr.size, curr.setflags,
                                         curr.cond, curr.extshift, sfra,
                                         curr.cachelevel, except, curr.internal),
                   (m_q_head + i),
                   isload(curr.opcode) ? g_load_latency : 1);
    if (record_factory) { 
      real_inst_record_t *irecord = record_factory->Allocate();
      irecord->SetUopNum(i);
      dyn_curr->setRecord(irecord); 
    }
    dyn_curr->setStage(FETCH_STAGE);
    if (m_crack_unaligned_memops &&
        ((isload(curr.opcode)  && (curr.ra != REG_ctx)) ||
         isstore(curr.opcode))) {
      dyn_curr->recordEvent(inst_record_t::UNALIGNED);
    }

    assert(dyn_curr->getMemOperand()->isClear() && dyn_curr->detached());
  }

  getDynamicInst(m_q_head)->setStartsX86Op();
  getDynamicInst(m_q_head + num_uops - 1)->setEndsX86Op();

  logInstructionDecodeEvent(insn_address, instruction_size, m_q_head, num_uops);

  DynamicInst *last_inst = getDynamicInst(m_q_head + num_uops - 1);
  assert(last_inst->getQPointer() == (m_q_head + num_uops - 1));
  if(isbranch(last_inst->getOpcode())) {
    assert((isuncondbranch(last_inst->getOpcode()) &&
            (last_inst->getTransOp()->extshift != BRANCH_HINT_PUSH_RAS)) ||
           (isindirectbranch(last_inst->getOpcode())) ||
           ((insn_address + instruction_size) == last_inst->getTransOp()->ripseq));
    m_fetch_rip = m_predictors.predict(last_inst, last_inst->getTransOp()->ripseq);
    if (m_fetch_rip != (insn_address + instruction_size)) {
      taken_branch = true;
    }
    // printf("uop: %d, predicted 0x%x, sequential 0x%x\n", (int)last_inst->getQPointer(),
    //        (int)m_fetch_rip, (int)(insn_address + instruction_size));
  } else {
    m_fetch_rip = insn_address + instruction_size;
    // printf("uop: %d, sequential 0x%x\n", (int)last_inst->getQPointer(),
    //        (int)(insn_address + instruction_size));
  }
  m_q_head += num_uops;
  m_crack_unaligned_memops = false;

  return true;
}

void Processor::logInvalidOpcodeEvent(InvalidOpcodeException& e){
  LOG_EVENT(DEBUG_INVALID_OPCODE, Box<TransOp>(), e.what());
  g_stats.incrementInvalidX86Instructions(m_processor_number);
}

void Processor::logUnimplementedOpcodeEvent(UnimplementedOpcodeException& e, W64 rip){
  LOG_EVENT(DEBUG_UNIMPLEMENTED_OPCODE, Box<TransOp>(), e.what());
  g_stats.incrementUnimplementedX86Instructions(m_processor_number);
  g_stats.incrementUnimplementedPCs(m_processor_number, rip);
}

void Processor::logNoTranslationForInstructionEvent(void){
  LOG_EVENT(DEBUG_MEMORY, Box<TransOp>(), "Instruction not in physical memory");
  g_stats.incrementNoTranslationForInsnX86Instructions(m_processor_number);
}

void Processor::logStatsForUop(TransOp &uop){
  g_stats.incrementTotalUOps(m_processor_number);
  if (isload(uop.opcode)){
    g_stats.incrementLoads(m_processor_number);
  } else  if (isfence(uop.opcode)){
    g_stats.incrementFences(m_processor_number);
  } else if (isstore(uop.opcode)){
    g_stats.incrementStores(m_processor_number);
  } else if (isbranch(uop.opcode)){
    g_stats.incrementBranches(m_processor_number);
  } 
}

void Processor::logInstructionDecodeEvent(W64 insn_address, unsigned instruction_size,
                                          QPointer q_ptr, unsigned num_uops) {
  if (strcmp(g_params.getDisasmFilePath().c_str(), "/dev/null") != 0) {
    // record the x86 instruction and its uops if we haven't seen it before
    if (!g_code_map.exist(insn_address)) { 
      W64 physAddr = SIM_logical_to_physical(m_cpu, Sim_DI_Instruction, insn_address);
      CHECK_SIM_EXCEPTION();
      tuple_int_string_t* disasm = SIM_disassemble(m_cpu, physAddr, 0/*phys addr*/);
      CHECK_SIM_EXCEPTION();
      
      stringstream sbuf;
      sbuf << hex  << insn_address << dec << ": " << disasm->string << "\n";
      for (int i = 0; i < num_uops; i++){
        sbuf << "   " << i << ": " << *getDynamicInst(q_ptr + i)->getTransOp() << "\n";
      }
      g_code_map.insert(insn_address, sbuf.str());
    }
  }

  LOG_EVENT(DEBUG_UOP, Box<TransOp>(), "Insn uops:");
  for (int i = 0; i < num_uops; i++){
    const TransOp *curr = getDynamicInst(q_ptr + i)->getTransOp();
    LOG_EVENT(DEBUG_UOP, Box<TransOp>(*curr), "");
  }
  
  if (IS_DEBUGGED(DEBUG_X86INSN)) {
    W64 physAddr = SIM_logical_to_physical(m_cpu, Sim_DI_Instruction, insn_address);
    CHECK_SIM_EXCEPTION();
    tuple_int_string_t* disasm = SIM_disassemble(m_cpu, physAddr, 0/*phys addr*/);
    CHECK_SIM_EXCEPTION();
    LOG_EVENT(DEBUG_X86INSN, Box<TransOp>(), "Decoded x86 insn: " << disasm->string);
    LOG_EVENT(DEBUG_X86INSN, Box<TransOp>(), "Instruction was " << instruction_size << " bytes long");
  }
}

void Processor::rename(void){
  QPointer q_rename_ready = fetch_pipe.readHead();

  // Don't try to rename past the end of the ROB
  q_rename_ready = std::min(q_rename_ready, m_q_tail + g_rob_size);

  for (int i = 0 ; i < g_rename_width ; ++i) {
    if (m_q_rename >= q_rename_ready) {
      break;
    }

    assert(m_q_rename < m_q_head);

    DynamicInst *dyn_curr = getDynamicInst(m_q_rename);

    LOG_EVENT(DEBUG_UOP, Box<TransOp>(*dyn_curr->getTransOp()), "Rename uop.");
	 
    if(!dyn_curr->uopIsLegal()) {
      setOldestBad(m_q_rename);
      break;
    }

    if (dyn_curr->getOpcode() == OP_nop) {
      dyn_curr->completeAtDecode();
      --i; 
    } else {
      dyn_curr->rename();
    }
    ++m_q_rename;
  }

  core_pipe.insertTail(m_q_rename);
}

void Processor::resolveBranch(QPointer q, Waddr actual_target) {
  DynamicInst *dyn_curr = getDynamicInst(q);
  m_predictors.resolve(dyn_curr, actual_target);

  g_stats.incrementTotalBranchPredictions(m_processor_number);
  if(dyn_curr->isMispredicted()) {
    g_stats.incrementTotalBranchMispredictions(m_processor_number);
    dyn_curr->recordEvent(inst_record_t::BRANCH_MISP);
  }

  if ((q + 1) < m_q_head) {  // if successor has already been fetched
    if(dyn_curr->isMispredicted()) {
      flushPipeline(q + 1);
      m_fetch_rip = actual_target;  // re-direct fetch to correct target
    } else {
      // correctly predicted branch
    }
  } else {
    m_fetch_rip = actual_target;  // successor hasn't been fetched, write to front end PC
    if (m_q_oldest_bad == m_q_head) {
      m_q_oldest_bad  = (QPointer)-1;  // this was marked "bad" because we didn't know what it was
    }
  }
}

// an OP_chk instruction exception
void Processor::checkException(QPointer q) {
  DynamicInst *dyn_curr = getDynamicInst(q);
  DynamicInst *dyn_last = dyn_curr;
  while (!dyn_last->endsX86Op()) {
    dyn_last =  getDynamicInst(dyn_last->getQPointer() + 1);
  }
  dyn_last->setEndsX86Op(false);
  assert(dyn_last->getQPointer() > q);

  // re-direct fetch to sequential x86 op
  m_fetch_rip = getDynamicInst(dyn_last->getQPointer())->getTransOp()->ripseq;

  // flush everything after this uop
  flushPipeline(q + 1);

  // fixup the x86 op meta data so that this op is the last member
  dyn_curr->setEndsX86Op();
  dyn_curr->setCheckException();

  dyn_curr->recordEvent(inst_record_t::BRANCH_MISP);
}

void Processor::alignmentException(QPointer q) {
  DynamicInst *dyn_unaligned = getDynamicInst(q);

  DynamicInst *dyn_first = dyn_unaligned;
  while (!dyn_first->startsX86Op()) {
    dyn_first =  getDynamicInst(dyn_first->getQPointer() - 1);
  }

  // flush this entire x86 op, and refetch it (but crack it next time)
  m_fetch_rip = dyn_first->getRIP();
  flushPipeline(dyn_first->getQPointer());
  m_crack_unaligned_memops = true;
}

void Processor::wakeup(DynamicInst *d) {
  if (m_scheduler != NULL) { 
    m_scheduler->wakeup(d);
  } 
}

void Processor::execute(void){
  for (int i = 0 ; i < g_execute_width ; ++ i) {
    QPointer q_execute_ready = core_pipe.readHead();
    if (m_q_execute >= q_execute_ready) {
      break;
    }

    assert(m_q_execute < m_q_head);
    DynamicInst *dyn_curr = getDynamicInst(m_q_execute);
    if (!dyn_curr->isRetireReady()) {
      dyn_curr->queue();
      dyn_curr->schedule();
    } else {
      // don't count instructions that completed at decode towards execute
      --i;
    }
    ++m_q_execute;
  }
  retire_pipe.insertTail(m_q_execute);

  // Actually select instructions for execution
  m_scheduler->select();
}

void Processor::flushPipeline(QPointer q_first_bad) {
  fetch_pipe.squash(q_first_bad);
  m_predictors.squash(q_first_bad);
  core_pipe.squash(q_first_bad);
  retire_pipe.squash(q_first_bad);
  m_q_execute = std::min(m_q_execute, q_first_bad);
  m_q_retire = std::min(m_q_retire, q_first_bad);
  if ((m_q_oldest_bad != (QPointer)-1) && (m_q_oldest_bad >= q_first_bad)) {
    m_q_oldest_bad  = (QPointer)-1;
  }

  // printf("flushPipeline q_first_bad = %d\n", (unsigned)q_first_bad);
  for (QPointer q = m_q_head - 1 ; q >= q_first_bad ; -- q) {
    DynamicInst *dyn_curr = getDynamicInst(q);
    dyn_curr->squash();
  }
  m_q_rename = q_first_bad;
  m_q_head = q_first_bad;
}

void Processor::replay(QPointer q_first_replayed) {
  core_pipe.squash(q_first_replayed);
  retire_pipe.squash(q_first_replayed);
  m_q_execute = std::min(m_q_execute, q_first_replayed);
  m_q_retire = std::min(m_q_retire, q_first_replayed);
  if (m_q_oldest_bad >= q_first_replayed) {
    m_q_oldest_bad  = (QPointer)-1;
  }

  for (QPointer q = m_q_rename - 1 ; q >= q_first_replayed ; --q) {
    DynamicInst *dyn_curr = getDynamicInst(q);
    QPointer fetch_cycle = 0;
    W8 uop_num = 0;
    if (record_factory) {
      real_inst_record_t *record = dyn_curr->getRecord();
      fetch_cycle = record->Get(inst_record_t::FETCH_STAGE);
      uop_num = record->GetUopNum();
      record->Free();
    }
    dyn_curr->squash();
    dyn_curr->setStage(FETCH_STAGE);
    if (record_factory) {
      real_inst_record_t *irecord = record_factory->Allocate();
      irecord->SetUopNum(uop_num);
      irecord->Set(inst_record_t::FETCH_STAGE, fetch_cycle);
      dyn_curr->setRecord(irecord);
    }
  }
  m_q_rename = q_first_replayed;
}

void Processor::retire() {
  QPointer q_retire_ready = retire_pipe.readHead();
  for (int i = 0 ; i < g_retire_width ; ++ i) {
    if (m_q_oldest_bad == m_q_retire) { // for bad fetches/decodes/executions
      resetUopBuf();
      stepSimicsCycles(1);
      resetPyriteState();
      break;
    }
	 
    if (m_q_retire >= q_retire_ready) {
      break;
    }

    DynamicInst *dyn_curr = getDynamicInst(m_q_retire);
    if (!dyn_curr->isRetireReady()) {
      if (isstore(dyn_curr->getOpcode())) {
        if (!dyn_curr->detached() || !dyn_curr->demandStore()) {
          break;  // store missed
        }
      } else { // not a store
        break;
      }
      assert(dyn_curr->isRetireReady()); 
    }

    // Check for ordering and load violations on loads
    if (dyn_curr->hasLoadOrderingViolation()) {
      // loads with ordering violations replayed (and for simplicity replay
      // it and all subsequent uops).
      assert(isload(dyn_curr->getOpcode()));
      replay(dyn_curr->getQPointer());
      dyn_curr->recordEvent(inst_record_t::ORDERING_VIOLATION_L);
      break;
    } else if (dyn_curr->hasLoadConsistencyViolation()) {
      // loads with consistency violations can still be
      // committed (all subsequent memory ops must be replayed)
      assert(isload(dyn_curr->getOpcode()));
      replay(dyn_curr->getQPointer() + 1);
      dyn_curr->recordEvent(inst_record_t::COHERENCE_VIOLATION);
      q_retire_ready = retire_pipe.readHead();
    }

    if (ischeck(dyn_curr->getOpcode()) && !dyn_curr->hasCheckException() && dyn_curr->endsX86Op()) {
      // An excepting check uop has been replayed and no longer throws an exception, but still
      // (incorrectly) terminates it's x86 op. A pipeline flush is needed to remedy the situation.
      DynamicInst *dyn_first = dyn_curr;
      while (!dyn_first->startsX86Op()) {
        dyn_first =  getDynamicInst(dyn_first->getQPointer() - 1);
      }
      m_fetch_rip = dyn_first->getRIP();
      flushPipeline(dyn_first->getQPointer());
      break;
    }

    // commit
    LOG_EVENT(DEBUG_UOP, Box<TransOp>(*dyn_curr->getTransOp()), "Retire uop.");

    // remove committed branches from predictors "inflight" set
    if(isbranch(dyn_curr->getOpcode())) {
      m_predictors.commit(dyn_curr);
    }
    
    if (dyn_curr->endsX86Op()) {
      assert(getDynamicInst(m_q_tail)->startsX86Op());

      // printf("committing x86 op: %d [uops %d - %d]\n", (int)m_q_x86_tail, (int)curr_x86->m_q_first_uop, (int)curr_x86->m_q_last_uop);
		
      // compute the correct next_to_retire PC to validate against simics
      W64 next_to_retire_rip = m_fetch_rip;
      if (m_q_head > (m_q_retire + 1)) {
        next_to_retire_rip = getDynamicInst(m_q_retire + 1)->getRIP();
      }
      m_retire_rip = next_to_retire_rip;

      // Commit the uop and compute the flagmask. The flagmask is the union of the
      // flags modified by uops in the x86 op, excluding uops with nouserflags set.
      W64 flagmask = 0;
      for (QPointer q = m_q_tail; q <= dyn_curr->getQPointer(); ++ q) {
        DynamicInst *dyn_iter = getDynamicInst(q);
        dyn_iter->commit();
        if (!dyn_iter->getTransOp()->nouserflags) {
          flagmask |= setflags_to_x86_flags[dyn_iter->getTransOp()->setflags];
        }
      }

      m_committing = true;
      bool succeed = commitX86Instruction(dyn_curr->getRIP(), flagmask);
      m_committing = false;

      if (m_reset_while_committing) {
        m_reset_while_committing = false;
        succeed = false;
      }

      if (succeed) {
        for (QPointer q = m_q_tail; q <= dyn_curr->getQPointer(); ++ q) {
          DynamicInst *dyn_iter = getDynamicInst(q);
          if (isfence(dyn_iter->getOpcode())) {  // do nothing
          } else if (isstore(dyn_iter->getOpcode()) && !dyn_iter->isInternal()) {
            succeed &= validateStore(dyn_iter->getMemOperand());
          }
          if (dyn_iter->isLSQInserted()) {
            m_lsq.remove(dyn_iter);
          }
        }
      }
		
      if (!succeed) {
        logIncorrectExecutionEvent(dyn_curr->getRIP());
        resetUopBuf();
        resetPyriteState();
        break;
      } else {
        g_stats.incrementCorrectlyExecutedX86Instructions(m_processor_number);
        for (QPointer q = m_q_tail; q <= dyn_curr->getQPointer(); ++ q) {
          DynamicInst *dyn_iter = getDynamicInst(q);
          if (record_handler != NULL) {
            record_handler->Process(dyn_iter->getRecord(), q);
          }
        }
        m_q_tail = dyn_curr->getQPointer() + 1;
        sim_cycle = getCurrentCycle();
        clearTransientState();
      }
    }
    ++ m_q_retire;
  }
}

void Processor::logUnalignedDataMemoryOperation(DynamicInst &dyn_uop, W64 addr, bool is_load){
  if (is_load){
    LOG_EVENT(DEBUG_MEMORY, Box<TransOp>(*dyn_uop.getTransOp()), "Load was not aligned");
    g_stats.incrementUnalignedLoadX86Instructions(m_processor_number);
    g_stats.incrementUnalignedLoadPCs(m_processor_number, dyn_uop.getRIP());
    g_stats.incrementUnalignedLoadAddresses(m_processor_number, addr);
  } else{
    LOG_EVENT(DEBUG_MEMORY, Box<TransOp>(*dyn_uop.getTransOp()), "Store was not aligned");
    g_stats.incrementUnalignedStoreX86Instructions(m_processor_number);
    g_stats.incrementUnalignedStorePCs(m_processor_number, dyn_uop.getRIP());
    g_stats.incrementUnalignedStoreAddresses(m_processor_number, addr);
  }
}

void Processor::logNoTranslationDataMemoryOperation(DynamicInst &dyn_uop, W64 addr, bool is_load){
  if (is_load){
    LOG_EVENT(DEBUG_MEMORY, Box<TransOp>(*dyn_uop.getTransOp()), "Load had no translation");
    g_stats.incrementNoTranslationLoadPCs(m_processor_number, dyn_uop.getRIP());
    g_stats.incrementNoTranslationLoadX86Instructions(m_processor_number);
    g_stats.incrementNoTranslationLoadX86Instructions(m_processor_number);
  } else{
    LOG_EVENT(DEBUG_MEMORY, Box<TransOp>(*dyn_uop.getTransOp()), "Store had no translation");
    g_stats.incrementNoTranslationStoreAddresses(m_processor_number, addr);
    g_stats.incrementNoTranslationStorePCs(m_processor_number, dyn_uop.getRIP());
    g_stats.incrementNoTranslationStoreX86Instructions(m_processor_number);
  }
}

void Processor::logUopExecution(TransOp &uop, W64 ra, W64 rb, W64 rc,
                                W64 raflags, W64 rbflags, W64 rcflags){
  LOG_EVENT(DEBUG_UOP, Box<TransOp>(uop), 
            "Arguments:" << endl
            << "  ra:0x" << hex << ra << " raflags:0x" << raflags << dec << endl
            << "  rb:0x" << hex << rb << " rbflags:0x" << rbflags << dec << endl
            << "  rc:0x" << hex << rc << " rcflags:0x" << rcflags << dec);
}

void Processor::logUopExecutionResult(TransOp &uop, IssueState is, W64 flags){
  LOG_EVENT(DEBUG_UOP, Box<TransOp>(uop), 
            "Issue State:" << endl
            << "  rd:0x" << hex << is.reg.rddata << " addr:0x" << is.reg.addr
            << "  flags? " << !uop.nouserflags << " computed flags:0x" << flags << dec << endl
            << "  taken:0x" << hex << is.brreg.riptaken << " fallthru:0x" << is.brreg.ripseq << dec);
}

void Processor::logCycleStats(void){
  if (procInKernelMode()){
    g_stats.incrementKernelCycles(m_processor_number);
    return;
  }
}

void Processor::logIncorrectExecutionEvent(W64 prev_rip){
  if (!(m_interrupt_occurred || m_exception_occurred)){
    g_stats.incrementIncorrectlyExecutedX86Instructions(m_processor_number);
    g_stats.incrementIncorrectlyExecutedPCs(m_processor_number, prev_rip);
  }
}

real_inst_record_handler_t *
Processor::setRIRHandler(real_inst_record_handler_t *RIR_handler) {
  real_inst_record_handler_t *ret_val = record_handler;
  record_handler = RIR_handler;

  //! have a "record_factory" available whenever the "record_handler" is not NULL
  assert(ret_val || !record_factory);
  if (ret_val) {
    assert(record_factory);
    delete record_factory;
    record_factory = 0;
  }

  if (RIR_handler) {
    record_factory = new real_inst_record_factory_t; 
  }

  return ret_val;
}

bool 
Processor::commitX86Instruction(W64 insn_rip, W64 flagmask){
  stepSimicsCycles(1);
  bool valid_execution = validatePyriteState(flagmask);
    
  return valid_execution;
}

void Processor::stepCycle(void){

  // process everything that needs to be cycled once per cycle.
  m_events_queue.doCycle();

  if (getCurrentCycle() > (sim_cycle + 10000)) {
    printf("DEADLOCK: CPU %d sim_cycle: %d\n", m_processor_number, (int)sim_cycle);
  }
  
  logCycleStats();

  fetch(); // and decode

  rename();
 
  execute();

  retire();
}

void
Processor::fetch() {
  bool taken_branch = false;

  for (int i = 0 ; i < g_x86_fetch_width ; ++ i) {
    if (m_q_oldest_bad != (QPointer)-1) { // don't bother if pipeline is already doomed
      break;
    }
    if (m_q_head >= (m_q_rename + g_front_end_max)) { // front end already full
      break;
    }
    bool decoded_successfully = decodeCurrentX86Instruction(taken_branch);
    if (!decoded_successfully) {
      m_q_oldest_bad = std::min(m_q_oldest_bad, m_q_head);
      break;
    }
    if (taken_branch) { // stop fetching at the first taken branch
      break;
    }
  }

  fetch_pipe.insertTail(m_q_head);
}

void
Processor::reset() {
  m_events_queue.resetCurrentCycle();
  m_predictors.clearStats();
  resetUopBuf();
  resetPyriteState();
  if (m_mem_interface != NULL) {
    m_mem_interface->reset();
  }
}

void 
Processor::warmup() {
  g_x86_fetch_width = 1;
  g_rename_width = 1;
  g_execute_width = 1;
  g_memory_issue_width = 1;
  g_store_issue_width = 1;
  g_retire_width = 1;
  g_L1D_latency = 0;  // the L1 hit latency is already accounted for in the FU latency
  g_L2_latency = 1;
  g_mem_latency = 1;
  g_load_latency = 1;

  g_fpipe_depth = 1;
  g_cpipe_depth = 1;
  g_rpipe_depth = 1;
  g_front_end_max = g_rename_width * (g_fpipe_depth + 1);
  g_rob_size = 20;  // shouldn't be an issue, but make sure that it is at least as big as the largest x86 inst.

  if (!g_params.getRuby()) {
    dynamic_cast<SimpleMemoryInterface *>(m_mem_interface)->initHitLatencies();
  }

  real_inst_record_compressor_t *new_rir_handler = 0;
  if (strcmp(g_params.getDumpFilePath().c_str(), "/dev/null") != 0) {
    string dumpFilePath = g_params.getDumpFilePath();
    if (SIM_number_processors() > 1) {
      ostringstream temp;
      temp << dumpFilePath << "." << m_processor_number;
      dumpFilePath = temp.str();
    }
    new_rir_handler = new real_inst_record_compressor_t(dumpFilePath.c_str());
  }
  real_inst_record_handler_t *old_handler = setRIRHandler(new_rir_handler);
  if (old_handler) {
    delete old_handler;
  }
}

void 
Processor::dataCollect() {
  g_x86_fetch_width = 3;
  g_rename_width = 4;
  g_execute_width = 4;
  g_memory_issue_width = 2;
  g_store_issue_width = 1;
  g_retire_width = 4;
  g_L1D_latency = 0;  // the L1 hit latency is already accounted for in the FU latency
  g_L2_latency = 8;
  g_mem_latency = 100;
  g_load_latency = 3;

  g_fpipe_depth = 4; // cfg.getLong(proc, "frontEndStages");  
  g_cpipe_depth = 4; // cfg.getLong(proc, "coreStages");  
  g_rpipe_depth = 2; // cfg.getLong(proc, "coreStages");  
  g_front_end_max = g_rename_width * (g_fpipe_depth + 1);
  g_rob_size = 80; //  + (g_params.getIgnoreBrrs() ? 2 : 0); // give a little extra ROB space for brrs ?? 

  if (!g_params.getRuby()) {
    dynamic_cast<SimpleMemoryInterface *>(m_mem_interface)->initHitLatencies();
  }

  real_inst_record_compressor_t *new_rir_handler = 0;
  if (strcmp(g_params.getDumpFilePath().c_str(), "/dev/null") != 0) {
    string dumpFilePath = g_params.getDumpFilePath();
    if (SIM_number_processors() > 1) {
      ostringstream temp;
      temp << dumpFilePath << "." << m_processor_number;
      dumpFilePath = temp.str();
    }
    new_rir_handler = new real_inst_record_compressor_t(dumpFilePath.c_str());
  }
  real_inst_record_handler_t *old_handler = setRIRHandler(new_rir_handler);
  if (old_handler) {
    delete old_handler;
  }
}

void Processor::print(void) {
  if (strcmp(g_params.getPredictorAccuracyFilePath().c_str(), "/dev/null") != 0) {
    string predictorAccuracyFilePath = g_params.getPredictorAccuracyFilePath();
    if (SIM_number_processors() > 1) {
      ostringstream temp;
      temp << predictorAccuracyFilePath << "." << m_processor_number;
      predictorAccuracyFilePath = temp.str();
    }
    FILE *outfile = fopen(predictorAccuracyFilePath.c_str(), "w");
    m_predictors.printAccuracies(outfile);
    fclose(outfile);
  }
}

void
Processor::cacheAccess(Waiter *w, RequestType t, Waddr addr) {
  assert(m_mem_interface != NULL);
  m_mem_interface->request(addr, t, w);
  return;
}

bool
Processor::cacheReady() {
  assert(m_mem_interface != NULL);
  return m_mem_interface->ready();
}

void
Processor::cachePermissionChangeNotification(Waddr addr, AccessPermission old_perm, AccessPermission new_perm) {
  assert(Address(addr).isBlockAligned());
  assert(new_perm != AccessPermission_NotPresent);

  if ((old_perm == AccessPermission_NotPresent) ||
      (old_perm == AccessPermission_Invalid) ||
      (old_perm == new_perm)) {
    return;
  }

  if (new_perm == AccessPermission_Invalid) {
    // Invalidation of a line, check for a consistency violation
    m_lsq.invalidationSearch(addr);
  }
}

DynamicInst *
Processor::getDynamicInst(QPointer logical_index) {
  return &(m_inst_buffer[mappedIndex(logical_index)]);
}
