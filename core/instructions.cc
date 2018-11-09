#include "instructions.hh"

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
  case Val16::Val: printf("$%hx", v.value.value); break;
  case Val16::PtrN: printf("($%hx)", v.value.ptr_n); break;
  case Val16::PtrR: printf("(%.2s)", (char *)"BCDEHLAFSPPC" + (2 * (int)v.value.r)); break;
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
  #define F(Z) case REG16::Z: return (u16)rr.Z;
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
  case PtrR: {
    u16 v = getRegister(value.ptr_r, reg);
    return mem[v];
  }
  case IoN: return mem[0xFF00 + value.io_n];
  case IoR: return mem[0xFF00 + getRegister(value.io_r, reg)];
  }
}

void Val8::set(Registers &reg, Memory &mem, Val8 source)  {
  u8 new_value = source.get(reg, mem);
  switch(type) {
  case Reg: setRegister(value.r, reg, new_value); break;
  case Val: abort(1); // TODO
  case PtrN: mem[value.ptr_n] = new_value; break;
  case PtrR: mem[getRegister(value.ptr_r, reg)] = new_value; break;
  case IoN: mem[0xFF00 + value.io_n] = new_value; break;
  case IoR: mem[0xFF00 + getRegister(value.io_r, reg)] = new_value; break;
  }
}

u16 Val16::get(Registers &reg, Memory &mem) {
  switch(type) {
  case Reg: return getRegister(value.r, reg);
  case Val: return value.value;
  case PtrN: return mem[value.ptr_n] + 256 * mem[value.ptr_n + 1];
  case PtrR:
    u16 v = getRegister(value.ptr_r, reg);
    return mem[v] + 256 * mem[v + 1];
  }
}

void Val16::set(Registers &reg, Memory &mem, Val16 source) {
  u16 new_value = source.get(reg, mem);
  switch(type) {
  case Reg: setRegister(value.r, reg, new_value); break;
  case Val: abort(2); // TODO
  case PtrN: {
    u16 v = value.ptr_n;
    mem[v] = new_value;
    mem[v + 1] = new_value >> 8;
    break; }
  case PtrR: {
    u16 v = getRegister(value.ptr_r, reg);
    mem[v] = new_value;
    mem[v + 1] = new_value >> 8;
    break; }
  }
}
