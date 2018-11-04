#include <cstdint>
#include <cstdio>
#include <memory>

struct Register16 {
  uint8_t *h = 0, *l = 0;
  Register16(uint8_t *h, uint8_t *l): h(h), l(l) { }
  uint16_t operator--(int) {
    auto v = *h * 256 + *l;
    if ((*l)--) (*h)--;
    return v;
  }
};



// Z80 processor
struct Z80 {
  uint64_t time_m = 0;
  uint64_t time_t = 0;

  struct Registers {
    uint8_t A = 0, B = 0, C = 0, D = 0, E = 0;
    union {
      struct { uint8_t H, L; } u8;
      uint16_t u16;
    } HL;
    uint16_t PC = 0, SP = 0;
    uint64_t M = 0, T = 0;
    uint8_t F = 0;
    bool FlagZero ();
    bool FlagSubtract ();
    bool FlagHalfCarry ();
    bool FlagCarry ();
  } registers;

  struct Memory {
    uint8_t * rom = 0;
    size_t rom_size = 0;

  } memory;
  uint8_t readu8(uint16_t addr);
  uint16_t readu16(uint16_t addr);
  uint8_t readu8pci();
  uint16_t readu16pci();
  void write(uint16_t addr, uint8_t val);
  void write(uint16_t addr, uint16_t val);

  void _ld(uint8_t * addr, uint8_t val) { *addr = val; }
  void _ld(uint16_t * addr, uint16_t val) { *addr = val; }
  void _xor_a(uint8_t val) { registers.A ^= val; }
  void _add(uint8_t * accumulator, uint8_t val) { *accumulator += val; }
  void _rst(uint16_t addr) { _push(registers.PC); registers.PC = addr; }
  void _push(uint16_t value) { memory.rom[registers.SP] = value; registers.SP -= 2; }
  void _pop(uint16_t * addr ) { *addr = memory.rom[registers.SP += 2]; }
  void _JR(bool cond, uint16_t relative) { if (cond) { registers.PC += relative; } }

  void _bit(uint8_t bit, uint8_t val) {
    bool f = (uint8_t)0x01 & (val >> bit);
    // set Z flag to f
    registers.F = (registers.F & ~0x80) | (f * 0x80);
    // off subtract flag
    registers.F = registers.F & ~0x40;
    // set halfcarry flag
    registers.F = registers.F | 0x20;
  }
  void execute() {
    auto op = readu8pci();
    uint8_t op2 = 0;
    switch(op) {
    case 0x20: {
      auto v = readu16pci();
      printf("JR NZ %hx\n", v);
      _JR(!registers.FlagZero(), v); break; }
    case 0x28: {
      auto v = readu16pci();
      printf("JR Z %hx\n", v);
      _JR(registers.FlagZero(), v); break; }
    case 0x21: { auto v = readu16pci(); printf("LD HL, %hx\n", v); _ld(&registers.HL.u16, v); break; }
    case 0x31: { auto v = readu16pci(); printf("LD SP %hx\n", v); _ld(&registers.SP, v); } break;
    case 0x32: { printf("LDD (HL), A\n");_ld(memory.rom + registers.HL.u16--, registers.A); break; }
    case 0xCB:
      // extended op:
      op2 = readu8pci();
      switch(op2) {
      case 0x7c: {
        printf("BIT %hhx H\n", 7);
        _bit(7, registers.HL.u8.H); break; }
      default:
        printf("extended op %hhx\n", op2);
        break;
      }
      break;
    case 0xAF: { printf ("XOR A\n"); _xor_a(registers.A); break; }
    case 0xFF: { printf ("RST 38\n"); _rst(38); break; }
    case 0x80: { printf ("ADD A B\n"); _add(&registers.A, registers.B); break; }
    default:
      printf("[%d] exec:: %hhx\n", registers.PC, op);
      break;
    }
    time_m += registers.M;
    time_t += registers.T;
  }

  void dump() {
    printf("Registers:\n");
#define SHOW(XX) printf(#XX ": %hhx\n", registers.XX)
#define SHOW16(XX) printf(#XX ": %hx\n", registers.XX)
    SHOW(A);
    SHOW(B);
    SHOW(C);
    SHOW(D);
    SHOW(E);
    SHOW(F);
    SHOW16(HL.u16);
    SHOW(HL.u8.H);
    SHOW(HL.u8.L);
    SHOW16(PC);
    SHOW16(SP);
  }
};

bool Z80::Registers::FlagZero () { return this->F & 0x80; }
bool Z80::Registers::FlagSubtract () { return this->F & 0x40; }
bool Z80::Registers::FlagHalfCarry () { return this->F & 0x20; }
bool Z80::Registers::FlagCarry () { return this->F & 0x10; }

uint8_t Z80::readu8(uint16_t addr) { return this->memory.rom[addr]; }
uint16_t Z80::readu16(uint16_t addr) {
  return this->memory.rom[addr] + 256 * this->memory.rom[addr + 1];
}
uint8_t Z80::readu8pci() {
  return readu8(registers.PC++);
}
uint16_t Z80::readu16pci() {
  registers.PC += 2;
  auto v = readu16(registers.PC - 2);
  return v;
}

int main() {

  auto boot_rom = fopen("DMG_ROM.bin", "r");
  fseek(boot_rom, 0, SEEK_END);
  auto rom_size = ftell(boot_rom);
  auto rom_buf = (uint8_t *)malloc(rom_size);
  fseek(boot_rom, 0, 0);
  fread(rom_buf, 1, rom_size, boot_rom);

  Z80 z80;
  z80.memory.rom = rom_buf;
  z80.memory.rom_size = rom_size;

  z80.execute();
  z80.execute();
  z80.execute();
  z80.execute();
  z80.execute();

  z80.execute();
  z80.execute();
  z80.execute();
  z80.execute();
  z80.execute();
  z80.execute();
  z80.execute();

  z80.dump();
}
