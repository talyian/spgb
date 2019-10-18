// main compilation unit
#include "platform.hpp"
#include "system/mmu.cpp"
#include "system/ppu.cpp"
#include "emulator.cpp"

namespace logs {
  void _log(CPU::Reg16 v) { _logx16((u16)v); };
  void _log(str s) { _logs((const char*)s.data, s.size); }
  void _log(u8 v) { _logx8(v); }
  void _log(u16 v) { _logx16(v); }
  void _log(u32 v) { _logx32(v); }
  void _log(i32 v) { _logf(v); }
  void _log(double f) { _logf(f); }
  void _log(const char* s) { _logs(s, sslen(s)); }
  void _log(void* s) { _logp(s); }
}

// system/cpu.hpp :: CPU::Reg16
CPU::Reg16 CPU::Reg16::operator-- () {
  u16 v = (u16) *this;
  CPU::Reg16 r = v - (u16)1;
  h = r.h;
  l = r.l;
  return *this;
}
CPU::Reg16 CPU::Reg16::operator++ (int) {
  u16 u = (u16) *this;
  CPU::Reg16 r = u + (u16)1;
  h = r.h;
  l = r.l;
  return u;
}
CPU::Reg16 CPU::Reg16::operator-- (int) {
  u16 u = (u16) *this;
  CPU::Reg16 r = u - (u16)1;
  h = r.h;
  l = r.l;
  return u;
}

