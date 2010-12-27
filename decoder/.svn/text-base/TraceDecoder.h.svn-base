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

#ifndef __TRACE_DECODER_H
#define __TRACE_DECODER_H

#include "ptlhwdef.h"
#include "globals.h"

struct RexByte {
  // a.k.a., b, x, r, w
  byte extbase:1, extindex:1, extreg:1, mode64:1, insnbits:4; 
  RexByte() { }
  RexByte(const byte& b) { *((byte*)this) = b; }
  operator byte() const { return (*((byte*)this)); }
};

struct ModRMByte { 
  byte rm:3, reg:3, mod:2; 
  ModRMByte() { }
  ModRMByte(const byte& b) { *((byte*)this) = b; }
  //operator bool() { return (*((byte*)this)) != 0; }
  operator byte() const { return (*((byte*)this)); }
};

struct SIBByte {
  byte base:3, index:3, scale:2;
  SIBByte() { }
  SIBByte(const byte& b) { *((byte*)this) = b; }
  operator byte() const { return (*((byte*)this)); }
};

static const int PFX_REPZ      = (1 << 0);
static const int PFX_REPNZ     = (1 << 1);
static const int PFX_LOCK      = (1 << 2);
static const int PFX_CS        = (1 << 3);
static const int PFX_SS        = (1 << 4);
static const int PFX_DS        = (1 << 5);
static const int PFX_ES        = (1 << 6);
static const int PFX_FS        = (1 << 7);
static const int PFX_GS        = (1 << 8);
static const int PFX_DATA      = (1 << 9);
static const int PFX_ADDR      = (1 << 10);
static const int PFX_REX       = (1 << 11);
static const int PFX_FWAIT     = (1 << 12);
static const int PFX_count     = 13;

extern const char* prefix_names[PFX_count];

#define FLAGS_DEFAULT_ALU SETFLAG_ZF|SETFLAG_CF|SETFLAG_OF

enum {
  // 64-bit
  APR_rax, APR_rcx, APR_rdx, APR_rbx, APR_rsp, APR_rbp, APR_rsi, APR_rdi, APR_r8, APR_r9, APR_r10, APR_r11, APR_r12, APR_r13, APR_r14, APR_r15,
  // 32-bit
  APR_eax, APR_ecx, APR_edx, APR_ebx, APR_esp, APR_ebp, APR_esi, APR_edi, APR_r8d, APR_r9d, APR_r10d, APR_r11d, APR_r12d, APR_r13d, APR_r14d, APR_r15d,
  // 16-bit
  APR_ax, APR_cx, APR_dx, APR_bx, APR_sp, APR_bp, APR_si, APR_di, APR_r8w, APR_r9w, APR_r10w, APR_r11w, APR_r12w, APR_r13w, APR_r14w, APR_r15w,
  // 8-bit
  APR_al, APR_cl, APR_dl, APR_bl, APR_ah, APR_ch, APR_dh, APR_bh,
  // 8-bit with REX, not double-counting the regular 8-bit regs:
  APR_spl, APR_bpl, APR_sil, APR_dil,
  APR_r8b, APR_r9b, APR_r10b, APR_r11b, APR_r12b, APR_r13b, APR_r14b, APR_r15b,
  // SSE registers
  APR_xmm0, APR_xmm1, APR_xmm2, APR_xmm3, APR_xmm4, APR_xmm5, APR_xmm6, APR_xmm7, APR_xmm8, APR_xmm9, APR_xmm10, APR_xmm11, APR_xmm12, APR_xmm13, APR_xmm14, APR_xmm15, 
  // segments:
  APR_es, APR_cs, APR_ss, APR_ds, APR_fs, APR_gs,
  // special:
  APR_rip, APR_zero, APR_COUNT,
};

extern const char* uniform_arch_reg_names[APR_COUNT];

extern const byte arch_pseudo_reg_to_arch_reg[APR_COUNT];

enum { b_mode = 1, v_mode, w_mode, d_mode, q_mode, x_mode, m_mode, cond_jump_mode, loop_jcxz_mode, dq_mode };

struct ArchPseudoRegInfo {
  W32 sizeshift:3, hibyte:1;
};

extern const ArchPseudoRegInfo reginfo[APR_COUNT];

extern const byte reg64_to_uniform_reg[16];
extern const byte xmmreg_to_uniform_reg[16];
extern const byte reg32_to_uniform_reg[16];
extern const byte reg16_to_uniform_reg[16];
extern const byte reg8_to_uniform_reg[8];
extern const byte reg8x_to_uniform_reg[16];
extern const byte segreg_to_uniform_reg[16];

//
// Decoded Operand
//

enum { OPTYPE_NONE, OPTYPE_REG, OPTYPE_MEM, OPTYPE_IMM };

struct TraceDecoder;

struct DecodedOperand {
  int type;
  bool indirect;
  union {
    struct {
      int reg;
    } reg;

    struct {
      W64s imm;
    } imm;

    struct {
      int size;
      int basereg;
      int indexreg;
      int scale;
      W64s offset;
      bool riprel;
    } mem;
  };

  bool gform_ext(TraceDecoder& state, int bytemode, int regfield, bool def64 = false, bool in_ext_z = false);
  bool gform(TraceDecoder& state, int bytemode);
  bool iform(TraceDecoder& state, int bytemode);
  bool iform64(TraceDecoder& state, int bytemode);
  bool eform(TraceDecoder& state, int bytemode);
  bool varreg(TraceDecoder& state, int bytemode, bool def64);
  bool varreg_def32(TraceDecoder& state, int bytemode);
  bool varreg_def64(TraceDecoder& state, int bytemode);

  ostream& print(ostream& os) const;
};

static inline ostream& operator <<(ostream& os, const DecodedOperand& decop) {
  return decop.print(os);
}

class NotEnoughBytesException: public exception{};
class InvalidOpcodeException: public exception{};
class UnimplementedOpcodeException: public exception{};

struct TraceDecoder {
  TransOp* transbuf;
  int transbufcount;
  byte use64;
  byte kernel;
  byte dirflag;
  byte* insnbytes;
  int insnbytes_bufsize;
  Waddr rip;
  Waddr ripstart;
  int byteoffset;
  int valid_byte_count;
  int op;
  W32 prefixes;
  ModRMByte modrm;
  RexByte rex;
  W64 user_insn_count;
  bool last_flags_update_was_atomic;
  bool invalid;
  PageFaultErrorCode pfec;
  Waddr faultaddr;
  bool opsize_prefix;
  bool addrsize_prefix;
  bool end_of_block;
  bool is_x87;
  bool is_sse;
  bool some_insns_complex;
  bool split_basic_block_at_locks_and_fences;
  bool split_invalid_basic_blocks;
  bool split_unaligned_memops;

  int outcome;

  enum {
    DECODE_OUTCOME_OK             = 0,
    DECODE_OUTCOME_PAGE_FAULT     = 1,
    DECODE_OUTCOME_INVALID_OPCODE = 2,
    DECODE_OUTCOME_GP_FAULT       = 3,
  };

  Level1PTE ptelo;
  Level1PTE ptehi;

  TraceDecoder(const RIPVirtPhys& rvp);
  TraceDecoder(Context& ctx, Waddr rip);

  void reset();
  void decode_prefixes();
  void immediate(int rdreg, int sizeshift, W64s imm, bool issigned = true);
  int bias_by_segreg(int basereg);
  void address_generate_and_load_or_store(int destreg, int srcreg, const DecodedOperand& memref, int opcode, int datatype = DATATYPE_INT, int cachelevel = 0, bool force_seg_bias = false);
  void operand_load(int destreg, const DecodedOperand& memref, int loadop = OP_ld, int datatype = 0, int cachelevel = 0);
  void result_store(int srcreg, int tempreg, const DecodedOperand& memref, int datatype = 0);
  void alu_reg_or_mem(int opcode, const DecodedOperand& rd, const DecodedOperand& ra, W32 setflags, int rcreg, 
                      bool flagsonly = false, bool isnegop = false, bool ra_rb_imm_form = false, W64s ra_rb_imm_form_rbimm = 0);

  void move_reg_or_mem(const DecodedOperand& rd, const DecodedOperand& ra, int force_rd = REG_zero);
  void signext_reg_or_mem(const DecodedOperand& rd, DecodedOperand& ra, int rasize, bool zeroext = false);
  bool memory_fence_if_locked(bool end_of_x86_insn = 0, int type = MF_TYPE_LFENCE|MF_TYPE_SFENCE);

  void fillbuf(byte* insnBuf, int insnBufSize); // jld

  inline W64 fetch(int n) { 
    if ((byteoffset + n - 1) >= insnbytes_bufsize){
      throw NotEnoughBytesException();
    }
    W64 r = lowbits(*((W64*)&insnbytes[byteoffset]), n*8); 
    rip += n; byteoffset += n; return r; 
  }
  inline byte fetch1() { 
    if (byteoffset >= insnbytes_bufsize){
      throw NotEnoughBytesException();
    }
    byte r = *((byte*)&insnbytes[byteoffset]); 
    rip += 1; byteoffset += 1; return r; 
  }
  inline W16 fetch2() { 
    if ((byteoffset + 1) >= insnbytes_bufsize){
      throw NotEnoughBytesException();
    }
    W16 r = *((W16*)&insnbytes[byteoffset]); 
    rip += 2; byteoffset += 2; return r; 
  }
  inline W32 fetch4() { 
    if ((byteoffset + 3) >= insnbytes_bufsize){
      throw NotEnoughBytesException();
    }
    W32 r = *((W32*)&insnbytes[byteoffset]); 
    rip += 4; byteoffset += 4; return r; 
  }
  inline W64 fetch8() { 
    if ((byteoffset + 7) >= insnbytes_bufsize){
      throw NotEnoughBytesException();
    }
    W64 r = *((W64*)&insnbytes[byteoffset]); 
    rip += 8; byteoffset += 8; return r; 
  }

  bool invalidate();
  bool decode_fast();
  void decode_complex();
  void decode_sse();
  void decode_x87();

  int translate(byte* insnBuf, int insnBufSize, TransOp* output_buf);
  void put(const TransOp& transop);
  void emitop(const TransOp& transop);
  void lastop();
  void split(Waddr target);
  void split_before() { split(ripstart); }
  void split_after() { split(rip); }
}; // end struct TraceDecoder

static inline TraceDecoder* operator <<(TraceDecoder* dec, const TransOp& transop) {
  dec->put(transop);
  return dec;
}

static inline TraceDecoder& operator <<(TraceDecoder& dec, const TransOp& transop) {
  dec.put(transop);
  return dec;
}

static const int MAX_TRANSOP_BUFFER_SIZE = 4;

struct TransOpBuffer {
  TransOp uops[MAX_TRANSOP_BUFFER_SIZE];
  uopimpl_func_t synthops[MAX_TRANSOP_BUFFER_SIZE];
  int index;
  int count;

  bool get(TransOp& uop, uopimpl_func_t& synthop) {
    if (!count) return false;
    uop = uops[index];
    synthop = synthops[index];
    index++;
    if (index >= count) { count = 0; index = 0; }
    return true;
  }

  void reset() {
    index = 0;
    count = 0;
  }

  int put() {
    assert(count < MAX_TRANSOP_BUFFER_SIZE);
    return count++;
  }

  bool empty() const {
    return (count == 0);
  }

  TransOpBuffer() { reset(); }
}; // end struct TransOpBuffer
void split_unaligned(const TransOp& transop, TransOpBuffer& buf);


//
// Generate a memory fence of the specified type.
//
inline bool TraceDecoder::memory_fence_if_locked(bool end_of_x86_insn, int type) {
  if (!(prefixes & PFX_LOCK)) return false;

  if (split_basic_block_at_locks_and_fences) {
    if (end_of_x86_insn) {
      //
      // Final mf that terminates the insn (always in its own BB): terminate the BB here.
      //
      TransOp mf(OP_mf, REG_temp0, REG_zero, REG_zero, REG_zero, 0);
      mf.extshift = type;
      this << mf;
      split_after();
      return true;
    } else {
      TransOp mf(OP_mf, REG_temp0, REG_zero, REG_zero, REG_zero, 0);
      mf.extshift = type;
      this << mf;
      return false; // continue emitting other uops
    }
  } else {
    //
    // Always emit fence intermingled with other insns
    //
    TransOp mf(OP_mf, REG_temp0, REG_zero, REG_zero, REG_zero, 0);
    mf.extshift = type;
    this << mf;
  }

  // never reached:
  return false;
}

enum {
  DECODE_TYPE_FAST, DECODE_TYPE_COMPLEX, DECODE_TYPE_X87, DECODE_TYPE_SSE, DECODE_TYPE_ASSIST, DECODE_TYPE_COUNT,
};

#define DECODE(form, decbuf, mode) invalid |= (!decbuf.form(*this, mode));

//
// Global functions
//
void init_decode();
void shutdown_decode();

static const int BB_CACHE_SIZE = 16384;

enum {
  INVALIDATE_REASON_SMC = 0,
  INVALIDATE_REASON_DMA,
  INVALIDATE_REASON_SPURIOUS,
  INVALIDATE_REASON_RECLAIM,
  INVALIDATE_REASON_DIRTY,
  INVALIDATE_REASON_EMPTY,
  INVALIDATE_REASON_COUNT
};

//
// This part is used when parsing stats.h to build the
// data store template; these must be in sync with the
// corresponding definitions elsewhere.
//
#ifdef DSTBUILD
static const char* decode_type_names[DECODE_TYPE_COUNT] = {
  "fast", "complex", "x87", "sse", "assist"
};

static const char* invalidate_reason_names[INVALIDATE_REASON_COUNT] = {
  "smc", "dma", "spurious", "reclaim", "dirty", "empty"
};
#endif


//
// uop implementations
//

struct AddrPair {
  byte* start;
  byte* end;
};

void init_uops();
void shutdown_uops();
uopimpl_func_t get_synthcode_for_uop(int op, int size, bool setflags, int cond, int extshift, int sfra, int cachelevel, bool except, bool internal);
uopimpl_func_t get_synthcode_for_cond_branch(int opcode, int cond, int size, bool except);

#endif
