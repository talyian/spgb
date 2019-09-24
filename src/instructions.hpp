#pragma once
#include "base.hpp"
#include "wasm_host.hpp"

enum class Conditions : u8{
  C, Z, T,
  NC, NZ,
};

enum class Register16 : u8 { SP, BC, DE, HL, AF };
enum class Register8 : u8 { A, B, C, D, E, F, H, L };
const char * name_of(Register16 r) {
  switch(r) {
  case Register16::BC: return("BC"); break;
  case Register16::DE: return("DE"); break;
  case Register16::HL: return("HL"); break;
  case Register16::AF: return("AF"); break;
  case Register16::SP: return("SP"); break;
  }    
}
const char * name_of(Register8 r) {
  switch(r) {
  case Register8::A: return("A"); break;
  case Register8::B: return("B"); break;
  case Register8::C: return("C"); break;
  case Register8::D: return("D"); break;
  case Register8::E: return("E"); break;
  case Register8::F: return("F"); break;
  case Register8::H: return("H"); break;
  case Register8::L: return("L"); break;
  }
}
enum OperandType {
  REG8, REG16,
  IMM8, IMM16,
  IO_REG,
  IO_IMM8,
  Load_REG16,
  Load_IMM16,
  Inc_REG16,
  Dec_REG16,
};

struct Operand {
  OperandType type;
  union D {
    u8 val8;
    u16 val16;
    D(decltype(val8) v) : val8(v) { }
    D(decltype(val16) v) : val16(v) { }
  } data;
};

struct Value8 {
  enum {
    REG8 = OperandType::REG8,
    IMM8 = OperandType::IMM8,
    LdDecReg8,
    LdIncReg8,
    LdReg8,
    Ld8,
    IO_R8,
    IO_I8,
  } type;
  union {
    u8 value;
    u16 addr;
    Register8 reg;
    Register16 reg16; // for ld-inc/ld-dec
  };
  Value8() = default;
  Value8(u8 v) : type(IMM8), value(v) { }
  Value8(Register8 v) : type(REG8), reg(v) { }
};

struct Value16 {
  enum {
    REG16 = OperandType::REG16,
    IMM16 = OperandType::IMM16,
    SP_d8
  } type;
  union {
    u8 offset;
    u16 value;
    Register16 reg;
  };
  static Value16 SP_offset(u8 v) { Value16 k; k.type = SP_d8; k.offset = v; return k; }
  Value16() = default;
  Value16(u16 v) : type(IMM16), value(v) { }
  Value16(Register16 v) : type(REG16), reg(v) { }
};

namespace logs {
void _log(Register16 r) {
  switch(r) {
  case Register16::AF: _log("AF"); break;
  case Register16::BC: _log("BC"); break;
  case Register16::DE: _log("DE"); break;
  case Register16::HL: _log("HL"); break;
  case Register16::SP: _log("SP"); break;
  }
}
void _log(Register8 r) { _log(name_of(r)); }
void _log(Value16 o) {
  switch(o.type) {
  case REG16: _log(o.reg); break;
  case IMM16: _logx16(o.value); break;
  default: _log("value16"); break;
  }
}
void _log(Value8 o) {
  char io_buf[10] = "IO:";
  
  switch(o.type) {
  case REG8: _log(o.reg); break;
  case IMM8: _log(o.value); break;
  case Value8::IO_R8: {
    const char * r = name_of(o.reg);
    for(char * b = io_buf + 3; *r; r++, b++) *b = *r;
    _log(io_buf); break; }
  case Value8::IO_I8: {
    io_buf[3] = (o.value >> 4)["0123456789ABCDEF"];
    io_buf[4] = (o.value & 0xF)["0123456789ABCDEF"];
    io_buf[5] = 0;
    _log(io_buf); break; }
  case Value8::LdDecReg8: {
    char *b = io_buf; *b++ = '*';
    const char * r = name_of(o.reg16);
    for(; *r; r++, b++) *b = *r;
    *b++ = '-'; *b++ = '-';
    _log(io_buf); break; }
  case Value8::LdIncReg8: {
    char *b = io_buf; *b++ = '*';
    const char * r = name_of(o.reg16);
    for(; *r; r++, b++) *b = *r;
    *b++ = '+'; *b++ = '+';
    _log(io_buf); break; }
  case Value8::LdReg8: {    
    char *b = io_buf; *b++ = '*';
    const char * r = name_of(o.reg16);
    for(; *r; r++, b++) *b = *r;
    _log(io_buf); break; }
  default: _log("value8"); break;
  }
}
void _log(Conditions o) {
  switch(o) {
  #define CASE(X) case Conditions::X: _log(#X); break;
      CASE(C)
      CASE(T)
      CASE(Z)
      CASE(NC)
      CASE(NZ)
  #undef CASE
  default: _log("?Cond?"); break;
  }
}

void _log(Operand o) {
  if (o.type == IMM8) _log(o.data.val8);
  else if (o.type == IMM16) _log(o.data.val16);
  else if (o.type == REG16) _log((Register16) o.data.val8);
  else if (o.type == REG8) _log((Register8) o.data.val8);
  else if (o.type == IO_IMM8) { _log("IO:"); _log(o.data.val8); }
  else if (o.type == IO_REG) { _log("IO:"); _log((Register8) o.data.val8); }
  else if (o.type == Load_REG16) { _log("["); _log((Register16) o.data.val8); _log("]"); }
  else if (o.type == Load_IMM16) { _log("["); _log(o.data.val16); _log("]"); }
  else if (o.type == Inc_REG16) { _log("["); _log((Register16) o.data.val8); _log("]++"); }
  else if (o.type == Dec_REG16) { _log("["); _log((Register16) o.data.val8); _log("]--"); }
  else _log("????????????????????????????????????????op");
}
}
using logs::_log;
