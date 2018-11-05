#include <cstdint>
#include <cstdio>
#include <memory>
#include "unistd.h"

#include "registers.hh"
void Registers::dump() {
  printf(" A: %02hhx  \t", A);
  printf(" B: %02hhx  \t", B);
  printf(" C: %02hhx  \t", C);
  printf(" D: %02hhx  \t", D);
  printf(" E: %02hhx  \t", E);
  printf(" F: %02hhx  \n", F);
  printf("HL: %04hx\t", (u16) HL);
  printf("SP: %04hx\t", SP);
  printf("PC: %04hx\n", PC);
}

#include "main.hh"

u8 getRegister(REG8 r, Registers &rr) {
  switch(r) {
  #define F(Z) case REG8::Z: return rr.Z;
  FORALL_REG8(F)
  #undef F
  }
};
void setRegister(REG8 r, Registers &rr, u8 value) {
  switch(r) {
  #define F(Z) case REG8::Z: rr.Z = value; break;
  FORALL_REG8(F)
  #undef F
  }
}
u16 getRegister(REG16 r, Registers &rr) {
  switch(r) {
  #define F(Z) case REG16::Z: return rr.Z;
  FORALL_REG16(F)
  F(PC)
  F(SP)
  #undef F
  }
};

void setRegister(REG16 r, Registers &rr, u16 value) {
  switch(r) {
  #define F(Z) case REG16::Z: rr.Z = value; break;
  FORALL_REG16(F)
  F(PC)
  F(SP)
  #undef F
  }
}

u8 Val8::get(Registers &reg, Memory &mem)  {
  switch(type) {
  case Reg: return getRegister(value.r, reg);
  case Val: return value.value;
  case PtrN: return mem[value.ptr_n];
  case PtrR: return 0; // TODO
  case IoN: return mem[0xFF00 + value.io_n];
  case IoR: return mem[0xFF00 + getRegister(value.io_r, reg)];
  }
}

void Val8::set(Registers &reg, Memory &mem, Val8 source)  {
  u8 new_value = source.get(reg, mem);
  switch(type) {
  case Reg: setRegister(value.r, reg, new_value); break;
  case Val: abort(); // TODO
  case PtrN: mem[value.ptr_n] = new_value; break;
  case PtrR: mem[getRegister(value.ptr_r, reg)] = new_value; break;
  case IoN: mem[0xFF00 + value.io_n] = new_value; break;
  case IoR: mem[0xFF00 + getRegister(value.io_r, reg)] = new_value; break;
  }
}

u16 Val16::get(Registers &reg, __attribute__((unused)) Memory &mem) {
  switch(type) {
  case Reg: return getRegister(value.r, reg);
  case Val: return value.value;
  }
}

void Val16::set(Registers &reg, Memory &mem, Val16 source) {
  u16 new_value = source.get(reg, mem);
  switch(type) {
  case Reg: setRegister(value.r, reg, new_value); break;
  case Val: abort(); // TODO
  }
}

u8 Executor::get(Val8 a) { return a.get(reg, mem); }
u16 Executor::get(Val16 a) { return a.get(reg, mem); }
void Executor::set(Val8 a, Val8 v) { a.set(reg, mem, v); }
void Executor::set(Val16 a, Val16 v) { a.set(reg, mem, v); }

void Executor::NOP() { }
void Executor::HALT() { } // TODO

void Executor::LD(Val8 dst, Val8 src) { set(dst, get(src)); }
void Executor::LD(Val16 dst, Val16 src) { set(dst, get(src)); }

// stack operations
void Executor::PUSH(Val16 val) {
  u16 v = get(val);
  mem[reg.SP--] = v;
  mem[reg.SP--] = v >> 8;
}
u16 Executor::PEEK() {
  u16 v = 0;
  v = (v << 8) | mem[reg.SP + 1];
  v = (v << 8) | mem[reg.SP + 2];
  return v;
}
void Executor::POP(Val16 addr) {
  u16 v = 0;
  v = (v << 8) | mem[++reg.SP];
  v = (v << 8) | mem[++reg.SP];
  set(addr, v);
}

// jumps
void Executor::JP(Cond cond, Val16 dst) {
  // printf("JP %04x\n", get(dst));
  if (cond_eval(cond)) {
    reg.PC = get(dst); } }
void Executor::JR(Cond cond, Val8 offset) {
  // printf("JR ");
  // show(cond);
  // printf(" %04x\n", reg.PC + (int8_t)get(offset));
  if (cond_eval(cond)) { reg.PC += (int8_t)get(offset); }}
void Executor::RET(Cond cond) {
  printf("RET %04x\n", PEEK());
  if (cond_eval(cond)) { POP(REG16::PC); }}
void Executor::CALL(Cond cond, Val16 target) {
  printf("CALL %04x\n", get(target));
  if (cond_eval(cond)) {
    PUSH(REG16::PC);
    reg.PC = get(target);
  }}
// tests
void Executor::CP(Val8 rhs) {
  u8 a = reg.A;
  u8 b = get(rhs);
  reg.setFZ(a == b ? 1 : 0);
  reg.setFC(a < b ? 1 : 0);
  reg.setFH(0); // TODO:
  reg.setFO(1); // TODO: really??
}
void Executor::INC(Val8 dst) {
  // printf("INC ");
  // dst.show();
  // printf(" -> %x\n", get(dst) + 1);
  set(dst, get(dst) + 1); }
void Executor::INC(Val16 dst) { set(dst, get(dst) + 1); }
void Executor::DEC(Val8 dst) {
  // printf("DEC ");
  // dst.show();
  // printf(" -> %x\n", get(dst) - 1);
  set(dst, get(dst) - 1); }
void Executor::DEC(Val16 dst) {
  // printf("DEC ");
  // dst.show();
  // printf(" -> %x\n", get(dst) - 1);
  set(dst, get(dst) - 1); }

// // bitwise
void Executor::RR(Val8 val) { }
void Executor::RRC(Val8 val) { }
// 9-bit shift
void Executor::RL(Val8 v) {
  u8 val = get(v);
  u8 carry = (val >> 7) & 1;
  val = (val << 1) | (reg.FC());
  reg.setFZ(val == 0);
  reg.setFC(carry);
  reg.setFO(0);
  reg.setFH(0); // really?
  // reg.F = (val == 0 ? 0x80 : 0) | (carry ? 0x10 : 0);
  set(v, val);
}
// 8-bit shift
void Executor::RLC(Val8 v) {
  u8 val = get(v);
  u8 carry = (val >> 7) & 1;
  val = (val << 1) | carry;
  set(v, val);
  // reg.F = (val == 0 ? 0x80 : 0) | (carry ? 0x10 : 0);
  reg.setFZ(val == 0);
  reg.setFC(carry);
  reg.setFO(0);
  reg.setFH(0); // really?
}

// void SLA(Val8 val);
// void SRA(Val8 val);
// void SWAP(Val8 val);

// // single-bit
void Executor::RES(int bit, Val8 val) { set(val, get(val) & ~(1 << bit)); }
void Executor::SET(int bit, Val8 val) { set(val, get(val) | (1 << bit)); }
void Executor::BIT(int bit, Val8 rhs) {
  u8 val = get(rhs);
  u8 val2 = (val >> bit) & 1;
  reg.setFZ(val2 == 0);
  reg.setFO(0);
  reg.setFH(1); // TODO: really???
}

// // Arithmetic
void Executor::XOR(Val8 val) { reg.A ^= get(val); }
void Executor::ADD(Val8 dst, Val8 val) { set(dst, get(dst) + get(val)); }
void Executor::ADD(Val16 dst, Val16 val) { set(dst, get(dst) + get(val)); }
void Executor::SUB(Val8 val) { reg.A -= get(val); }

template<class Executor>
struct OpParser {
  Registers &registers;
  Memory &mem;
  Executor &ii;

  u8 * buf;
  size_t buflen;

  #define SS(X) REG8 X = REG8::X;
  FORALL_REG8(SS)
  #undef SS
  #define SS(X) REG16 X = REG16::X;
  FORALL_REG16(SS)
  SS(SP) SS(PC)
  #undef SS

  OpParser(Registers &registers, Memory &mem, Executor &ii, u8 * buf, size_t buflen)
  : registers(registers), mem(mem), ii(ii), buf(buf), buflen(buflen)  { }

  OpParser(Registers &registers, Memory &mem, Executor &ii, FILE * file)
    : registers(registers), mem(mem), ii(ii) {
    fseek(file, 0, SEEK_END);
    buflen = ftell(file);
    buf = (uint8_t *)malloc(buflen);
    fseek(file, 0, 0);
    fread(buf, 1, buflen, file);
  }

  Val8 Load(u16 x) { Val8 v(0); v.type=Val8::PtrN; v.value.ptr_n=x; return v; }
  Val8 Load(REG16 x) { Val8 v(0); v.type=Val8::PtrR; v.value.ptr_r=x; return v; }
  Val8 IO(u8 x) { Val8 v(0); v.type=Val8::IoN; v.value.io_n=x; return v; }
  Val8 IO(REG8 x) { Val8 v(0); v.type=Val8::IoR; v.value.io_r=x; return v; }
  // The parser decodes a single instruction and feeds it to a given interpreter
  void Step() {
    ii.timer = 0;
    u16 pc = registers.PC;
    u8 op = read8();
    switch (op) {
    case 0x00: ii.NOP(); break;
    // ALL LDs
    case 0x06: ii.LD(B, read8()); break;
    case 0x0E: ii.LD(C, read8()); break;
    case 0x11: ii.LD(DE, read16()); break;
    case 0x16: ii.LD(D, read8()); break;
    case 0x1A: ii.LD(A, Load(DE)); break;
    case 0x1E: ii.LD(E, read8()); break;
    case 0x21: ii.LD(HL, read16()); break;
    case 0x22: /* LDI (HL), A */
      ii.LD(Load(HL), A);
      ii.INC(HL);
      break;
    case 0x2E: ii.LD(L, read8()); break;
    case 0x31: ii.LD(SP, read16()); break;
    case 0x32: { ii.LD(Load(HL), A); ii.DEC(HL); break; }
    case 0x3E: ii.LD(A, read8()); break;

    case 0x40: ii.LD(B, B); break;
    case 0x41: ii.LD(B, C); break;
    case 0x42: ii.LD(B, D); break;
    case 0x43: ii.LD(B, E); break;
    case 0x44: ii.LD(B, H); break;
    case 0x45: ii.LD(B, L); break;
    case 0x46: ii.LD(B, Load(HL)); break;
    case 0x47: ii.LD(B, A); break;
    case 0x48: ii.LD(C, B); break;
    case 0x49: ii.LD(C, C); break;
    case 0x4A: ii.LD(C, D); break;
    case 0x4B: ii.LD(C, E); break;
    case 0x4C: ii.LD(C, H); break;
    case 0x4D: ii.LD(C, L); break;
    case 0x4E: ii.LD(C, Load(HL)); break;
    case 0x4F: ii.LD(C, A); break;

    case 0x50: ii.LD(D, B); break;
    case 0x51: ii.LD(D, C); break;
    case 0x52: ii.LD(D, D); break;
    case 0x53: ii.LD(D, E); break;
    case 0x54: ii.LD(D, H); break;
    case 0x55: ii.LD(D, L); break;
    case 0x56: ii.LD(D, Load(HL)); break;
    case 0x57: ii.LD(D, A); break;
    case 0x58: ii.LD(E, B); break;
    case 0x59: ii.LD(E, C); break;
    case 0x5A: ii.LD(E, D); break;
    case 0x5B: ii.LD(E, E); break;
    case 0x5C: ii.LD(E, H); break;
    case 0x5D: ii.LD(E, L); break;
    case 0x5E: ii.LD(E, Load(HL)); break;
    case 0x5F: ii.LD(E, A); break;
    case 0x60: ii.LD(H, B); break;
    case 0x61: ii.LD(H, C); break;
    case 0x62: ii.LD(H, D); break;
    case 0x63: ii.LD(H, E); break;
    case 0x64: ii.LD(H, H); break;
    case 0x65: ii.LD(H, L); break;
    case 0x66: ii.LD(H, Load(HL)); break;
    case 0x67: ii.LD(H, A); break;
    case 0x68: ii.LD(L, B); break;
    case 0x69: ii.LD(L, C); break;
    case 0x6A: ii.LD(L, D); break;
    case 0x6B: ii.LD(L, E); break;
    case 0x6C: ii.LD(L, H); break;
    case 0x6D: ii.LD(L, L); break;
    case 0x6E: ii.LD(L, Load(HL)); break;
    case 0x6F: ii.LD(L, A); break;

    case 0x70: ii.LD(Load(HL), B); break;
    case 0x71: ii.LD(Load(HL), C); break;
    case 0x72: ii.LD(Load(HL), D); break;
    case 0x73: ii.LD(Load(HL), E); break;
    case 0x74: ii.LD(Load(HL), H); break;
    case 0x75: ii.LD(Load(HL), L); break;
    case 0x76: ii.HALT(); break;
    case 0x77: ii.LD(B, A); break;
    case 0x78: ii.LD(A, B); break;
    case 0x79: ii.LD(A, C); break;
    case 0x7A: ii.LD(A, D); break;
    case 0x7B: ii.LD(A, E); break;
    case 0x7C: ii.LD(A, H); break;
    case 0x7D: ii.LD(A, L); break;
    case 0x7E: ii.LD(A, Load(HL)); break;
    case 0x7F: ii.LD(A, A); break;

      // All IO
    case 0xE0:
      // printf("Write %x\n", buf[registers.PC]);
      ii.LD(IO(read8()), A); break;
    case 0xE2:
      // printf("Write A\n");
      ii.LD(IO(C), A); break;
    case 0xEA: ii.LD(Load(read16()), A); break;
    case 0xF0:
      // printf("Read %x -> %x\n", buf[registers.PC], mem.buf[0xFF44]);
      ii.LD(A, IO(read8())); break; //
      // PUSH
    case 0xC5: ii.PUSH(BC); break;
    case 0xD5: ii.PUSH(DE); break;
    case 0xE5: ii.PUSH(HL); break;
    case 0xF5: ii.PUSH(AF); break;
      // POP
    case 0xC1: ii.POP(BC); break;
    case 0xD1: ii.POP(DE); break;
    case 0xE1: ii.POP(HL); break;
    case 0xF1: ii.POP(AF); break;
      // Returns
    case 0xC0: ii.RET(Cond::NZ); break;
    case 0xC8: ii.RET(Cond::Z); break;
    case 0xC9: ii.RET(Cond::ALWAYS); break;
    case 0xD0: ii.RET(Cond::NC); break;
    case 0xD8: ii.RET(Cond::C); break;
    case 0xD9: abort();
      // CP
    case 0xFE: ii.CP(read8()); break;
    case 0xBE: ii.CP(Load(HL)); break;
      // INC
    case 0x03: ii.INC(BC); break;
    case 0x04: ii.INC(B); break;
    case 0x0C: ii.INC(C); break;
    case 0x13: ii.INC(DE); break;
    case 0x14: ii.INC(D); break;
    case 0x1C: ii.INC(E); break;
    case 0x23: ii.INC(HL); break;
    case 0x24: ii.INC(H); break;
    case 0x2C: ii.INC(L); break;
    case 0x33: ii.INC(SP); break;
    case 0x34: ii.INC(Load(HL)); break;
    case 0x3C: ii.INC(A); break;
      // DEC
    case 0x05: ii.DEC(B); break;
    case 0x0B: ii.DEC(BC); break;
    case 0x0D: ii.DEC(C); break;
    case 0x15: ii.DEC(D); break;
    case 0x1B: ii.DEC(DE); break;
    case 0x1D: ii.DEC(E); break;
    case 0x25: ii.DEC(H); break;
    case 0x2B: ii.DEC(HL); break;
    case 0x2D: ii.DEC(L); break;
    case 0x35: ii.DEC(Load(HL)); break;
    case 0x3B: ii.DEC(SP); break;
    case 0x3D: ii.DEC(A); break;
      // ADD
    case 0x09: ii.ADD(HL, BC); break;
    case 0x19: ii.ADD(HL, DE); break;
    case 0x29: ii.ADD(HL, HL); break;
    case 0x39: ii.ADD(HL, SP); break;
    case 0x80: ii.ADD(A, B); break;
    case 0x81: ii.ADD(A, C); break;
    case 0x82: ii.ADD(A, D); break;
    case 0x83: ii.ADD(A, E); break;
    case 0x84: ii.ADD(A, H); break;
    case 0x85: ii.ADD(A, L); break;
    case 0x86: ii.ADD(A, Load(HL)); break;
    case 0x87: ii.ADD(A, A); break;
    case 0xC6: ii.ADD(A, read8()); break;
    case 0xE8: ii.ADD(SP, (int8_t)read8()); break; // warning! signed int!
      // SUB
    case 0x90: ii.SUB(B); break;
    case 0x91: ii.SUB(C); break;
    case 0x92: ii.SUB(D); break;
    case 0x93: ii.SUB(E); break;
    case 0x94: ii.SUB(H); break;
    case 0x95: ii.SUB(L); break;
    case 0x96: ii.SUB(Load(HL)); break;
    case 0x97: ii.SUB(A); break;
    case 0x98: ii.SUB(read8()); break;
      // rotations
    case 0x07: ii.RLC(A); break;
    case 0x0F: ii.RRC(A); break;
    case 0x17: ii.RL(A); break;
    case 0x1F: ii.RR(A); break;

    case 0x18: ii.JR(Cond::ALWAYS, read8()); break;
    case 0x20: ii.JR(Cond::NZ, read8()); break;
    case 0x28: ii.JR(Cond::Z, read8()); break;
    case 0x30: ii.JR(Cond::NC, read8()); break;
    case 0x38: ii.JR(Cond::C, read8()); break;
    case 0xAF: ii.XOR(A); break;

    case 0xCB: {
      auto op2 = read8();
      switch (op2) {
      case 0x11: ii.RL(C); break;
      case 0x7c: ii.BIT(7, H); break;
      default: printf("Extended op: %hhx\n", op2); break;
      }
      break;
    }
    case 0xCD: ii.CALL(Cond::ALWAYS, read16()); break;
    default: printf ("[%04x] -- op -- %02x\n", pc, op); break;
    }
  }
  u8 read8() { return buf[registers.PC++]; }
  u16 read16() { registers.PC += 2; return buf[registers.PC - 2] + buf[registers.PC - 1] * 256; }
};

#include "gpu.cc"

u8 *full_screen_buf; // 256x256 pixels;
u8 *display_buf; // 160x144 pixels;
u8 *io_buf;

int main() {
  char stdout_buf[2048];
  setvbuf(stdout, stdout_buf, _IOFBF, sizeof stdout_buf);
  Registers reg;
  Memory memory;
  Executor exec(reg, memory);
  OpPrint printer(reg);
  struct PPU ppu(memory);
  OpParser pp(reg, memory, exec, fopen("data/DMG_ROM.bin", "r"));

  while(reg.PC < 0x100) {
    for(int i=0; i<1000; i++) {
      pp.Step();
      ppu.Step(400);
      sleep(0);
    }
  }
  reg.dump();
}
