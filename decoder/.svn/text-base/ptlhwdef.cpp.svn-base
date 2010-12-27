//
// PTLsim: Cycle Accurate x86-64 Simulator
// Hardware Definitions
//
// Copyright 1999-2006 Matt T. Yourst <yourst@yourst.com>
//
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

#include "TraceDecoder.h"
#include "ptlsim-stl.h"
//#include <dcache.h>

extern void print_message(const char* text);

const char* opclass_names[OPCLASS_COUNT] = {
  "logic", "addsub", "addsubc", "addshift", "sel", "cmp", "br.cc", "jmp", "bru", 
  "assist", "mf", "ld", "st", "ld.pre", "shiftsimple", "shift", "mul", "bitscan", "flags",  "chk", 
  "fpu", "fp-div-sqrt", "fp-cmp", "fp-perm", "fp-cvt-i2f", "fp-cvt-f2i", "fp-cvt-f2f",
};

//
// Micro-operation (uop) definitions
//
#define makeccbits(b0, b1, b2) ((b0 << 0) + (b1 << 1) + (b2 << 2))
#define ccA   makeccbits(1, 0, 0)
#define ccB   makeccbits(0, 1, 0)
#define ccAB  makeccbits(1, 1, 0)
#define ccABC makeccbits(1, 1, 1)
#define ccC   makeccbits(0, 0, 1)

#define makeopbits(b3, b4, b5) ((b3 << 3) + (b4 << 4) + (b5 << 5))

#define opA   makeopbits(1, 0, 0)
#define opAB  makeopbits(1, 1, 0)
#define opABC makeopbits(1, 1, 1)
#define opB   makeopbits(0, 1, 0)

const OpcodeInfo opinfo[OP_MAX_OPCODE] = {
  // name, opclass, latency, fu
  {"nop",            OPCLASS_LOGIC,         0          },
  {"mov",            OPCLASS_LOGIC,         opAB       }, // move or merge
  // Logical
  {"and",            OPCLASS_LOGIC,         opAB       },
  {"andnot",         OPCLASS_LOGIC,         opAB       },
  {"xor",            OPCLASS_LOGIC,         opAB       },
  {"or",             OPCLASS_LOGIC,         opAB       },
  {"nand",           OPCLASS_LOGIC,         opAB       },
  {"ornot",          OPCLASS_LOGIC,         opAB       },
  {"eqv",            OPCLASS_LOGIC,         opAB       },
  {"nor",            OPCLASS_LOGIC,         opAB       },
  // Mask, insert or extract bytes
  {"maskb",          OPCLASS_SIMPLE_SHIFT,  opAB       }, // mask rd = ra,rb,[ds,ms,mc], bytes only
  // Add and subtract
  {"add",            OPCLASS_ADDSUB,        opABC|ccC  }, // ra + rb
  {"sub",            OPCLASS_ADDSUB,        opABC|ccC  }, // ra - rb
  {"adda",           OPCLASS_ADDSHIFT,      opABC      }, // ra + rb + rc
  {"suba",           OPCLASS_ADDSHIFT,      opABC      }, // ra - rb + rc
  {"addm",           OPCLASS_ADDSUB,        opABC      }, // lowbits(ra + rb, m)
  {"subm",           OPCLASS_ADDSUB,        opABC      }, // lowbits(ra - rb, m)
  // Condition code logical ops
  {"andcc",          OPCLASS_FLAGS,         opAB|ccAB  },
  {"orcc",           OPCLASS_FLAGS,         opAB|ccAB  },
  {"xorcc",          OPCLASS_FLAGS,         opAB|ccAB  },
  {"ornotcc",        OPCLASS_FLAGS,         opAB|ccAB  },
  // Condition code movement and merging
  {"movccr",         OPCLASS_FLAGS,         opB|ccB    },
  {"movrcc",         OPCLASS_FLAGS,         opB        },
  {"collcc",         OPCLASS_FLAGS,         opABC|ccABC},
  // Simple shifting (restricted to small immediate 1..8)
  {"shls",           OPCLASS_SIMPLE_SHIFT,  opAB       }, // rb imm limited to 0-8
  {"shrs",           OPCLASS_SIMPLE_SHIFT,  opAB       }, // rb imm limited to 0-8
  {"bswap",          OPCLASS_LOGIC,         opAB       }, // byte swap rb
  {"sars",           OPCLASS_SIMPLE_SHIFT,  opAB       }, // rb imm limited to 0-8
  // Bit testing
  {"bt",             OPCLASS_LOGIC,         opAB       },
  {"bts",            OPCLASS_LOGIC,         opAB       },
  {"btr",            OPCLASS_LOGIC,         opAB       },
  {"btc",            OPCLASS_LOGIC,         opAB       },
  // Set and select
  {"set",            OPCLASS_SELECT,        opABC|ccC  },
  {"set.sub",        OPCLASS_SELECT,        opABC      },
  {"set.and",        OPCLASS_SELECT,        opABC      },
  {"sel",            OPCLASS_SELECT,        opABC|ccABC}, // rd = falsereg,truereg,condreg
  // Branches
  {"br",             OPCLASS_COND_BRANCH,   opAB|ccAB}, // branch
  {"br.sub",         OPCLASS_COND_BRANCH,   opAB     }, // compare and branch ("cmp" form: subtract)
  {"br.and",         OPCLASS_COND_BRANCH,   opAB     }, // compare and branch ("test" form: and)
  {"jmp",            OPCLASS_INDIR_BRANCH,  opA      }, // indirect user branch
  {"bru",            OPCLASS_UNCOND_BRANCH, 0        }, // unconditional branch (branch cap)
  {"jmpp",           OPCLASS_INDIR_BRANCH|OPCLASS_BARRIER,  opA}, // indirect branch within PTL
  {"brp",            OPCLASS_UNCOND_BRANCH|OPCLASS_BARRIER, 0}, // unconditional branch (PTL only)
  // Checks
  {"chk",            OPCLASS_CHECK,         opAB|ccAB}, // check condition and rollback if false (uses cond codes); rcimm is exception type
  {"chk.sub",        OPCLASS_CHECK,         opAB     }, // check ("cmp" form: subtract)
  {"chk.and",        OPCLASS_CHECK,         opAB     }, // check ("test" form: and)
  // Loads and stores
  {"ld",             OPCLASS_LOAD,          opABC    }, // load zero extended
  {"ldx",            OPCLASS_LOAD,          opABC    }, // load sign extended
  {"ld.pre",         OPCLASS_PREFETCH,      opAB     }, // prefetch
  {"st",             OPCLASS_STORE,         opABC    }, // store
  {"mf",             OPCLASS_FENCE,         0        }, // memory fence (extshift holds type: 01 = st, 10 = ld, 11 = ld.st)
  // Shifts, rotates and complex masking
  {"shl",            OPCLASS_SHIFTROT,      opABC|ccC},
  {"shr",            OPCLASS_SHIFTROT,      opABC|ccC},
  {"mask",           OPCLASS_SHIFTROT,      opAB     }, // mask rd = ra,rb,[ds,ms,mc]
  {"sar",            OPCLASS_SHIFTROT,      opABC|ccC},
  {"rotl",           OPCLASS_SHIFTROT,      opABC|ccC},  
  {"rotr",           OPCLASS_SHIFTROT,      opABC|ccC},   
  {"rotcl",          OPCLASS_SHIFTROT,      opABC|ccC},
  {"rotcr",          OPCLASS_SHIFTROT,      opABC|ccC},  
  // Multiplication
  {"mull",           OPCLASS_MULTIPLY,      opAB },
  {"mulh",           OPCLASS_MULTIPLY,      opAB },
  {"mulhu",          OPCLASS_MULTIPLY,      opAB },
  // Bit scans
  {"ctz",            OPCLASS_BITSCAN,       opB  },
  {"clz",            OPCLASS_BITSCAN,       opB  },
  {"ctpop",          OPCLASS_BITSCAN,       opB  },  
  {"permb",          OPCLASS_SHIFTROT,      opABC}, // from fpa port
  // Floating point
  // uop.size bits have following meaning:
  // 00 = single precision, scalar (preserve high 32 bits of ra)
  // 01 = single precision, packed (two 32-bit floats)
  // 1x = double precision, scalar or packed (use two uops to process 128-bit xmm)
  {"addf",           OPCLASS_FP_ALU,        opAB },
  {"subf",           OPCLASS_FP_ALU,        opAB },
  {"mulf",           OPCLASS_FP_ALU,        opAB },
  {"maddf",          OPCLASS_FP_ALU,        opABC},
  {"msubf",          OPCLASS_FP_ALU,        opABC},
  {"divf",           OPCLASS_FP_DIVSQRT,    opAB },
  {"sqrtf",          OPCLASS_FP_DIVSQRT,    opAB },
  {"rcpf",           OPCLASS_FP_DIVSQRT,    opAB },
  {"rsqrtf",         OPCLASS_FP_DIVSQRT,    opAB },
  {"minf",           OPCLASS_FP_COMPARE,    opAB },
  {"maxf",           OPCLASS_FP_COMPARE,    opAB },
  {"cmpf",           OPCLASS_FP_COMPARE,    opAB },
  // For fcmpcc, uop.size bits have following meaning:
  // 00 = single precision ordered compare
  // 01 = single precision unordered compare
  // 10 = double precision ordered compare
  // 11 = double precision unordered compare
  {"cmpccf",         OPCLASS_FP_COMPARE,    opAB },
  // and/andn/or/xor are done using integer uops
  {"permf",          OPCLASS_FP_PERMUTE,    opAB }, // shuffles
  // For these conversions, uop.size bits select truncation mode:
  // x0 = normal IEEE-style rounding
  // x1 = truncate to zero
  {"cvtf.i2s.ins",   OPCLASS_FP_CONVERTI2F, opAB }, // one W32s <rb> to single, insert into low 32 bits of <ra> (for cvtsi2ss)
  {"cvtf.i2s.p",     OPCLASS_FP_CONVERTI2F, opB  }, // pair of W32s <rb> to pair of singles <rd> (for cvtdq2ps, cvtpi2ps)
  {"cvtf.i2d.lo",    OPCLASS_FP_CONVERTI2F, opB  }, // low W32s in <rb> to double in <rd> (for cvtdq2pd part 1, cvtpi2pd part 1, cvtsi2sd)
  {"cvtf.i2d.hi",    OPCLASS_FP_CONVERTI2F, opB  }, // high W32s in <rb> to double in <rd> (for cvtdq2pd part 2, cvtpi2pd part 2)
  {"cvtf.q2s.ins",   OPCLASS_FP_CONVERTI2F, opAB }, // one W64s <rb> to single, insert into low 32 bits of <ra> (for cvtsi2ss with REX.mode64 prefix)
  {"cvtf.q2d",       OPCLASS_FP_CONVERTI2F, opAB }, // one W64s <rb> to double in <rd>, ignore <ra> (for cvtsi2sd with REX.mode64 prefix)
  {"cvtf.s2i",       OPCLASS_FP_CONVERTF2I, opB  }, // one single <rb> to W32s in <rd> (for cvtss2si, cvttss2si)
  {"cvtf.s2q",       OPCLASS_FP_CONVERTF2I, opB  }, // one single <rb> to W64s in <rd> (for cvtss2si, cvttss2si with REX.mode64 prefix)
  {"cvtf.s2i.p",     OPCLASS_FP_CONVERTF2I, opB  }, // pair of singles in <rb> to pair of W32s in <rd> (for cvtps2pi, cvttps2pi, cvtps2dq, cvttps2dq)
  {"cvtf.d2i",       OPCLASS_FP_CONVERTF2I, opB  }, // one double <rb> to W32s in <rd> (for cvtsd2si, cvttsd2si)
  {"cvtf.d2q",       OPCLASS_FP_CONVERTF2I, opB  }, // one double <rb> to W64s in <rd> (for cvtsd2si with REX.mode64 prefix)
  {"cvtf.d2i.p",     OPCLASS_FP_CONVERTF2I, opAB }, // pair of doubles in <ra> (high), <rb> (low) to pair of W32s in <rd> (for cvtpd2pi, cvttpd2pi, cvtpd2dq, cvttpd2dq), clear high 64 bits of dest xmm
  {"cvtf.d2s.ins",   OPCLASS_FP_CONVERTFP,  opAB }, // double in <rb> to single, insert into low 32 bits of <ra> (for cvtsd2ss)
  {"cvtf.d2s.p",     OPCLASS_FP_CONVERTFP,  opAB }, // pair of doubles in <ra> (high), <rb> (low) to pair of singles in <rd> (for cvtpd2ps)
  {"cvtf.s2d.lo",    OPCLASS_FP_CONVERTFP,  opB  }, // low single in <rb> to double in <rd> (for cvtps2pd, part 1, cvtss2sd)
  {"cvtf.s2d.hi",    OPCLASS_FP_CONVERTFP,  opB  }, // high single in <rb> to double in <rd> (for cvtps2pd, part 2)
  // Multiplication
  {"divr",           OPCLASS_DIVIDE,      opABC },
  {"divq",           OPCLASS_DIVIDE,      opABC },
  {"divru",          OPCLASS_DIVIDE,      opABC },
  {"divqu",          OPCLASS_DIVIDE,      opABC },
};

const char* exception_names[EXCEPTION_COUNT] = {
// 0123456789abcdef
  "NoException",
  "Propagate",
  "BranchMiss",
  "Unaligned",
  "PageRead",
  "PageWrite",
  "PageExec",
  "StStAlias",
  "LdStAlias",
  "CheckFailed",
  "SkipBlock",
  "CacheLocked",
  "LFRQFull",
  "Float",
  "FloatNotAvail"
};

const char* x86_exception_names[256] = {
  "divide",
  "debug",
  "nmi",
  "breakpoint",
  "overflow",
  "bounds",
  "invalid opcode",
  "fpu not avail",
  "double fault",
  "coproc overrun",
  "invalid tss",
  "seg not present",
  "stack fault",
  "gp fault",
  "page fault",
  "spurious int",
  "fpu",
  "unaligned",
  "machine check",
  "sse",
  "int14h", "int15h", "int16h", "int17h",
  "int18h", "int19h", "int1Ah", "int1Bh", "int1Ch", "int1Dh", "int1Eh", "int1Fh",
  "int20h", "int21h", "int22h", "int23h", "int24h", "int25h", "int26h", "int27h",
  "int28h", "int29h", "int2Ah", "int2Bh", "int2Ch", "int2Dh", "int2Eh", "int2Fh",
  "int30h", "int31h", "int32h", "int33h", "int34h", "int35h", "int36h", "int37h",
  "int38h", "int39h", "int3Ah", "int3Bh", "int3Ch", "int3Dh", "int3Eh", "int3Fh",
  "int40h", "int41h", "int42h", "int43h", "int44h", "int45h", "int46h", "int47h",
  "int48h", "int49h", "int4Ah", "int4Bh", "int4Ch", "int4Dh", "int4Eh", "int4Fh",
  "int50h", "int51h", "int52h", "int53h", "int54h", "int55h", "int56h", "int57h",
  "int58h", "int59h", "int5Ah", "int5Bh", "int5Ch", "int5Dh", "int5Eh", "int5Fh",
  "int60h", "int61h", "int62h", "int63h", "int64h", "int65h", "int66h", "int67h",
  "int68h", "int69h", "int6Ah", "int6Bh", "int6Ch", "int6Dh", "int6Eh", "int6Fh",
  "int70h", "int71h", "int72h", "int73h", "int74h", "int75h", "int76h", "int77h",
  "int78h", "int79h", "int7Ah", "int7Bh", "int7Ch", "int7Dh", "int7Eh", "int7Fh",
  "int80h", "int81h", "int82h", "int83h", "int84h", "int85h", "int86h", "int87h",
  "int88h", "int89h", "int8Ah", "int8Bh", "int8Ch", "int8Dh", "int8Eh", "int8Fh",
  "int90h", "int91h", "int92h", "int93h", "int94h", "int95h", "int96h", "int97h",
  "int98h", "int99h", "int9Ah", "int9Bh", "int9Ch", "int9Dh", "int9Eh", "int9Fh",
  "intA0h", "intA1h", "intA2h", "intA3h", "intA4h", "intA5h", "intA6h", "intA7h",
  "intA8h", "intA9h", "intAAh", "intABh", "intACh", "intADh", "intAEh", "intAFh",
  "intB0h", "intB1h", "intB2h", "intB3h", "intB4h", "intB5h", "intB6h", "intB7h",
  "intB8h", "intB9h", "intBAh", "intBBh", "intBCh", "intBDh", "intBEh", "intBFh",
  "intC0h", "intC1h", "intC2h", "intC3h", "intC4h", "intC5h", "intC6h", "intC7h",
  "intC8h", "intC9h", "intCAh", "intCBh", "intCCh", "intCDh", "intCEh", "intCFh",
  "intD0h", "intD1h", "intD2h", "intD3h", "intD4h", "intD5h", "intD6h", "intD7h",
  "intD8h", "intD9h", "intDAh", "intDBh", "intDCh", "intDDh", "intDEh", "intDFh",
  "intE0h", "intE1h", "intE2h", "intE3h", "intE4h", "intE5h", "intE6h", "intE7h",
  "intE8h", "intE9h", "intEAh", "intEBh", "intECh", "intEDh", "intEEh", "intEFh",
  "intF0h", "intF1h", "intF2h", "intF3h", "intF4h", "intF5h", "intF6h", "intF7h",
  "intF8h", "intF9h", "intFAh", "intFBh", "intFCh", "intFDh", "intFEh", "intFFh"
};

const char* arch_reg_names[TRANSREG_COUNT] = {
  // Integer registers
  "rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi",
  "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
  // SSE registers
  "xmml0", "xmmh0", "xmml1", "xmmh1", "xmml2", "xmmh2", "xmml3", "xmmh3",
  "xmml4", "xmmh4", "xmml5", "xmmh5", "xmml6", "xmmh6", "xmml7", "xmmh7",
  "xmml8", "xmmh8", "xmml9", "xmmh9", "xmml10", "xmmh10", "xmml11", "xmmh11",
  "xmml12", "xmmh12", "xmml13", "xmmh13", "xmml14", "xmmh14", "xmml15", "xmmh15",
  // x87 FP/MMX
  "fptos", "fpsw", "fptags", "fpstack", "tr4", "tr5", "tr6", "ctx",
  // Special
  "rip", "flags", "iflags", "selfrip","nextrip", "ar1", "ar2", "zero",
  // The following are ONLY used during the translation and renaming process:
  "tr0", "tr1", "tr2", "tr3", "tr4", "tr5", "tr6", "tr7",
  "zf", "cf", "of", "imm", "mem", "tr8", "tr9", "tr10",
};

const char* datatype_names[DATATYPE_COUNT] = {
  "int", "float", "vec-float",
  "double", "vec-double", 
  "vec-8bit", "vec-16bit", 
  "vec-32bit", "vec-64bit", 
  "vec-128bit"
};

extern const char* datatype_names[DATATYPE_COUNT];
/*
 * Convert a condition code (as in jump, setcc, cmovcc, etc) to
 * the one or two architectural registers last updated with the
 * flags that uop will test.
 */
const CondCodeToFlagRegs cond_code_to_flag_regs[16] = {
  {0, REG_of,   REG_of},   // of:               jo          (rb only)
  {0, REG_of,   REG_of},   // !of:              jno         (rb only)
  {0, REG_cf,   REG_cf},   // cf:               jb jc jnae  (rb only)
  {0, REG_cf,   REG_cf},   // !cf:              jnb jnc jae (rb only)
  {0, REG_zf,   REG_zf},   // zf:               jz je       (ra only)
  {0, REG_zf,   REG_zf},   // !zf:              jnz jne     (ra only)
  {1, REG_zf,   REG_cf},   // cf|zf:            jbe jna
  {1, REG_zf,   REG_cf},   // !cf & !zf:        jnbe ja
  {0, REG_zf,   REG_zf},   // sf:               js          (ra only)
  {0, REG_zf,   REG_zf},   // !sf:              jns         (ra only)
  {0, REG_zf,   REG_zf},   // pf:               jp jpe      (ra only)
  {0, REG_zf,   REG_zf},   // !pf:              jnp jpo     (ra only)
  {1, REG_zf,   REG_of},   // sf != of:         jl jnge (*)
  {1, REG_zf,   REG_of},   // sf == of:         jnl jge (*)
  {1, REG_zf,   REG_of},   // zf | (sf != of):  jle jng (*)
  {1, REG_zf,   REG_of},   // !zf & (sf == of): jnle jg (*)
  //
  // (*) Technically three flags are involved in the comparison here,
  // however as pursuant to the ZAPS trick, zf/af/pf/sf are always
  // either all written together or not written at all. Hence the
  // last writer of SF will also deliver ZF in the same result.
  //
};

const char* cond_code_names[16] = { "o", "no", "c", "nc", "e", "ne", "be", "nbe", "s", "ns", "p", "np", "l", "nl", "le", "nle" };
const char* x86_flag_names[32] = {
  "c", "X", "p", "W", "a", "B", "z", "s", "t", "i", "d", "o", "iopl0", "iopl1", "nt", "0",
  "rf", "vm", "ac", "vif", "vip", "id", "22", "23", "24", "25", "26", "27", "28", "29", "30", "31"
};

const char* setflag_names[SETFLAG_COUNT] = {"z", "c", "o"};
const W16 setflags_to_x86_flags[1<<3] = {
  0       | 0       | 0,         // 000 = n/a
  0       | 0       | FLAG_ZAPS, // 001 = Z
  0       | FLAG_CF | 0,         // 010 =  C
  0       | FLAG_CF | FLAG_ZAPS, // 011 = ZC
  FLAG_OF | 0       | 0,         // 100 =   O
  FLAG_OF | 0       | FLAG_ZAPS, // 101 = Z O
  FLAG_OF | FLAG_CF | 0,         // 110 =  CO
  FLAG_OF | FLAG_CF | FLAG_ZAPS, // 111 = ZCO
};

stringbuf& operator <<(stringbuf& sb, const TransOpBase& op) {
  // e.g. add.8, add.16, add.32, add.64
  static const char* size_names[4] = {".8", ".16", ".32", ".64"};
  // e.g. addfp, addfv, addfd, xxx
  static const char* fptype_names[4] = {"p", "v", "d", "d"};

  bool ld = isload(op.opcode);
  bool st = isstore(op.opcode);
  bool fp = (isclass(op.opcode, OPCLASS_FP_ALU));

  stringbuf sbname;

  sbname << nameof(op.opcode);
  sbname << (fp ? fptype_names[op.size] : size_names[op.size]);

  if (isclass(op.opcode, OPCLASS_USECOND)) sbname << ".", cond_code_names[op.cond];

  if (ld|st) {
    if (op.opcode == OP_mf) {
      static const char* mf_names[4] = {"none", "st", "ld", "all"};
      sbname << '.', mf_names[op.extshift];
    }
    sbname << ((op.cond == LDST_ALIGN_LO) ? ".lo" : (op.cond == LDST_ALIGN_HI) ? ".hi" : "");
  } else if (op.opcode == OP_mask) {
    sbname << ((op.cond == 0) ? "" : (op.cond == 1) ? ".z" : (op.cond == 2) ? ".x" : ".???");
  }

  if ((ld|st) && (op.cachelevel > 0)) sbname << ".L", (char)('1' + op.cachelevel);
  if ((ld|st) && (op.locked)) sbname << ((ld) ? ".acq" : ".rel");
  if (op.internal) sbname << ".p";
  if (op.eom) sbname << ".";

  //cout << "***** " << sbname.str() << endl;
  sb << sbname, " ", arch_reg_names[op.rd];
  sb << " = ";
  if (ld|st) sb << "[";
  sb << arch_reg_names[op.ra];
  if (op.rb == REG_imm) {
    if (abs(op.rbimm) <= 32768) sb << ",", op.rbimm; else sb << ",", (void*)op.rbimm;
  } else {
    sb << ",", arch_reg_names[op.rb];
  }
  if (ld|st) sb << "]";
  if ((op.opcode == OP_mask) | (op.opcode == OP_maskb)) {
    MaskControlInfo mci(op.rcimm);
    int sh = (op.opcode == OP_maskb) ? 3 : 0;
    sb << ",[ms=", (mci.info.ms >> sh), " mc=", (mci.info.mc >> sh), " ds=", (mci.info.ds >> sh), "]";
  } else {
    if (op.rc != REG_zero) { if (op.rc == REG_imm) sb << ",", op.rcimm; else sb << ",", arch_reg_names[op.rc]; }
  }
  if ((op.opcode == OP_adda || op.opcode == OP_suba) && (op.extshift != 0)) sb << "*", (1 << op.extshift);

  if (op.setflags) {
    sb << " ";
    if (op.nouserflags) sb << "int:";
    sb << "[";
    for (int i = 0; i < SETFLAG_COUNT; i++) {
      if (bit(op.setflags, i)) sb << setflag_names[i];
    }
    sb << "] ";
  }

  if (isbranch(op.opcode)) sb << " [taken ", (void*)(Waddr)op.riptaken, ", seq ", (void*)(Waddr)op.ripseq, "]";

  return sb;
}

ostream& operator <<(ostream& os, const TransOpBase& op) {
  stringbuf sb;
  sb << op;
  os << sb.str();
  return os;
}

ostream& RIPVirtPhysBase::print(ostream& os) const {
  os << (void*)(Waddr)rip;
  return os;
}

char* regname(int r) {
  static stringbuf temp;
  assert(r >= 0);
  assert(r < 256);
  //temp.reset(); jld

  temp << 'r', r;
  return (char*) temp.str().c_str();
}

stringbuf& nameof(stringbuf& sbname, const TransOp& uop) {
  static const char* size_names[4] = {"b", "w", "d", ""};
  static const char* fptype_names[4] = {"ss", "ps", "sd", "pd"};
  static const char* mask_exttype[4] = {"", "zxt", "sxt", "???"};

  int op = uop.opcode;

  bool ld = isload(op);
  bool st = isstore(op);
  bool fp = (isclass(op, OPCLASS_FP_ALU));

  sbname << nameof(op);

  if ((op != OP_maskb) & (op != OP_mask))
    sbname << (fp ? fptype_names[uop.size] : size_names[uop.size]);
  else sbname << ".", mask_exttype[uop.cond];

  if (isclass(op, OPCLASS_USECOND))
    sbname << ".", cond_code_names[uop.cond];

  if (ld|st) {
    sbname << ((uop.cond == LDST_ALIGN_LO) ? ".low" : (uop.cond == LDST_ALIGN_HI) ? ".high" : "");
    if (uop.cachelevel > 0) sbname << ".L", (char)('1' + uop.cachelevel);
  }

  if (uop.internal) sbname << ".p";
  
  return sbname;
}

ostream& operator <<(ostream& os, const UserContext& arf) {
  static const int width = 4;
  foreach (i, ARCHREG_COUNT) {
    os << "  ", padstring(arch_reg_names[i], -6), " 0x", hexstring(arf[i], 64), "  ";
    if ((i % width) == (width-1)) os << endl;
  }
  return os;
}

ostream& operator <<(ostream& os, const IssueState& state) {
  os << "  rd 0x" << hexstring(state.reg.rddata, 64) << " (" << flagstring(state.reg.rdflags) << ") << sfrd " << state.st << " (exception " << exception_name(state.reg.rddata) << ")" << endl;
  return os;
}

stringbuf& operator <<(stringbuf& os, const SFR& sfr) {
  if (sfr.invalid) {
    os << "< Invalid: fault 0x", hexstring(sfr.data, 8), " > ";
  } else {
    os << bytemaskstring((const byte*)&sfr.data, sfr.bytemask, 8), " ";
  }

  os << "@ 0x", hexstring(sfr.physaddr << 3, 64), " for memid tag ", sfr.tag;
  return os;
}

stringbuf& print_value_and_flags(stringbuf& sb, W64 value, W16 flags) {
  stringbuf flagsb;
  if (flags & FLAG_ZF) flagsb << "z";
  if (flags & FLAG_PF) flagsb << "p";
  if (flags & FLAG_SF) flagsb << "s";
  if (flags & FLAG_CF) flagsb << "c";
  if (flags & FLAG_OF) flagsb << "o";

  if (flags & FLAG_INV)
    sb << " < ", padstring(exception_name(LO32(value)), -14), " >";
  else sb << " 0x", hexstring(value, 64);
  sb << "| ", flagsb.str();
  return sb;
}

ostream& operator <<(ostream& os, const PageFaultErrorCode& pfec) {
  os << "[";
  os << (pfec.p ? " present" : " not-present");
  os << (pfec.rw ? " write" : " read");
  os << (pfec.us ? " user" : " kernel");
  os << (pfec.rsv ? " reserved-bits-set" : "");
  os << (pfec.nx ? " execute" : "");
  os << " ]";

  return os;
}

ostream& operator <<(ostream& os, const SegmentDescriptor& seg) {
  os << "base ", hexstring(seg.getbase(), 32), ", limit ", hexstring(seg.getlimit(), 32),
    ", ring ", seg.dpl;
  os << ((seg.s) ? " sys" : " usr");
  os << ((seg.l) ? " 64bit" : "      ");
  os << ((seg.d) ? " 32bit" : " 16bit");
  os << ((seg.g) ? " g=4KB" : "      ");

  if (!seg.p) os << "not present";

  return os;
}

ostream& operator <<(ostream& os, const SegmentDescriptorCache& seg) {
  os << "0x", hexstring(seg.selector, 16), ": ";

  os << "base ", hexstring(seg.base, 64), ", limit ", hexstring(seg.limit, 64), ", ring ", seg.dpl, ":";
  os << ((seg.supervisor) ? " sys" : " usr");
  os << ((seg.use64) ? " 64bit" : "      ");
  os << ((seg.use32) ? " 32bit" : "      ");

  if (!seg.present) os << " (not present)";

  return os;
}

ostream& operator <<(ostream& os, const Context& ctx) {
  static const int arfwidth = 4;

  os << "Context:" << endl;

  os << "  Segment Registers:" << endl;
  os << "    cs " << ctx.seg[SEGID_CS] << endl;
  os << "    ss " << ctx.seg[SEGID_SS] << endl;
  os << "    ds " << ctx.seg[SEGID_DS] << endl;
  os << "    es " << ctx.seg[SEGID_ES] << endl;
  os << "    fs " << ctx.seg[SEGID_FS] << endl;
  os << "    gs " << ctx.seg[SEGID_GS] << endl;
  os << "  FPU:" << endl;
  os << "    FP Control Word: 0x" << hexstring(ctx.fpcw, 32) << endl;
  os << "    MXCSR:           0x" << hexstring(ctx.mxcsr, 32) << endl;
  os << "  internal_eflags:    0x" << hexstring(ctx.internal_eflags, 32) << endl;

  return os;
}

bool uopWritesDestinationRegister(TransOp &uop) {
  if (isstore(uop.opcode)){
    return false;
  } else if (isclass(uop.opcode, (1 << 10))){ // memory fence--noop for now
    return false;
  }
  return true;
}

