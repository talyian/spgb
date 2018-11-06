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

void show(Cond c) {
  switch(c) {
  case Cond::ALWAYS: break;
  case Cond::C: printf("C"); break;
  case Cond::Z: printf("Z"); break;
  case Cond::NC: printf("NC"); break;
  case Cond::NZ: printf("NZ"); break;
  }
}

void show(Val16 v) {
  switch(v.type) {
  case Val16::Reg: printf("%.2s", (char *)"BCDEHLAFSPPC" + (2 * (int)v.value.r)); break;
  case Val16::Val: printf("%hx", v.value.value); break;
  }
}

void show(Val8 v) {
  switch(v.type) {
  case Val8::Val: printf("$%hhx", v.value.value); break;
  case Val8::Reg: printf("%c", "ABCDEFHL"[(int)v.value.r]); break;
  case Val8::PtrN: printf("($%hx)", v.value.ptr_n); break;
  case Val8::PtrR: printf("(%.2s)", (char *)"BCDEHLAFSPPC" + (2 * (int)v.value.ptr_r)); break;
  case Val8::IoN: printf("(IO:FF00+$%hhx)", v.value.io_n); break;
  case Val8::IoR: printf("(IO:FF00+%c)", "ABCDEFHL"[(int)v.value.io_r]); break;
  }
}

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
  printf("JP %04x\n", get(dst));
  if (cond_eval(cond)) {
    reg.PC = get(dst); } }
void Executor::JR(Cond cond, Val8 offset) {
   if (reg.PC > 0x10) {
    // {
    // printf("[%x] ", reg._PC);
    // printf("JR ");
    // show(cond);
    // printf(" %04x\n", reg.PC + (int8_t)get(offset));
    // reg.dump();
  }
  if (cond_eval(cond)) { reg.PC += (int8_t)get(offset); }}
void Executor::RET(Cond cond) {
  // printf("RET %04x\n", PEEK());
  if (cond_eval(cond)) { POP(REG16::PC); }}
void Executor::CALL(Cond cond, Val16 target) {
  // printf("CALL %04x\n", get(target));
  if (cond_eval(cond)) {
    PUSH(REG16::PC);
    reg.PC = get(target);
  }}
void Executor::RST(u8 f) {
  printf("RST %hhx\n", f);
  PUSH(REG16::PC);
  reg.PC = f;
}
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

// bitwise operations
// TODO: the 1-byte opcodes RLCA RLA RRA RRCA don't set zero-flag?
// 9-bit Rotate right
void Executor::RR(Val8 v) {
  u8 val = get(v);
  u8 new_val = (val >> 1) | (reg.FC() << 7);
  reg.F = 0;
  reg.setFZ(new_val == 0);
  reg.setFC(val & 1);
  set(v, val);
}
// 8-bit rotate right
void Executor::RRC(Val8 v) {
  u8 val = get(v);
  val = (val << 7) | (val >> 1);
  reg.F = 0;
  reg.setFZ(val == 0);
  reg.setFC(val & 0x80); // old bit 7 is now bit 6
  set(v, val);
}
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
// shift left
void Executor::SLA(Val8 val) {
  u8 v = get(val);
  reg.F = 0;
  reg.setFZ((v << 1) == 0);
  reg.setFC(v >> 7);
  set(val, v << 1);
}
// shift right - signed
void Executor::SRA(Val8 val) {
  int8_t v = get(val);
  reg.F = 0;
  reg.setFZ((v >> 1) == 0);
  reg.setFC(v & 1);
  set(val, v >> 1);
}
// shift right
void Executor::SRL(Val8 val) {
  u8 v = get(val);
  reg.F = 0;
  reg.setFZ((v >> 1) == 0);
  reg.setFC(v & 1);
  set(val, v >> 1);
}
// swap nibbles
void Executor::SWAP(Val8 v) {
  u8 val = get(v);
  val = (val >> 4) | (val << 4);
  set(v, val);
  reg.F = 0;
  reg.setFZ(val == 0);
}

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
void Executor::AND(Val8 val) { reg.A &= get(val); reg.F = 0x20; reg.setFZ(reg.A == 0); }
void Executor::OR(Val8 val) { reg.A |= get(val); reg.F = 0; reg.setFZ(reg.A == 0); }
void Executor::XOR(Val8 val) { reg.A ^= get(val); reg.F = 0; reg.setFZ(reg.A == 0); }
void Executor::CPL() { reg.A = ~reg.A; reg.F |= ~0x60; }
void Executor::ADD(Val8 dst, Val8 val) { set(dst, get(dst) + get(val)); }
void Executor::ADC(Val8 dst, Val8 val) { set(dst, get(dst) + get(val) + reg.FC()); }
void Executor::ADD(Val16 dst, Val16 val) { set(dst, get(dst) + get(val)); }
void Executor::SUB(Val8 val) { reg.A -= get(val); }
void Executor::SBC(Val8 val) { reg.A -= get(val) + reg.FC(); }

// Misc
void Executor::DI() { } // TODO
void Executor::EI() { } // TODO

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
    registers._PC = registers.PC;
    u16 op = read8();
    if (op == 0xCB) { op = 0x100 | read8(); }
    #define OP(CMD, op_len, op_time, flags) {ii.CMD;ii.timer+=op_time;break;}
    #include "op_table.cc"
  }
  u8 read8() { return buf[registers.PC++]; }
  u16 read16() { registers.PC += 2; return buf[registers.PC - 2] + buf[registers.PC - 1] * 256; }
};

#include "gpu.cc"

int main() {
  Registers registers;
  Memory memory;
  Executor exec(registers, memory);
  PPU ppu { memory };
  OpParser pp(registers, memory, exec, fopen("data/DMG_ROM.bin", "r"));

  while(registers.PC < 0x100) {
    for(int i=0; i<1000; i++) {
      pp.Step();
      ppu.Step(400);
      sleep(0);
    }
  }
  registers.dump();
}
