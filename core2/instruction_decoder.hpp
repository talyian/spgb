#include "base.hpp"
#include "wasm_host.hpp"

struct InstructionDecoder {
  u8 * buf;
  u32 buflen;
  u32 pc;
  bool error = 0;
  InstructionDecoder(u8 * buf, u32 len, u32 pos): buf(buf), buflen(len), pc(pos) { }
  void EOF() { log(__FUNCTION__); error = 1; }
  u16 IMM16() { u16 v = buf[pc++]; v = v + 256 * buf[pc++]; return v; }
  u8 Imm8() { return buf[pc++]; }
  void decode() {
    if (pc >= buflen) return EOF();
    _log((u16)pc);
    u16 op = buf[pc++];
    if (op == 0xcb) op = 0x100 + buf[pc++];
    switch(op) {
    #include "generated_instruction_decode_table.inc"
    default:
      log("unknown op", op);
      error = 1;
    }
  }
};
