#pragma once
#include "base.hpp"
#include "wasm_host.hpp"

enum class Conditions : u8{
  C, Z, T,
  NC, NZ,
};

enum Register :u8 {
  SP, BC, DE, HL, AF,
  A, B, C, D, E, F,
  H, L
};
enum class Register16 : u8 { SP = SP, BC=BC, DE=DE, HL=HL, AF=AF };
enum class Register8 : u8 { A=A, B=B, C=C, D=D, E=E, F=F, H=H, L=L };
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
  } type;
  union {
    u8 value;
    Register8 reg;
  };
  Value8(u8 v) : type(IMM8), value(v) { }
  Value8(Register8 v) : type(REG8), reg(v) { }
};

struct Value16 {
  enum {
    REG16 = OperandType::REG16,
    IMM16 = OperandType::IMM16,
  } type;
  union { 
    u16 value;
    Register16 reg;
  };
  Value16(u16 v) : type(IMM16), value(v) { }
  Value16(Register16 v) : type(REG16), reg(v) { }
};

namespace logs {
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
void _log(Register o) {
  switch(o) {
#define CASE(X) case X: _log(#X); break;
  CASE(SP)
  CASE(BC)
  CASE(DE)
  CASE(HL)
  CASE(AF)
  CASE(A)
  CASE(B)
  CASE(C)
  CASE(D)
  CASE(E)
  CASE(F)
  CASE(H)
  CASE(L)
#undef CASE
  default: _log("??"); break;
  }
}
void _log(Operand o) {
  if (o.type == IMM8) _log(o.data.val8);
  else if (o.type == IMM16) _log(o.data.val16);
  else if (o.type == REG16) _log((Register) o.data.val8);
  else if (o.type == REG8) _log((Register) o.data.val8);
  else if (o.type == IO_IMM8) { _log("IO:"); _log(o.data.val8); }
  else if (o.type == IO_REG) { _log("IO:"); _log((Register) o.data.val8); }
  else if (o.type == Load_REG16) { _log("["); _log((Register) o.data.val8); _log("]"); }
  else if (o.type == Load_IMM16) { _log("["); _log(o.data.val16); _log("]"); }
  else if (o.type == Inc_REG16) { _log("["); _log((Register) o.data.val8); _log("]++"); }
  else if (o.type == Dec_REG16) { _log("["); _log((Register) o.data.val8); _log("]--"); }
  else
    _log("????????????????????????????????????????op");
}
}
using logs::_log;
