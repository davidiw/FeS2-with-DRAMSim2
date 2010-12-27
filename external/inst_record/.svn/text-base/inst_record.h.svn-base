// ----------------------------------------------------------------------
//
//  This file is part of TraceVis.
//
//  TraceVis is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  TraceVis is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with TraceVis.  If not, see <http://www.gnu.org/licenses/>.
//
// ----------------------------------------------------------------------

//===-- inst_record.h - track events in inst's lifetime ---*- C++ -*--=//
//
//! This object is used to track the events in an instruction's
//! lifetime for use in plotting an execution and finding the critical
//! path.  The object tracks times it reaches different stages,
//! uncommon events (e.g., cache misses, mispredictions), and register
//! dependences.  
//
//===----------------------------------------------------------------------===//

#ifndef __INST_RECORD_H
#define __INST_RECORD_H

#include <stdio.h>
#include "host.h"
#define MAX_PRODUCER_REGS 4

/* Machine independent type definitions */
typedef byte_t W8;
typedef half_t W16;
typedef uquad_t W64;

/* definition of a type for indexing into a trace of dynamic
   instructions.  Made to be 64 bits to avoid overflow on long
   simulations. */
typedef uquad_t q_pointer_t;

/*******************************************************************/
/************************** inst_record_t **************************/
/*******************************************************************/

//! an abstract base class that defines the interface.
class inst_record_t {
public:
  enum stage_t {FETCH_STAGE, DECODE_STAGE, QUEUE_STAGE, READY_STAGE, 
                EXECUTE_STAGE, COMPLETE_STAGE, RETIRE_STAGE, MAX_STAGES};
  enum event_t {ICACHE_MISS = 1 << 0, DCACHE_MISS = 1 << 1, 
                BRANCH_MISP = 1 << 2, ORDERING_VIOLATION_S = 1 << 3, 
                ORDERING_VIOLATION_L = 1 << 4, COHERENCE_VIOLATION = 1 << 5, 
                WRONG_PATH = 1 << 6, PIPELINE_RESET = 1 << 7, UNALIGNED = 1 << 8 };
  virtual ~inst_record_t() {}
  virtual void Free() = 0;

  // associate (and read) a time with a transition into a new pipeline stage.
  virtual void Set(int stage, q_pointer_t time) = 0;
  virtual q_pointer_t Get(int stage) = 0;

  // associate (and read) a PC with an instruction in the trace.
  virtual void SetPC(W64 PC) = 0;
  virtual W64 GetPC() = 0;

  // associate (and read) a PC with an instruction in the trace.
  virtual void SetUopNum(W8 uop_num) = 0;
  virtual W8 GetUopNum() = 0;

  // associate (and read) upto MAX_PRODUCER_REGS data-dependent
  // predecessors (described by their index into the trace) with an
  // instruction
  virtual void SetRegProducer(q_pointer_t ddp) = 0; 
  virtual q_pointer_t GetRegProducer(int i) = 0; 
  virtual int GetMaxProducerRegs() { return MAX_PRODUCER_REGS; }

  // associate (and read) events with an instruction
  virtual void AddEvent(int event) = 0;
  virtual W16 GetEvents() = 0;

  // print text representation of the instruction
  virtual void Print(FILE *file) = 0;
};

/*******************************************************************/
/************************ real_inst_record_t ***********************/
/*******************************************************************/

//! this implementation is for use during construction and can be used
//! for short lived objects.  For more persistent records, the
//! Compress method is used to write a smaller version of the object
//! to a file.

class real_inst_record_t : public inst_record_t {
public:
  real_inst_record_t();
  ~real_inst_record_t() {}

  void Clear();

  virtual void Set(int stage, q_pointer_t time);
  virtual q_pointer_t Get(int stage);
  virtual void SetPC(W64 PC);
  virtual W64 GetPC();
  virtual void SetUopNum(W8 _uop_num);
  virtual W8 GetUopNum();
  virtual void SetRegProducer(q_pointer_t ddp); /* data-dependent predecessor */
  virtual q_pointer_t GetRegProducer(int i); /* upto MAX_PRODUCER_REGS of them */
  virtual void AddEvent(int event) { events |= event; }
  virtual W16 GetEvents() { return events; }
  virtual void Print(FILE *file);
  void Compress(q_pointer_t q, FILE *file);

  virtual void Free();
  
private:
  W64 PC;
  q_pointer_t times[MAX_STAGES];
  q_pointer_t reg_producer[MAX_PRODUCER_REGS];
  W16 events;
  W8 uop_num;
};

/*******************************************************************/
/******************** real_inst_record_factory_t *******************/
/*******************************************************************/

// We use a factory for allocating these because it simplifies
// associating events (e.g., I-cache misses) with an instruction
// before we've instantiated the trace record.

class real_inst_record_factory_t {
public:
  real_inst_record_factory_t() { events = 0; }

  real_inst_record_t *Allocate();
  void AddEventForNextRecord(int event) { events |= event; }

private:
  W16 events;
};

/*******************************************************************/
/********************* compressed_inst_record_t ********************/
/*******************************************************************/

//! this structure is how compressed versions are actually stored on
//! disk.  The object is used to give this data structure the interface
//! consistent with the above abstract type.

// We store the times for all but the first stage as 16-bit offsets
// from the time of the first stage.  This effectively limits an 
// instruction's lifetime to 65,000 cycles.

// We store instruction dependences as negative 16-bit offsets from
// the current instruction, limiting the effective instruction window
// size to 65,000 inflight instructions.

struct comp_irecord_t {
  W64 PC;
  q_pointer_t base_time;
  W16 time_offset[inst_record_t::MAX_STAGES];
  W16 dep_offset[MAX_PRODUCER_REGS];
  W16 events;
  W8 uop_num;
} __attribute__((__packed__));


// This object is used to compress a inst_record into the
// comp_irecord_t structure above.  A single instance of the object
// below can be used to compress many trace records by using the
// Set(q, cir) method to point to a new comp_irecord_t.

class compressed_inst_record_t : public inst_record_t {
public:
  compressed_inst_record_t();
  ~compressed_inst_record_t() {}

  void Set(q_pointer_t _q, comp_irecord_t *_cir) { q = _q; cir = _cir; }

  virtual void Set(int stage, q_pointer_t time);
  virtual q_pointer_t Get(int stage);
  virtual void SetPC(W64 PC);
  virtual W64 GetPC();
  virtual void SetUopNum(W8 _uop_num);
  virtual W8 GetUopNum();
  virtual void SetRegProducer(q_pointer_t ddp); /* data-dependent predecessor */
  virtual q_pointer_t GetRegProducer(int i); /* upto MAX_PRODUCER_REGS of them */
  virtual void AddEvent(int event);
  virtual W16 GetEvents();
  virtual void Print(FILE *file);
  virtual void Print(FILE *file, inst_record_t::stage_t stage);

  virtual void Free() {} /* do nothing */
  
private:
  q_pointer_t q;
  comp_irecord_t *cir;
};

/*******************************************************************/
/******************** real_inst_record_handler_t *******************/
/*******************************************************************/

// The inst_record_handlers defined below serve as sinks for a stream
// of real_inst_records.  The first compresses them to a file.  The
// second prints them out to a file.  Both deallocate the
// real_inst_records after they are processed.

class real_inst_record_handler_t {
public:
  virtual ~real_inst_record_handler_t() {}
  virtual void Process(real_inst_record_t *rir, q_pointer_t q) = 0;
};

class real_inst_record_compressor_t : public real_inst_record_handler_t {
public:
  real_inst_record_compressor_t(const char *filename) { record_file = fopen(filename, "w"); }
  virtual ~real_inst_record_compressor_t() { fclose(record_file); }

  virtual void Process(real_inst_record_t *rir, q_pointer_t q) {
	 if (rir == NULL) { return; }
	 rir->Compress(q, record_file);
	 rir->Free();
  }

private:
  FILE *record_file;
};

class real_inst_record_printer_t : public real_inst_record_handler_t {
public:
  real_inst_record_printer_t(FILE *file) { record_file = file; }
  real_inst_record_printer_t(const char *filename) { record_file = fopen(filename, "w"); }
  virtual ~real_inst_record_printer_t() { fclose(record_file); }

  virtual void Process(real_inst_record_t *rir, q_pointer_t q) {
	 if (rir == NULL) { return; }
	 fprintf(record_file, "%8d ", (int)q);
	 rir->Print(record_file);
	 rir->Free();
  }

private:
  FILE *record_file;
};

#endif /* __INST_RECORD_H */


