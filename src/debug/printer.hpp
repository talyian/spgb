#include "../base.hpp"
#include "../system/mmu.hpp"

struct Printer {
  Printer(MemoryMapper &mmu) : mmu(mmu) {

  }
  MemoryMapper &mmu;
  u16 PC = 0, PC_start = 0;
  u16 _read_u16() { return _read_u8() + 0x100 * _read_u8(); }
  u8 _read_u8() { return mmu.get(PC++); }
  template<class T> void mlog(T x) { log(PC_start, x); }
  template<class T, class ... TS> void mlog(T x, TS ... xs) { log(PC_start, x, xs ...); }
  
  void decode() {
    PC_start = PC;
    u16 opcode = mmu.get(PC++);
    if (opcode == 0xCB) opcode = 0x100 + mmu.get(PC++);

#define LD16_XXXX(RR) mlog("LD", #RR, _read_u16());
#define LOOP0(X)                                         \
      X(0x0, B); X(0x1, C); X(0x2, D); X(0x3, E);       \
      X(0x4, H); X(0x5, L); X(0x7, A);
#define LOOP(X) LOOP0(X) X(0x6, LoadHL);
    
    switch(opcode) {
    case 0x00: mlog("NOP"); break;
    case 0x10: mlog("STOP"); break;
    case 0x76: mlog("HALT"); break;

    #define LOOP16(X) X(0x00, BC); X(0x01, DE); X(0x02, HL); X(0x03, SP);
    #define X(OP, RR) \
      case 0x01 + 0x10 * OP: mlog("LD", #RR, _read_u16()); break; \
      case 0x03 + 0x10 * OP: mlog("INC", #RR); break; \
      case 0x0B + 0x10 * OP: mlog("DEC", #RR); break; \
      case 0x09 + 0x10 * OP: mlog("ADD HL,", #RR); break; \
      case 0xC1 + 0x10 * OP: mlog("POP", #RR); break; \
      case 0xC5 + 0x10 * OP: mlog("PUSH", #RR); break; \
      ;
    LOOP16(X)
    #undef X
    case 0x08: LD16_XXXX(SP); break;
      
    case 0x02: mlog("LD (BC), A"); break;
    case 0x12: mlog("LD (DE), A"); break;
    case 0x22: mlog("LD (HL++), A"); break;
    case 0x32: mlog("LD (HL--), A"); break;
    case 0x0A: mlog("LD A, (BC)"); break;
    case 0x1A: mlog("LD A, (DE)"); break;
    case 0x2A: mlog("LD A, (HL++)"); break;
    case 0x3A: mlog("LD A, (HL--)"); break;
      
    case 0x27: mlog("DAA"); break;
    case 0x2F: mlog("CPL"); break;      
    case 0x37: mlog("SCF"); break;      
    case 0x3F: mlog("CCF"); break;
    case 0xF3: mlog("DI"); break;
    case 0xFB: mlog("EI"); break;

    case 0xC7: mlog("RST 00"); break;
    case 0xCF: mlog("RST 08"); break;
    case 0xD7: mlog("RST 10"); break;
    case 0xDF: mlog("RST 18"); break;
    case 0xE7: mlog("RST 20"); break;
    case 0xEF: mlog("RST 28"); break;
    case 0xF7: mlog("RST 30"); break;
    case 0xFF: mlog("RST 38"); break;
      
    case 0x07: mlog("RLCA"); break; 
    case 0x0F: mlog("RRCA"); break; 
    case 0x17: mlog("RLA"); break; 
    case 0x1F: mlog("RRA"); break; 

    #define X(OP, REG)                     \
      case 0x04 + 8 * OP: mlog("INC", #REG); break;  \
      case 0x05 + 8 * OP: mlog("DEC", #REG); break; \
      case 0x06 + 8 * OP: mlog("LD", #REG, _read_u8()); break; \
      \
      case 0x80 + OP: mlog("ADD", #REG); break; \
      case 0x88 + OP: mlog("ADC", #REG); break; \
      case 0x90 + OP: mlog("SUB", #REG); break; \
      case 0x98 + OP: mlog("SBC", #REG); break; \
      case 0xA0 + OP: mlog("AND", #REG); break; \
      case 0xA8 + OP: mlog("XOR", #REG); break; \
      case 0xB0 + OP:  mlog("OR", #REG); break; \
      case 0xB8 + OP:  mlog("CP", #REG); break; \
      \
      case 0x40 + OP: mlog("LD B", #REG); break; \
      case 0x48 + OP: mlog("LD C", #REG); break; \
      case 0x50 + OP: mlog("LD D", #REG); break; \
      case 0x58 + OP: mlog("LD E", #REG); break; \
      case 0x60 + OP: mlog("LD H", #REG); break; \
      case 0x68 + OP: mlog("LD L", #REG); break; \
      case 0x70 + OP + 0x300 * (OP == 6): mlog("LD (HL)", #REG); break; \
      case 0x78 + OP: mlog("LD A", #REG); break;
    LOOP(X)
    #undef X

#define TARGET (u16)(PC_start + (i8)_read_u8() + 2)
    case 0x20: mlog("JR NZ,", TARGET); break;
    case 0x30: mlog("JR NC,", TARGET); break;
    case 0x18: mlog("JR ",    TARGET); break;
    case 0x28: mlog("JR Z,",  TARGET); break;
    case 0x38: mlog("JR C,",  TARGET); break;

    case 0xc2: mlog("JP NZ,", _read_u16()); break;
    case 0xc3: mlog("JP ", _read_u16()); break;
    case 0xca: mlog("JP Z,", _read_u16()); break;
    case 0xd2: mlog("JP NC,", _read_u16()); break;
    case 0xda: mlog("JP C,", _read_u16()); break;
    case 0xE9: mlog("JP HL"); break;
      // CALL
    case 0xC4: mlog("CALL NZ,", _read_u16()); break;
    case 0xCC: mlog("CALL Z,", _read_u16()); break;
    case 0xCD: mlog("CALL ", _read_u16()); break;
    case 0xD4: mlog("CALL NC,", _read_u16()); break;
    case 0xDC: mlog("CALL C,", _read_u16()); break;
      // RET - 20/8/16 cycles
    case 0xC0: mlog("RET NZ,", _read_u16()); break;
    case 0xC8: mlog("RET Z,", _read_u16()); break;
    case 0xC9: mlog("RET ", _read_u16()); break;
    case 0xD0: mlog("RET NC,", _read_u16()); break;
    case 0xD8: mlog("RET C,", _read_u16()); break;
    case 0xD9: mlog("RETI", _read_u16()); break;
      
    case 0xE0: mlog("LD FF00+", _read_u8(), ", A"); break;
    case 0xE2: mlog("LD FF00+", "C, A"); break;
    case 0xEA: mlog("LD (", _read_u16(),  "), A");
    case 0xE8: mlog("ADD SP", (i8)_read_u8()); break;
    case 0xF0: mlog("LD A, FF00+", _read_u8()); break;
    case 0xF2: mlog("LD A, FF00+C"); break;
    case 0xF8: mlog("LD HL, SP+", (i8)_read_u8()); break;
    case 0xF9: mlog("LD SP, HL"); break;
    case 0xFA: mlog("LD A,", _read_u16()); break;

    case 0xC6: mlog("ADD", _read_u8()); break;
    case 0xD6: mlog("SUB", _read_u8()); break;
    case 0xE6: mlog("AND", _read_u8()); break;
    case 0xF6: mlog("OR", _read_u8()); break;
    case 0xCE: mlog("ADC", _read_u8()); break;
    case 0xDE: mlog("SBC", _read_u8()); break;
    case 0xEE: mlog("XOR", _read_u8()); break;
    case 0xFE: mlog("CP", _read_u8()); break;
      
    #define X(op, RR) \
      case 0x100 + op: mlog("RLC", #RR); break; \
      case 0x108 + op: mlog("RRC", #RR); break; \
      case 0x110 + op: mlog("RL", #RR); break; \
      case 0x118 + op: mlog("RR", #RR); break; \
      case 0x120 + op: mlog("SLA", #RR); break; \
      case 0x128 + op: mlog("SRA", #RR); break; \
      case 0x130 + op: mlog("SWAP", #RR); break; \
      case 0x138 + op: mlog("SRL", #RR); break; 
    LOOP(X)
    #undef X
#define BIT(N, RR) mlog("BIT", (u8)N, #RR)
#define RES(N, RR) mlog("RES", (u8)N, #RR)
#define SET(N, RR) mlog("SET", (u8)N, #RR)
      
    #define X(op, target) \
      case 0x140 + op: BIT(0, target); break;\
      case 0x148 + op: BIT(1, target); break;\
      case 0x150 + op: BIT(2, target); break;\
      case 0x158 + op: BIT(3, target); break;\
      case 0x160 + op: BIT(4, target); break;\
      case 0x168 + op: BIT(5, target); break;\
      case 0x170 + op: BIT(6, target); break;\
      case 0x178 + op: BIT(7, target); break;
    LOOP(X)
    #undef X
    #define X(op, target) \
      case 0x180 + op: RES(0, target); break;\
      case 0x188 + op: RES(1, target); break;\
      case 0x190 + op: RES(2, target); break;\
      case 0x198 + op: RES(3, target); break;\
      case 0x1A0 + op: RES(4, target); break;\
      case 0x1A8 + op: RES(5, target); break;\
      case 0x1B0 + op: RES(6, target); break;\
      case 0x1B8 + op: RES(7, target); break;
    LOOP(X)
    #undef X
    #define X(op, target) \
      case 0x1C0 + op: SET(0, target); break;\
      case 0x1C8 + op: SET(1, target); break;\
      case 0x1D0 + op: SET(2, target); break;\
      case 0x1D8 + op: SET(3, target); break;\
      case 0x1E0 + op: SET(4, target); break;\
      case 0x1E8 + op: SET(5, target); break;\
      case 0x1F0 + op: SET(6, target); break;\
      case 0x1F8 + op: SET(7, target); break;
    LOOP(X)
    #undef X
    default:
      mlog("printer-unknown opcode", opcode);
      _stop();
    }
  }
};
