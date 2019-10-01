#include "base.hpp"
#include "instructions.hpp"

struct Assembler {
  u8 * buf;
  u16 pc;
  u16 buf_len;

  void _write(u16 v) { buf[pc++] = v >> 8; buf[pc++] = v; }
  void _write(u8 v) { buf[pc++] = v; }
  
  // ENTRY0(0x000, 01,  4, -1, "----", NOP    , 0, 0)
  void NOP() { buf[pc++] = 0x00; }
  // ENTRY2(0x001, 03, 12, -1, "----", LD16   , BC(), Imm16())
  // ENTRY2(0x002, 01,  8, -1, "----", LD8    , Load8(BC()), A())
  void LD8(Value8 o, Value8 v) {
    // LD8(r8, r8)
#define LOOP_LEFT(F) \
    F(B(), 0x00) \
    F(C(), 0x08) \
    F(D(), 0x10) \
    F(E(), 0x18) \
    F(H(), 0x20) \
    F(L(), 0x28) \
    F(Load8(HL()), 0x30) \
    F(A(), 0x38)
    
    
    // LD8(x,  A)
    if (v == Register8::A) {
      if (o.type == Value8::Ld8Reg && o.reg16 == Register16::BC) { buf[pc++] = 0x02; return; }
      if (o.type == Value8::Ld8Reg && o.reg16 == Register16::DE) { buf[pc++] = 0x12; return; }
      if (o.type == Value8::Ld8Inc && o.reg16 == Register16::HL) { buf[pc++] = 0x22; return; }
      if (o.type == Value8::Ld8Dec && o.reg16 == Register16::HL) { buf[pc++] = 0x32; return; }
      if (o == Register8::B) { buf[pc++] = 0x47; return; }
      if (o == Register8::C) { buf[pc++] = 0x4f; return; }
      if (o == Register8::D) { buf[pc++] = 0x57; return; }
      if (o == Register8::E) { buf[pc++] = 0x5f; return; }
      if (o == Register8::H) { buf[pc++] = 0x67; return; }
      if (o == Register8::L) { buf[pc++] = 0x6f; return; }
      if (o.type == Value8::Ld8Reg && o.reg16 == Register16::HL) { buf[pc++] = 0x77; return; }
      if (o == Register8::A) { buf[pc++] = 0x7f; return; }
      if (o.type == Value8::IoImm8) { buf[pc++] = 0xE0; buf[pc++] = o.value; return; }
      if (o.type == Value8::IoReg8 && o.reg == Register8::C) { buf[pc++] = 0xE2; return; }
      if (o.type == Value8::Ld8Imm) { buf[pc++] = 0xea; _write(o.addr); return; }
    }

#define LOOP_LEFT(LL) \
    LL(Register::B, 0x40);                      \
    LL(Register::B, 0x40);                      \
    LL(Register::B, 0x40);                      \
    
#define LOOP(LD)            \
      LD(Register8::B,0x0); \
      LD(Register8::C,0x8); \
      LD(Register8::D,0x10); \
      LD(Register8::E,0x18); \
      LD(Register8::H,0x20); \
      LD(Register8::L,0x28); \
      LD(Value8::_Load(Register16::HL), 0x30); \
      LD(Register8::A,0x38);
    
    if (v == Register8::B) {
#define LD(R, ADDR) if (o == R) { buf[pc++] = 0x40 + ADDR; buf[pc++] = v.value; return; }
LOOP(LD)
#undef LD
    }
    if (v == Register8::C) {
#define LD(R, ADDR) if (o == R) { buf[pc++] = 0x41 + ADDR; buf[pc++] = v.value; return; }
LOOP(LD)
#undef LD
    }
    if (v == Register8::D) {
#define LD(R, ADDR) if (o == R) { buf[pc++] = 0x42 + ADDR; buf[pc++] = v.value; return; }
LOOP(LD)
#undef LD
    }
    if (v == Register8::E) {
#define LD(R, ADDR) if (o == R) { buf[pc++] = 0x43 + ADDR; buf[pc++] = v.value; return; }
LOOP(LD)
#undef LD
    }
 
    if (v.type == Value8::IMM8) {
      if (o == Register8::B) { buf[pc++] = 0x06; buf[pc++] = v.value; return; }
      if (o == Register8::C) { buf[pc++] = 0x0e; buf[pc++] = v.value; return; }
      if (o == Register8::D) { buf[pc++] = 0x16; buf[pc++] = v.value; return; }
      if (o == Register8::E) { buf[pc++] = 0x1e; buf[pc++] = v.value; return; }
      if (o == Register8::H) { buf[pc++] = 0x26; buf[pc++] = v.value; return; }
      if (o == Register8::L) { buf[pc++] = 0x2e; buf[pc++] = v.value; return; }
      if (o.type == Value8::Ld8Reg && o.reg16 == Register16::HL) { buf[pc++] = 0x36; buf[pc++] = v.value; return; }
      if (o == Register8::A) { buf[pc++] = 0x3e; buf[pc++] = v.value; return; }
    }
  }
  // ENTRY1(0x003, 01,  8, -1, "----", INC    , BC(), 0)
  // ENTRY1(0x004, 01,  4, -1, "Z0H-", INC    , B(), 0)
  // ENTRY1(0x005, 01,  4, -1, "Z1H-", DEC    , B(), 0)
  // ENTRY2(0x006, 02,  8, -1, "----", LD8    , B(), Imm8())
  // ENTRY0(0x007, 01,  4, -1, "000C", RLCA   , 0, 0)
  // ENTRY2(0x008, 03, 20, -1, "----", LD16   , Load16(Imm16()), SP())
  // ENTRY2(0x009, 01,  8, -1, "-0HC", ADD    , HL(), BC())
  
};
