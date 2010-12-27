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
//! path.
//
//===----------------------------------------------------------------------===//

#include "inst_record.h"
#include <cassert>
#include <set>

void
real_inst_record_t::Clear() {
  PC = 0;
  for (int i = 0 ; i < MAX_STAGES ; i ++) { times[i] = 0; }
  for (int i = 0 ; i < 3 ; i ++) { reg_producer[i] = 0; }
  events = 0;
}

real_inst_record_t::real_inst_record_t() {
  Clear();
}

void 
real_inst_record_t::Set(int stage, q_pointer_t time) { 
  assert((stage >= 0) && (stage < MAX_STAGES));
  times[stage] = time;
}

q_pointer_t 
real_inst_record_t::Get(int stage) {
  assert((stage >= 0) && (stage < MAX_STAGES));
  return times[stage];
}

void 
real_inst_record_t::SetPC(W64 PC) {
  this->PC = PC;
}

W64 
real_inst_record_t::GetPC() {
  return PC;
}

void 
real_inst_record_t::SetUopNum(W8 _uop_num) {
  uop_num = _uop_num;
}

W8 
real_inst_record_t::GetUopNum() {
  return uop_num;
}

void 
real_inst_record_t::SetRegProducer(q_pointer_t ddp) {
  for (int i = 0 ; i < 3 ; i ++) {
	 if (reg_producer[i] == ddp) { /* already have the dependence */
		return;
	 }
	 if (reg_producer[i] == 0) {
		reg_producer[i] = ddp;
		return;
	 }
  } 
  assert(0);
}

q_pointer_t 
real_inst_record_t::GetRegProducer(int i) {
  assert ((i >= 0) && (i < 3));
  return reg_producer[i];
}

void 
real_inst_record_t::Print(FILE *file) {
  fprintf(file, "0x%llx:%2d %c%c%c%c%c%c%c%c%c %8dF %8dD %8dQ %8dR %8dE %8dC %8dR", PC, uop_num,
          (events & ICACHE_MISS ? 'I' : ' '), (events & DCACHE_MISS ? 'D' : ' '), 
          (events & BRANCH_MISP ? 'B' : ' '), (events & ORDERING_VIOLATION_S ? 'S' : ' '), 
          (events & ORDERING_VIOLATION_L ? 'L' : ' '), (events & COHERENCE_VIOLATION ? 'C' : ' '), 
          (events & WRONG_PATH ? 'W' : ' '), (events & PIPELINE_RESET ? 'R' : ' '),
          (events & UNALIGNED ? 'U' : ' '),
          (int)times[FETCH_STAGE], (int)times[DECODE_STAGE], 
          (int)times[QUEUE_STAGE], (int)times[READY_STAGE], 
          (int)times[EXECUTE_STAGE], (int)times[COMPLETE_STAGE], 
          (int)times[RETIRE_STAGE]);
  for (int i = 0 ; i < 3 ; i ++) {
	 if (reg_producer[i] != 0) { fprintf(file, " (%8d)", (int)reg_producer[i]); }
  }
  fprintf(file, "\n");
}

std::set<real_inst_record_t *> RIR_POOL;

real_inst_record_t *
real_inst_record_factory_t::Allocate() {
  real_inst_record_t *ret_val;
  if (RIR_POOL.empty()) {
	 ret_val = new real_inst_record_t;
  } else {
	 ret_val = *RIR_POOL.begin();
	 RIR_POOL.erase(ret_val);
  }
  ret_val->Clear();
  ret_val->AddEvent(events);
  events = 0;
  return ret_val;
}

void
real_inst_record_t::Free() {
  assert(RIR_POOL.find(this) == RIR_POOL.end());
  RIR_POOL.insert(this);
}

void 
real_inst_record_t::Compress(q_pointer_t q, FILE *file) {
  comp_irecord_t cir;
  
  cir.PC = PC;
  cir.uop_num = uop_num;
  cir.base_time = times[FETCH_STAGE];
  for (int i = 0 ; i < MAX_STAGES ; i ++) { 
	 q_pointer_t temp = times[i] - cir.base_time;
	 assert(temp < (1 << 16));
	 cir.time_offset[i] = temp;
  }
  int j = 0;
  for (int i = 0 ; i < 3 ; i ++) { 
	 cir.dep_offset[i] = 0;
	 q_pointer_t temp = q - reg_producer[i];
	 if ((reg_producer[i] != 0) && (temp < (1 << 16))) {
		cir.dep_offset[j] = temp;
		j ++;
	 } 
  }
  cir.events = events;

  fwrite(&cir, sizeof(comp_irecord_t), 1, file);
}
 
compressed_inst_record_t::compressed_inst_record_t() {
  q = 0;
  cir = NULL;
}

void 
compressed_inst_record_t::Set(int stage, q_pointer_t time) { 
  assert(0);
}

q_pointer_t 
compressed_inst_record_t::Get(int stage) {
  assert((stage >= 0) && (stage < MAX_STAGES));
  return cir->base_time + cir->time_offset[stage];
}

void 
compressed_inst_record_t::SetPC(W64 PC) {
  assert(0);
}

W64 
compressed_inst_record_t::GetPC() {
  return cir->PC;
}

void 
compressed_inst_record_t::SetUopNum(W8 _uop_num) {
  cir->uop_num = _uop_num;
}

W8 
compressed_inst_record_t::GetUopNum() {
  return cir->uop_num;
}

void 
compressed_inst_record_t::SetRegProducer(q_pointer_t ddp) {
  assert(0);
}

q_pointer_t 
compressed_inst_record_t::GetRegProducer(int i) {
  assert ((i >= 0) && (i < 3));
  if(cir->dep_offset[i] == 0) return 0;
  return q - cir->dep_offset[i];
}

void 
compressed_inst_record_t::AddEvent(int event) { 
  assert(0);
}

W16
compressed_inst_record_t::GetEvents() {
  return cir->events;
}


void 
compressed_inst_record_t::Print(FILE *file) {
  fprintf(file, "0x%llx:%2d %c%c%c%c%c%c%c%c%c %8dF %8dD %8dQ %8dR %8dE %8dC %8dR", GetPC(), cir->uop_num,
          (cir->events & ICACHE_MISS ? 'I' : ' '), (cir->events & DCACHE_MISS ? 'D' : ' '), 
          (cir->events & BRANCH_MISP ? 'B' : ' '), (cir->events & ORDERING_VIOLATION_S ? 'S' : ' '), 
          (cir->events & ORDERING_VIOLATION_L ? 'L' : ' '), (cir->events & COHERENCE_VIOLATION ? 'C' : ' '), 
          (cir->events & WRONG_PATH ? 'W' : ' '), (cir->events & PIPELINE_RESET ? 'R' : ' '),
          (cir->events & UNALIGNED ? 'U' : ' '),
          (int)Get(FETCH_STAGE), (int)Get(DECODE_STAGE), (int)Get(QUEUE_STAGE), 
          (int)Get(READY_STAGE), (int)Get(EXECUTE_STAGE), (int)Get(COMPLETE_STAGE), 
          (int)Get(RETIRE_STAGE));
  for (int i = 0 ; i < 3 ; i ++) {
	 if (cir->dep_offset[i] != 0) { fprintf(file, " (%8d)", (int)(q - cir->dep_offset[i])); }
  }
  fprintf(file, "\n");
}

void
compressed_inst_record_t::Print(FILE *file, inst_record_t::stage_t stage){

  fprintf(file, "0x%llx:\t", GetPC());
  switch(stage){
  case QUEUE_STAGE:
    fprintf(file, "D\t");
    break;
  case COMPLETE_STAGE:
    fprintf(file, "E\t");
    break;
  case RETIRE_STAGE:
    fprintf(file, "C\t");
    break;
  default: 
    break;
  }
  fprintf(file, "%8d\n", (int)Get(stage));
}


