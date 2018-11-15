// CPU -
// this file contains the implementation of the Z80 instructions

#include "cpu.hh"

u8 CPU::get(Val8 a) { return a.get(reg, mem); }
u16 CPU::get(Val16 a) { return a.get(reg, mem); }
void CPU::set(Val8 a, Val8 v) { a.set(reg, mem, v); }
void CPU::set(Val16 a, Val16 v) { a.set(reg, mem, v); }

void CPU::NOP() { }
void CPU::HALT() { halted = true; }
void CPU::LD(Val8 dst, Val8 src) {
  set(dst, get(src)); }
void CPU::LD(Val16 dst, Val16 src) {
  set(dst, get(src));
  // todo: LD HL, SP+r8 sets HC, just like ADD SP,r8
}

// stack operations
void CPU::PUSH(Val16 val) { u16 v = get(val); mem[--reg.SP] = v; mem[--reg.SP] = v >> 8; }
// PEEK is not a real op!
u16 CPU::PEEK() { return (mem[reg.SP+0] << 8) | mem[reg.SP + 1]; }
void CPU::POP(Val16 addr) {
  u16 v = mem[reg.SP++];
  v = (v << 8) | mem[reg.SP++];
  // the bottom 4 bits of F are always zero
  if (addr.type == Val16::Reg && addr.value.r == REG16::AF)
    set(addr, (v & 0xFFF0));
  else
    set(addr, v);
}

// jumps
void CPU::JP(Cond cond, Val16 dst) { if(cond_eval(cond)) {reg.PC = get(dst);}}
void CPU::JR(Cond cond, Val8 offset) { if(cond_eval(cond)) {reg.PC += (int8_t)get(offset);}}
void CPU::RET(Cond cond) {if (cond_eval(cond)) { POP(REG16::PC); }}
void CPU::CALL(Cond cond, Val16 target) { if (cond_eval(cond)) { PUSH(REG16::PC); reg.PC = get(target); }}
void CPU::RST(u8 f) { reg.IME = 0; PUSH(REG16::PC); reg.PC = f; }

// tests
void CPU::CP(Val8 rhs) {
  u8 a = reg.A;
  u8 b = get(rhs);
  reg.setFZ(a == b ? 1 : 0);
  reg.setFO(1);
  reg.setFH((a & 0xF) < (b & 0xF));
  reg.setFC(a < b);
}
void CPU::INC(Val8 dst) {
  u8 v = get(dst) + 1;
  set(dst, v);
  reg.setFZ(v == 0);
  reg.setFO(1);
  reg.setFH((v & 0xF) == 0);
}
void CPU::DEC(Val8 dst) {
  u8 v = get(dst) - 1;
  set(dst, v);
  reg.setFZ(v == 0);
  reg.setFO(1);
  reg.setFH((v & 0xF) == 0xFF);
}

void CPU::INC(Val16 dst) { set(dst, get(dst) + 1); }
void CPU::DEC(Val16 dst) { set(dst, get(dst) - 1); }

// bitwise operations
// TODO: the 1-byte opcodes RLCA RLA RRA RRCA don't set zero-flag?
// 9-bit Rotate right
void CPU::RR(Val8 v) {
  u8 val = get(v);
  u8 new_val = (val >> 1) | (reg.FC() << 7);
  reg.F = 0;
  reg.setFZ(new_val == 0);
  reg.setFC(val & 1);
  set(v, val);
}
// 8-bit rotate right
void CPU::RRC(Val8 v) {
  u8 val = get(v);
  val = (val << 7) | (val >> 1);
  reg.F = 0;
  reg.setFZ(val == 0);
  reg.setFC(val & 0x80); // old bit 7 is now bit 6
  set(v, val);
}
// 9-bit shift
void CPU::RL(Val8 v) {
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
void CPU::RLC(Val8 v) {
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
void CPU::SLA(Val8 val) {
  u8 v = get(val);
  reg.F = 0;
  reg.setFZ((v << 1) == 0);
  reg.setFC(v >> 7);
  set(val, v << 1);
}
// shift right - signed
void CPU::SRA(Val8 val) {
  int8_t v = get(val);
  reg.F = 0;
  reg.setFZ((v >> 1) == 0);
  reg.setFC(v & 1);
  set(val, v >> 1);
}
// shift right
void CPU::SRL(Val8 val) {
  u8 v = get(val);
  reg.F = 0;
  reg.setFZ((v >> 1) == 0);
  reg.setFC(v & 1);
  set(val, v >> 1);
}
// swap nibbles
void CPU::SWAP(Val8 v) {
  u8 val = get(v);
  val = (val >> 4) | (val << 4);
  set(v, val);
  reg.F = 0;
  reg.setFZ(val == 0);
}

// // single-bit
void CPU::RES(int bit, Val8 val) { set(val, get(val) & ~(1 << bit)); }
void CPU::SET(int bit, Val8 val) {
  u8 v = get(val);
  u8 v2 = v | (1 << bit);
  set(val, v2);
}
void CPU::BIT(int bit, Val8 rhs) {
  u8 val = get(rhs);
  u8 val2 = (val >> bit) & 1;
  reg.setFZ(val2 == 0);
  reg.setFO(0);
  reg.setFH(1); // TODO: really???
}

// // Arithmetic
void CPU::AND(Val8 val) { reg.A &= get(val); reg.F = 0x20; reg.setFZ(reg.A == 0); }
void CPU::OR(Val8 val) { reg.A |= get(val); reg.F = 0; reg.setFZ(reg.A == 0); }
void CPU::XOR(Val8 val) { reg.A ^= get(val); reg.F = 0; reg.setFZ(reg.A == 0); }
void CPU::CPL() { reg.A = ~reg.A; reg.F |= ~0x60; }
void CPU::ADD(Val8 dst, Val8 val) {
  u8 oldval = get(dst);
  u8 newval = oldval + get(val);
  set(dst, newval);
  reg.setFZ(newval == 0);
  reg.setFO(0);
  reg.setFH((newval & 0xF) < (oldval & 0xF));
  reg.setFC(newval < oldval);
}
void CPU::ADC(Val8 dst, Val8 val) {
  u8 oldval = get(dst);
  u8 newval = oldval + get(val) + reg.FC();
  set(dst, newval);
  reg.setFZ(newval == 0);
  reg.setFO(0);
  reg.setFH((newval & 0xF) < (oldval & 0xF));
  reg.setFC(newval < oldval);
}
void CPU::ADD(Val16 dst, Val16 val) {
  u16 oldval = get(dst);
  u16 newval = oldval + get(val);
  set(dst, newval);
  // hmmmm.
  if (dst.type == Val16::Reg && dst.value.r == REG16::SP) reg.setFZ(0);
  reg.setFO(0);
  reg.setFH((newval & 0xFFF) < (oldval & 0xFFF));
  reg.setFC(newval < oldval);
}
void CPU::SUB(Val8 val) {
  u8 old_a = reg.A;
  reg.A = old_a - get(val);
  reg.setFZ(reg.A == 0);
  reg.setFO(1);
  reg.setFH((reg.A & 0xF) > (old_a & 0xF));
  reg.setFC(reg.A > old_a);
}
void CPU::SBC(Val8 val) {
  u8 old_a = reg.A;
  reg.A = old_a - get(val) - reg.FC();
  reg.setFZ(reg.A == 0);
  reg.setFO(1);
  reg.setFH((reg.A & 0xF) > (old_a & 0xF));
  reg.setFC(reg.A > old_a);
}

// Misc
void CPU::DI() { reg.IME=0x00; }
void CPU::EI() { reg.IME=0xFF; }
void CPU::STOP() { printf("TODO: STOP\n"); abort(3);}
void CPU::DAA() {
  if (reg.FO()) { // subtraction
    if (reg.FC()) reg.A -= 0x60;
    if (reg.FH()) reg.A -= 6;
  } else {
    if (reg.FC() || reg.A > 0x99) { reg.A += 0x60; reg.setFC(1); }
    if (reg.FH() || (reg.A & 0xF) > 9) { reg.A += 6; }
  }
  reg.setFZ(reg.A == 0);
  reg.setFH(0);
}
void CPU::CCF() {
  reg.setFC(~reg.FC());
  reg.setFO(0);
  reg.setFH(0);
}
void CPU::SCF() {
  reg.setFC(1);
  reg.setFO(0);
  reg.setFH(0);
}
