// main compilation unit

#include "instruction_decoder.cpp"
#include "instruction_runner.cpp"
#include "instructions.cpp"
#include "emulator/mmu.cpp"
#include "emulator/ppu.cpp"
#include "emulator.cpp"

namespace logs {
  void _log(Reg16 v) { _logx16((u16)v); };
  void _log(str s) { _logs((const char*)s.data, s.size); }
  void _log(u8 v) { _logx8(v); }
  void _log(u16 v) { _logx16(v); }
  void _log(u32 v) { _logx32(v); }
  void _log(i32 v) { _logf(v); }
  void _log(double f) { _logf(f); }
  void _log(const char* s) { _logs(s, sslen(s)); }
  void _log(void* s) { _logp(s); }
}


