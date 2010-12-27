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

#ifndef __PROCESSOR_H
#define __PROCESSOR_H

#include <simics/api.h>
#undef likely
#undef unlikely
#include <simics/arch/x86.h>
#include <simics/alloc.h>
#include <simics/utils.h>

#include "globals.h" 
#include "TraceDecoder.h"
#include "Vector.h"
#include "Map.h"
#include "RegFile.h"
#include "LoadStoreQueue.h"
#include "MemoryInterface.h"
#include "Event.h"
#include "Predictor.h"
#include "PipeStages.h"
#include "Waiter.h"
#include "AccessPermission.h"

class Scheduler;
class real_inst_record_handler_t;
class real_inst_record_factory_t;
class DynamicInst;

class Processor : public Context {

private:
  conf_object_t* m_cpu;
  int m_processor_number;
  bool m_is64bit;
  TraceDecoder* m_decoder;

  PhysicalFile m_physical_file;
  LogicalFile m_front_end_map, m_retire_map;

  DynamicInst *m_inst_buffer;
  unsigned m_buf_size;

  QPointer m_q_head;  // head points to next insertion slot (first empty slot)
  QPointer m_q_tail;  // tail points to the first uop in the oldest valid x86 instruction

  QPointer m_q_rename;      // points to the first instructions that hasn't been renamed
  QPointer m_q_execute;     // points to first not yet executed instruction
  QPointer m_q_retire;      // points to first not ready to retire instruction
  QPointer m_q_oldest_bad;  // points to the oldest bad uop if there is one (else -1)

  EventsQueue m_events_queue;

  PipeStages<QPointer> fetch_pipe;
  PipeStages<QPointer> core_pipe;
  PipeStages<QPointer> retire_pipe;

  W64 m_fetch_rip;
  W64 m_retire_rip;

  PredictorSet m_predictors;
  Scheduler *m_scheduler;
  LoadStoreQueue m_lsq;
  MemoryInterface *m_mem_interface;

  typedef std::pair<W64, W64> RangeType;
  Vector<RangeType> m_ram_ranges;

  real_inst_record_handler_t *record_handler;
  real_inst_record_factory_t *record_factory;

  bool m_interrupt_occurred;
  bool m_exception_occurred;
  bool m_crack_unaligned_memops;
  bool m_committing;
  bool m_reset_while_committing;

  unsigned mappedIndex(QPointer logical_index) { return logical_index & (m_buf_size - 1); }
  DynamicInst *getDynamicInst(QPointer logical_index);
  void resetUopBuf(void);
  void initializeUopBuf();
  bool validateRegister(int ptl_reg_id);
  bool validateFlags(W64 flagmask);
  bool validateArchitecturalRegisters(W64 flagmask);
  bool validateStore(const DoubleWord *store_operand);
  void stepSimicsCycles(int num_cycles);
  bool procInKernelMode();
  
  bool validatePyriteState(W64 flagmask); 
  void resetPyriteRegister(int ptl_reg_id);
  void resetPyriteArchitecturalRegisters();
  void clearTransientState(void);
  void resetPyriteState(void);
  void initializeProcessorState(void);
  bool fetchMoreBytes(Waddr &fetch_address, W8 *fetch_buffer, unsigned &num_bytes);
  bool decodeCurrentX86Instruction(bool &taken_branch);

  void logInvalidOpcodeEvent(InvalidOpcodeException& e);
  void logUnimplementedOpcodeEvent(UnimplementedOpcodeException& e, W64 rip);
  void logInstructionDecodeEvent(W64 insn_address, unsigned intruction_size,
                                 QPointer q_ptr, unsigned num_uops);
  void logNoTranslationForInstructionEvent(void);
  void logIncorrectExecutionEvent(W64 prev_rip);
  void logCycleStats(void);

  void fetch();
  void rename();
  void execute();
  void retire(void);
  bool commitX86Instruction(W64 insn_rip, W64 flagmask);

  void flushPipeline(QPointer q_first_bad);
  void replay(QPointer q_first_replayed);
  real_inst_record_handler_t *setRIRHandler(real_inst_record_handler_t *RIR_handler);

public:
  Processor(int processor_number);
  ~Processor() {}

  void reset();
  void warmup();
  void dataCollect();
  void dataDump();

  int getProcNum() const { return m_processor_number; }
  EventsQueue &getEventsQueue() { return m_events_queue; }
  LoadStoreQueue &getLSQ() { return m_lsq; }
  PhysicalFile &getPhysicalFile() { return m_physical_file; }
  void cacheAccess(Waiter *w, RequestType t, Waddr addr);
  bool cacheReady();
  MemoryInterface *getMemoryInterface() { return m_mem_interface; }
  LogicalFile &getFrontEndMap() { return m_front_end_map; }
  LogicalFile &getRetireMap() { return m_retire_map; }

  void setOldestBad(QPointer q) { m_q_oldest_bad = std::min(m_q_oldest_bad, q); }

  void wakeup(DynamicInst *d);
  W64 readSimicsRegister(int ptl_reg_id);
  W64 readSimicsRegisterByName(const char *reg_name);
  void resolveBranch(QPointer q, Waddr actual_target);
  void checkException(QPointer q);
  void alignmentException(QPointer q);
  Tick getCurrentCycle() { return m_events_queue.getCurrentCycle(); }

  void logUopExecution(TransOp &uop, W64 ra, W64 rb, W64 rc,
                       W64 raflags, W64 rbflags, W64 rcflags);
  void logUopExecutionResult(TransOp &uop, IssueState is, W64 flags);
  void logNoTranslationDataMemoryOperation(DynamicInst &dyn_uop, W64 addr, bool is_load);
  void logUnalignedDataMemoryOperation(DynamicInst &dyn_uop, W64 addr, bool is_load);
  void logStatsForUop(TransOp &uop);

  void init(void);
  void stepCycle();
  void setExceptionOccurred(bool b){ m_exception_occurred = b; }
  void setInterruptOccurred(bool b){ m_interrupt_occurred = b; }
  bool initializeRamRanges();
  bool validRamAddress(W64 physAddr);
  bool addressHasTranslation(W64 virtAddr, data_or_instr_t dataOrInstr);
  bool translateAddress(W64 virtAddr, W64 &physAddr, data_or_instr_t dataOrInstr);
  // max num_bytes is 8
  // address does not have to be aligned
  unsigned long long readFromSimicsMemory(W64 phys_addr,
                                          W64 num_bytes);

  void cachePermissionChangeNotification(Waddr a, AccessPermission old_perm, AccessPermission new_perm);

  void print(void);
}; // end class Processor

extern Vector<Processor*> g_processors_vec;

#endif
