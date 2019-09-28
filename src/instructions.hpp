#pragma once
#include "base.hpp"
#include "wasm_host.hpp"


enum class Conditions : u8 { C, Z, T, NC, NZ, };
enum class Register16 : u8 { SP, BC, DE, HL, AF };
enum class Register8 : u8 { A, B, C, D, E, F, H, L };

struct Value8 {
  enum {
    REG8, // 8-bit register value
    IMM8, // 8-bit immediate value
    
    Ld8Imm, // 8-bit load immediate value
    Ld8Reg, // 8-bit load address in register
    Ld8Dec, // 8-bit load-and-decrement register
    Ld8Inc, // 8-bit load-and-increment register
    
    IoReg8, // $FF00+(register)
    IoImm8, // $FF00+immediate
  } type;
  union {
    u8 value;
    Register8 reg;
    u16 addr;
    Register16 reg16; // for ld-inc/ld-dec
  };
  Value8() = default;
  Value8(u8 v) : type(IMM8), value(v) { }
  Value8(Register8 v) : type(REG8), reg(v) { }
  static Value8 _Inc(Register16 r) { Value8 v; v.type = Ld8Inc; v.reg16 = r; return v; }
  static Value8 _Dec(Register16 r) { Value8 v; v.type = Ld8Dec; v.reg16 = r; return v; }
  static Value8 _Load(Register16 r) { Value8 v; v.type = Ld8Reg; v.reg16 = r; return v; }
  static Value8 _Load(u16 r) { Value8 v; v.type = Ld8Imm; v.addr = r; return v; }
  static Value8 _Io(u8 val) { Value8 v; v.type = IoImm8; v.value = val; return v; }
  static Value8 _Io(Register8 r) { Value8 v; v.type = IoReg8; v.reg = r; return v; }
};

struct Value16 {
  enum {
    REG16,
    IMM16,
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
  void _log(Register16 r);
  void _log(Register8 r);
  void _log(Value16 o);
  void _log(Value8 o);
  void _log(Conditions o);
}
using logs::_log;
