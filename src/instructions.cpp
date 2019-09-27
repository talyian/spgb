#include "instructions.hpp"

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
  case Value8::Ld8: {
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
