// Instructions
// REG8 / REG16 - data types for registers
// Val8 / Val15 - data types for instruction operands
// OpParser - decodes opcodes and calls instructions on a templated Executor
// OpPrinter - simple Executor that prints out text for an instruction
#pragma once
#include "registers.hh"
#include "memory.hh"

#include <cstdint>
typedef uint8_t u8;
typedef uint16_t u16;

// Register Types
enum class REG8 { A, B, C, D, E, F, H, L };
enum class REG16 { BC, DE, HL, AF, SP, PC };

// X macros for looping over all 8-bit and 16-bit registers
#define FORALL_REG8(SS) SS(A) SS(B) SS(C) SS(D) SS(E) SS(F) SS(H) SS(L)
#define FORALL_REG16(SS) SS(BC) SS(DE) SS(HL) SS(AF) // SS(SP) SS(PC)

// An 8-bit operand can be any of:
//  - an immediate 8-bit value
//  - an 8-bit register
//  - 16-bit value as a pointer
//  - 8-bit IO port
struct Val8 {
  enum { Reg = 0x34, Val = 0x35, PtrN = 0x36, PtrR = 0x37, IoN, IoR } type;
  union {
    REG8 r;
    u8 value;
    u16 ptr_n;
    REG16 ptr_r;
    u8 io_n;
    REG8 io_r;
  } value;

  u8 get(Registers &reg, Memory &mem);
  void set(Registers &reg, Memory &mem, Val8 source);
  Val8(u8 val) { type = Val; value.value = val; }
  Val8(REG8 r) { type = Reg; value.r = r; }
};

// A 16-bit operand can be any of the following:
//  - a 16-bit register
//  - a 16-bit immediate value
//  - a pointer to a 16-bit value - immediate address
//  - pointer to 16-bit value - register (i.e. HL)
struct Val16 {
  enum { Reg, Val, PtrN, PtrR } type;
  union {
    REG16 r;
    u16 value;
    u16 ptr_n;
    REG16 ptr_r;
  } value;
  u16 get(Registers &reg, Memory &mem);
  void set(Registers &reg, Memory &mem, Val16 source);
  Val16(u16 val) { type = Val; value.value=val; }
  Val16(REG16 r) { type = Reg; value.r = r; }
};

enum class Cond { ALWAYS, Z, NZ, C, NC };
void show(Cond c);
void show(Val16 v);
void show(Val8 v);

// The meat of the OpParser is the file opcodes.cc
// which maps all 512 opcodes to an instruction
template<class Executor>
struct OpParser {
  Registers &registers;
  Memory &mem;
  Executor &ii;

  OpParser(Registers &registers, Memory &mem, Executor &ii)
  : registers(registers), mem(mem), ii(ii)  { }

  // using enum REG8, REG16
  #define SS(X) REG8 X = REG8::X;
  FORALL_REG8(SS)
  #undef SS
  #define SS(X) REG16 X = REG16::X;
  FORALL_REG16(SS)
  SS(SP) SS(PC)
  #undef SS

  Val8 Load(REG16 x) {
    Val8 v(0); v.type=Val8::PtrR; v.value.ptr_r=x; return v; }
  Val8 Load(u16 x) {
    Val8 v(0); v.type=Val8::PtrN; v.value.ptr_n=x; return v;
  }
  Val8 IO(REG8 x) { Val8 v(0); v.type=Val8::IoR; v.value.io_r=x; return v; }
  Val8 IO(u8 x) { Val8 v(0); v.type=Val8::IoN; v.value.io_n=x; return v; }
  Val16 Load16(REG16 x) { Val16 v(0); v.type=Val16::PtrR; v.value.ptr_r = x; return v; }
  Val16 Load16(u16 x) { Val16 v(0); v.type=Val16::PtrN; v.value.ptr_n = x; return v; }
  u8 read8() { return mem[registers.PC++]; }
  u16 read16() { registers.PC += 2; return mem[registers.PC - 2] + mem[registers.PC - 1] * 256; }

  // The parser decodes a single instruction and feeds it to a given interpreter
  void Step() {
    ii.timer = 0;
    // the saved PC is useful for "start of instruction" displays
    registers._PC = registers.PC;
    u16 op = read8();
    if (op == 0xCB) { op = 0x100 | read8(); }
// some shorthand macros used by opcodes.cc
#define OP(CMD, op_len, op_time, flags) {ii.CMD;ii.timer+=op_time;break;}
#define OP_RAW(BLOCK, op_len, op_time, flags) {BLOCK;ii.timer+=op_time;break;}
#define CT Cond::ALWAYS
#define CZ Cond::Z
#define CNZ Cond::NZ
#define CC Cond::C
#define CNC Cond::NC
#define r8 abort(5) // TODO: error
#define I8 (int8_t)read8()
#define U8 read8()
#define U16 read16()
    switch(op) {
    #include "opcodes.cc"
    default:
      fprintf(stderr, "Unknown op %x\n",op);
      abort();
    }
#undef OP
#undef OP_RAW
#undef CT
#undef CZ
#undef CNZ
#undef CC
#undef CNC
#undef r8
#undef I8
#undef U8
#undef U16
  }
};

struct OpPrinter {
  uint64_t timer = 0;
  u16 pc, _pc;

  void NOP() { pp("NOP"); }
  void HALT() { pp("HALT"); }

  void LD(Val8 dst, Val8 src) { pp("LD", dst, src); }
  void LD(Val16 dst, Val16 src) { pp("LD", dst, src); }

  // stack operations
  void PUSH(Val16 val) { pp("PUSH", val); }
  void POP(Val16 addr) { pp("POP", addr); }

  // jumps
  void JP(Cond cond, Val16 dst) { pp("JP", cond, dst); }
  void JR(Cond cond, Val8 offset) { pp("JR", cond, offset); }
  void RET(Cond cond) { pp("RET", cond); }
  void CALL(Cond cond, Val16 target) { pp("CALL", cond, target); }
  void RST(u8 val) { pp("RST", val); }
  // tests
  void CP(Val8 val)   { pp("CP", val); }
  void INC(Val8 dst)  { pp("INC", dst); }
  void INC(Val16 dst) { pp("INC", dst); }
  void DEC(Val8 dst)  { pp("DEC", dst); }
  void DEC(Val16 dst) { pp("DEC", dst); }

  // bitwise
  void RR(Val8 val)   { pp("RR", val); }
  void RRC(Val8 val)  { pp("RRC", val); }
  void RL(Val8 val)   { pp("RL", val); }
  void RLC(Val8 val)  { pp("RLC", val); }
  void SLA(Val8 val)  { pp("SLA", val); }
  void SRA(Val8 val)  { pp("SRA", val); }
  void SRL(Val8 val)  { pp("SRL", val); }
  void SWAP(Val8 val) { pp("SWAP", val); }

  // single-bit
  void RES(int bit, Val8 val) { pp("RES", bit, val); }
  void SET(int bit, Val8 val) { pp("SET", bit, val); }
  void BIT(int bit, Val8 val) { pp("BIT", bit, val); }

  // Arithmetic
  void XOR(Val8 val) { pp("XOR", val); }
  void AND(Val8 val) { pp("AND", val); }
  void OR(Val8 val) { pp("OR", val); }
  void ADD(Val8 dst, Val8 val) { pp("ADD", dst, val); }
  void ADC(Val8 dst, Val8 val) { pp("ADC", dst, val); }
  void ADD(Val16 dst, Val16 val) { pp("ADD", dst, val); }
  void SUB(Val8 val) { pp("SUB", val); }
  void SBC(Val8 val) { pp("SBC", val); }

  void DI() { pp("DI"); }
  void EI() { pp("EI"); }
  void STOP() { pp("STOP"); }
  void DAA() { pp("DAA"); }
  void CCF() { pp("CCF"); }
  void SCF() { pp("SCF"); }
  void CPL() { pp("CPL"); }

  void p(const char *s) { printf("%s", s); }
  void p(int x) { printf("%x", x); }
  void p(Val8 v) { show(v); }
  void p(Val16 v) { show(v); }
  void p(Cond c) { show(c); }

  template<class T>
  void _pp(T single) { p(single); p("\n"); }
  template<class T, class ...TS>
  void _pp(T first, TS ...rest) {
    p(first);
    p(" ");
    _pp(rest...);
  }
  template<class ...TS>
  void pp(TS ...rest) {
    printf("[%04x] ", pc);
    _pp(rest...);
  }
};
