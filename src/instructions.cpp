#include "instructions.hpp"

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
  case Value16::REG16: _log(o.reg); break;
  case Value16::IMM16: _logx16(o.value); break;
  case Value16::SP_d8: _log("SP+"); _log((i8)o.offset); break;
  default: _log("value16"); break;
  }
}
void _log(Value8 o) {
  char io_buf[10] = "IO:";
  
  switch(o.type) {
  case Value8::REG8: _log(o.reg); break;
  case Value8::IMM8: _log(o.value); break;
  case Value8::IoReg8: {
    const char * r = name_of(o.reg);
    for(char * b = io_buf + 3; *r; r++, b++) *b = *r;
    _log(io_buf); break; }
  case Value8::IoImm8: {
    io_buf[3] = (o.value >> 4)["0123456789ABCDEF"];
    io_buf[4] = (o.value & 0xF)["0123456789ABCDEF"];
    io_buf[5] = 0;
    _log(io_buf); break; }
  case Value8::Ld8Dec: {
    char *b = io_buf; *b++ = '*';
    const char * r = name_of(o.reg16);
    for(; *r; r++, b++) *b = *r;
    *b++ = '-'; *b++ = '-';
    _log(io_buf); break; }
  case Value8::Ld8Inc: {
    char *b = io_buf; *b++ = '*';
    const char * r = name_of(o.reg16);
    for(; *r; r++, b++) *b = *r;
    *b++ = '+'; *b++ = '+';
    _log(io_buf); break; }
  case Value8::Ld8Reg: {    
    char *b = io_buf; *b++ = '*';
    const char * r = name_of(o.reg16);
    for(; *r; r++, b++) *b = *r;
    _log(io_buf); break; }
  case Value8::Ld8Imm: {
    char *b = io_buf; *b++ = '*';
    u16 addr = o.addr;
    b = io_buf + 6;
    *--b = 0;
    for(int i=0; i<4; i++) { 
      *--b = (addr % 0x10)["0123456789ABCDEF"];
      addr /= 0x10;
    }
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

}
using logs::_log;
