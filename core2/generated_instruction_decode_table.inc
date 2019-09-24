case 0x0:  // 0 1 4 ---- NOP()
  ii.NOP(); pc = pc_start + 1; break;
case 0x1:  // 1 3 12 ---- LD(BC,Imm16)
  ii.LD(BC(), Imm16()); pc = pc_start + 3; break;
case 0x2:  // 2 1 8 ---- LD(Load(BC),A)
  ii.LD(Load(BC()), A()); pc = pc_start + 1; break;
case 0x3:  // 3 1 8 ---- INC(BC)
  ii.INC(BC()); pc = pc_start + 1; break;
case 0x4:  // 4 1 4 Z0H- INC(B)
  ii.INC(B()); pc = pc_start + 1; break;
case 0x5:  // 5 1 4 Z1H- DEC(B)
  ii.DEC(B()); pc = pc_start + 1; break;
case 0x6:  // 6 2 8 ---- LD(B,Imm8)
  ii.LD(B(), Imm8()); pc = pc_start + 2; break;
case 0x7:  // 7 1 4 000C RLCA()
  ii.RLCA(); pc = pc_start + 1; break;
case 0x8:  // 8 3 20 ---- LD(Load16(Imm16),SP)
  ii.LD(Load16(Imm16()), SP()); pc = pc_start + 3; break;
case 0x9:  // 9 1 8 -0HC ADD(HL,BC)
  ii.ADD(HL(), BC()); pc = pc_start + 1; break;
case 0xA:  // A 1 8 ---- LD(A,Load(BC))
  ii.LD(A(), Load(BC())); pc = pc_start + 1; break;
case 0xB:  // B 1 8 ---- DEC(BC)
  ii.DEC(BC()); pc = pc_start + 1; break;
case 0xC:  // C 1 4 Z0H- INC(C)
  ii.INC(C()); pc = pc_start + 1; break;
case 0xD:  // D 1 4 Z1H- DEC(C)
  ii.DEC(C()); pc = pc_start + 1; break;
case 0xE:  // E 2 8 ---- LD(C,Imm8)
  ii.LD(C(), Imm8()); pc = pc_start + 2; break;
case 0xF:  // F 1 4 000C RRCA()
  ii.RRCA(); pc = pc_start + 1; break;
case 0x10:  // 10 2 4 ---- STOP()
  ii.STOP(); pc = pc_start + 2; break;
case 0x11:  // 11 3 12 ---- LD(DE,Imm16)
  ii.LD(DE(), Imm16()); pc = pc_start + 3; break;
case 0x12:  // 12 1 8 ---- LD(Load(DE),A)
  ii.LD(Load(DE()), A()); pc = pc_start + 1; break;
case 0x13:  // 13 1 8 ---- INC(DE)
  ii.INC(DE()); pc = pc_start + 1; break;
case 0x14:  // 14 1 4 Z0H- INC(D)
  ii.INC(D()); pc = pc_start + 1; break;
case 0x15:  // 15 1 4 Z1H- DEC(D)
  ii.DEC(D()); pc = pc_start + 1; break;
case 0x16:  // 16 2 8 ---- LD(D,Imm8)
  ii.LD(D(), Imm8()); pc = pc_start + 2; break;
case 0x17:  // 17 1 4 000C RLA()
  ii.RLA(); pc = pc_start + 1; break;
case 0x18:  // 18 2 12 ---- JR(CT,ImmI8)
  ii.JR(CT(), ImmI8()); pc = pc_start + 2; break;
case 0x19:  // 19 1 8 -0HC ADD(HL,DE)
  ii.ADD(HL(), DE()); pc = pc_start + 1; break;
case 0x1A:  // 1A 1 8 ---- LD(A,Load(DE))
  ii.LD(A(), Load(DE())); pc = pc_start + 1; break;
case 0x1B:  // 1B 1 8 ---- DEC(DE)
  ii.DEC(DE()); pc = pc_start + 1; break;
case 0x1C:  // 1C 1 4 Z0H- INC(E)
  ii.INC(E()); pc = pc_start + 1; break;
case 0x1D:  // 1D 1 4 Z1H- DEC(E)
  ii.DEC(E()); pc = pc_start + 1; break;
case 0x1E:  // 1E 2 8 ---- LD(E,Imm8)
  ii.LD(E(), Imm8()); pc = pc_start + 2; break;
case 0x1F:  // 1F 1 4 000C RRA()
  ii.RRA(); pc = pc_start + 1; break;
case 0x20:  // 20 2 12/8 ---- JR(CNZ,ImmI8)
  ii.JR(CNZ(), ImmI8()); pc = pc_start + 2; break;
case 0x21:  // 21 3 12 ---- LD(HL,Imm16)
  ii.LD(HL(), Imm16()); pc = pc_start + 3; break;
case 0x22:  // 22 1 8 ---- LD(Inc(HL),A)
  ii.LD(Inc(HL()), A()); pc = pc_start + 1; break;
case 0x23:  // 23 1 8 ---- INC(HL)
  ii.INC(HL()); pc = pc_start + 1; break;
case 0x24:  // 24 1 4 Z0H- INC(H)
  ii.INC(H()); pc = pc_start + 1; break;
case 0x25:  // 25 1 4 Z1H- DEC(H)
  ii.DEC(H()); pc = pc_start + 1; break;
case 0x26:  // 26 2 8 ---- LD(H,Imm8)
  ii.LD(H(), Imm8()); pc = pc_start + 2; break;
case 0x27:  // 27 1 4 Z-0C DAA()
  ii.DAA(); pc = pc_start + 1; break;
case 0x28:  // 28 2 12/8 ---- JR(CZ,ImmI8)
  ii.JR(CZ(), ImmI8()); pc = pc_start + 2; break;
case 0x29:  // 29 1 8 -0HC ADD(HL,HL)
  ii.ADD(HL(), HL()); pc = pc_start + 1; break;
case 0x2A:  // 2A 1 8 ---- LD(A,Inc(HL))
  ii.LD(A(), Inc(HL())); pc = pc_start + 1; break;
case 0x2B:  // 2B 1 8 ---- DEC(HL)
  ii.DEC(HL()); pc = pc_start + 1; break;
case 0x2C:  // 2C 1 4 Z0H- INC(L)
  ii.INC(L()); pc = pc_start + 1; break;
case 0x2D:  // 2D 1 4 Z1H- DEC(L)
  ii.DEC(L()); pc = pc_start + 1; break;
case 0x2E:  // 2E 2 8 ---- LD(L,Imm8)
  ii.LD(L(), Imm8()); pc = pc_start + 2; break;
case 0x2F:  // 2F 1 4 -11- CPL()
  ii.CPL(); pc = pc_start + 1; break;
case 0x30:  // 30 2 12/8 ---- JR(CNC,ImmI8)
  ii.JR(CNC(), ImmI8()); pc = pc_start + 2; break;
case 0x31:  // 31 3 12 ---- LD(SP,Imm16)
  ii.LD(SP(), Imm16()); pc = pc_start + 3; break;
case 0x32:  // 32 1 8 ---- LD(Dec(HL),A)
  ii.LD(Dec(HL()), A()); pc = pc_start + 1; break;
case 0x33:  // 33 1 8 ---- INC(SP)
  ii.INC(SP()); pc = pc_start + 1; break;
case 0x34:  // 34 1 12 Z0H- INC(Load(HL))
  ii.INC(Load(HL())); pc = pc_start + 1; break;
case 0x35:  // 35 1 12 Z1H- DEC(Load(HL))
  ii.DEC(Load(HL())); pc = pc_start + 1; break;
case 0x36:  // 36 2 12 ---- LD(Load(HL),Imm8)
  ii.LD(Load(HL()), Imm8()); pc = pc_start + 2; break;
case 0x37:  // 37 1 4 -001 SCF()
  ii.SCF(); pc = pc_start + 1; break;
case 0x38:  // 38 2 12/8 ---- JR(CC,ImmI8)
  ii.JR(CC(), ImmI8()); pc = pc_start + 2; break;
case 0x39:  // 39 1 8 -0HC ADD(HL,SP)
  ii.ADD(HL(), SP()); pc = pc_start + 1; break;
case 0x3A:  // 3A 1 8 ---- LD(A,Dec(HL))
  ii.LD(A(), Dec(HL())); pc = pc_start + 1; break;
case 0x3B:  // 3B 1 8 ---- DEC(SP)
  ii.DEC(SP()); pc = pc_start + 1; break;
case 0x3C:  // 3C 1 4 Z0H- INC(A)
  ii.INC(A()); pc = pc_start + 1; break;
case 0x3D:  // 3D 1 4 Z1H- DEC(A)
  ii.DEC(A()); pc = pc_start + 1; break;
case 0x3E:  // 3E 2 8 ---- LD(A,Imm8)
  ii.LD(A(), Imm8()); pc = pc_start + 2; break;
case 0x3F:  // 3F 1 4 -00C CCF()
  ii.CCF(); pc = pc_start + 1; break;
case 0x40:  // 40 1 4 ---- LD(B,B)
  ii.LD(B(), B()); pc = pc_start + 1; break;
case 0x41:  // 41 1 4 ---- LD(B,C)
  ii.LD(B(), C()); pc = pc_start + 1; break;
case 0x42:  // 42 1 4 ---- LD(B,D)
  ii.LD(B(), D()); pc = pc_start + 1; break;
case 0x43:  // 43 1 4 ---- LD(B,E)
  ii.LD(B(), E()); pc = pc_start + 1; break;
case 0x44:  // 44 1 4 ---- LD(B,H)
  ii.LD(B(), H()); pc = pc_start + 1; break;
case 0x45:  // 45 1 4 ---- LD(B,L)
  ii.LD(B(), L()); pc = pc_start + 1; break;
case 0x46:  // 46 1 8 ---- LD(B,Load(HL))
  ii.LD(B(), Load(HL())); pc = pc_start + 1; break;
case 0x47:  // 47 1 4 ---- LD(B,A)
  ii.LD(B(), A()); pc = pc_start + 1; break;
case 0x48:  // 48 1 4 ---- LD(C,B)
  ii.LD(C(), B()); pc = pc_start + 1; break;
case 0x49:  // 49 1 4 ---- LD(C,C)
  ii.LD(C(), C()); pc = pc_start + 1; break;
case 0x4A:  // 4A 1 4 ---- LD(C,D)
  ii.LD(C(), D()); pc = pc_start + 1; break;
case 0x4B:  // 4B 1 4 ---- LD(C,E)
  ii.LD(C(), E()); pc = pc_start + 1; break;
case 0x4C:  // 4C 1 4 ---- LD(C,H)
  ii.LD(C(), H()); pc = pc_start + 1; break;
case 0x4D:  // 4D 1 4 ---- LD(C,L)
  ii.LD(C(), L()); pc = pc_start + 1; break;
case 0x4E:  // 4E 1 8 ---- LD(C,Load(HL))
  ii.LD(C(), Load(HL())); pc = pc_start + 1; break;
case 0x4F:  // 4F 1 4 ---- LD(C,A)
  ii.LD(C(), A()); pc = pc_start + 1; break;
case 0x50:  // 50 1 4 ---- LD(D,B)
  ii.LD(D(), B()); pc = pc_start + 1; break;
case 0x51:  // 51 1 4 ---- LD(D,C)
  ii.LD(D(), C()); pc = pc_start + 1; break;
case 0x52:  // 52 1 4 ---- LD(D,D)
  ii.LD(D(), D()); pc = pc_start + 1; break;
case 0x53:  // 53 1 4 ---- LD(D,E)
  ii.LD(D(), E()); pc = pc_start + 1; break;
case 0x54:  // 54 1 4 ---- LD(D,H)
  ii.LD(D(), H()); pc = pc_start + 1; break;
case 0x55:  // 55 1 4 ---- LD(D,L)
  ii.LD(D(), L()); pc = pc_start + 1; break;
case 0x56:  // 56 1 8 ---- LD(D,Load(HL))
  ii.LD(D(), Load(HL())); pc = pc_start + 1; break;
case 0x57:  // 57 1 4 ---- LD(D,A)
  ii.LD(D(), A()); pc = pc_start + 1; break;
case 0x58:  // 58 1 4 ---- LD(E,B)
  ii.LD(E(), B()); pc = pc_start + 1; break;
case 0x59:  // 59 1 4 ---- LD(E,C)
  ii.LD(E(), C()); pc = pc_start + 1; break;
case 0x5A:  // 5A 1 4 ---- LD(E,D)
  ii.LD(E(), D()); pc = pc_start + 1; break;
case 0x5B:  // 5B 1 4 ---- LD(E,E)
  ii.LD(E(), E()); pc = pc_start + 1; break;
case 0x5C:  // 5C 1 4 ---- LD(E,H)
  ii.LD(E(), H()); pc = pc_start + 1; break;
case 0x5D:  // 5D 1 4 ---- LD(E,L)
  ii.LD(E(), L()); pc = pc_start + 1; break;
case 0x5E:  // 5E 1 8 ---- LD(E,Load(HL))
  ii.LD(E(), Load(HL())); pc = pc_start + 1; break;
case 0x5F:  // 5F 1 4 ---- LD(E,A)
  ii.LD(E(), A()); pc = pc_start + 1; break;
case 0x60:  // 60 1 4 ---- LD(H,B)
  ii.LD(H(), B()); pc = pc_start + 1; break;
case 0x61:  // 61 1 4 ---- LD(H,C)
  ii.LD(H(), C()); pc = pc_start + 1; break;
case 0x62:  // 62 1 4 ---- LD(H,D)
  ii.LD(H(), D()); pc = pc_start + 1; break;
case 0x63:  // 63 1 4 ---- LD(H,E)
  ii.LD(H(), E()); pc = pc_start + 1; break;
case 0x64:  // 64 1 4 ---- LD(H,H)
  ii.LD(H(), H()); pc = pc_start + 1; break;
case 0x65:  // 65 1 4 ---- LD(H,L)
  ii.LD(H(), L()); pc = pc_start + 1; break;
case 0x66:  // 66 1 8 ---- LD(H,Load(HL))
  ii.LD(H(), Load(HL())); pc = pc_start + 1; break;
case 0x67:  // 67 1 4 ---- LD(H,A)
  ii.LD(H(), A()); pc = pc_start + 1; break;
case 0x68:  // 68 1 4 ---- LD(L,B)
  ii.LD(L(), B()); pc = pc_start + 1; break;
case 0x69:  // 69 1 4 ---- LD(L,C)
  ii.LD(L(), C()); pc = pc_start + 1; break;
case 0x6A:  // 6A 1 4 ---- LD(L,D)
  ii.LD(L(), D()); pc = pc_start + 1; break;
case 0x6B:  // 6B 1 4 ---- LD(L,E)
  ii.LD(L(), E()); pc = pc_start + 1; break;
case 0x6C:  // 6C 1 4 ---- LD(L,H)
  ii.LD(L(), H()); pc = pc_start + 1; break;
case 0x6D:  // 6D 1 4 ---- LD(L,L)
  ii.LD(L(), L()); pc = pc_start + 1; break;
case 0x6E:  // 6E 1 8 ---- LD(L,Load(HL))
  ii.LD(L(), Load(HL())); pc = pc_start + 1; break;
case 0x6F:  // 6F 1 4 ---- LD(L,A)
  ii.LD(L(), A()); pc = pc_start + 1; break;
case 0x70:  // 70 1 8 ---- LD(Load(HL),B)
  ii.LD(Load(HL()), B()); pc = pc_start + 1; break;
case 0x71:  // 71 1 8 ---- LD(Load(HL),C)
  ii.LD(Load(HL()), C()); pc = pc_start + 1; break;
case 0x72:  // 72 1 8 ---- LD(Load(HL),D)
  ii.LD(Load(HL()), D()); pc = pc_start + 1; break;
case 0x73:  // 73 1 8 ---- LD(Load(HL),E)
  ii.LD(Load(HL()), E()); pc = pc_start + 1; break;
case 0x74:  // 74 1 8 ---- LD(Load(HL),H)
  ii.LD(Load(HL()), H()); pc = pc_start + 1; break;
case 0x75:  // 75 1 8 ---- LD(Load(HL),L)
  ii.LD(Load(HL()), L()); pc = pc_start + 1; break;
case 0x76:  // 76 1 4 ---- HALT()
  ii.HALT(); pc = pc_start + 1; break;
case 0x77:  // 77 1 8 ---- LD(Load(HL),A)
  ii.LD(Load(HL()), A()); pc = pc_start + 1; break;
case 0x78:  // 78 1 4 ---- LD(A,B)
  ii.LD(A(), B()); pc = pc_start + 1; break;
case 0x79:  // 79 1 4 ---- LD(A,C)
  ii.LD(A(), C()); pc = pc_start + 1; break;
case 0x7A:  // 7A 1 4 ---- LD(A,D)
  ii.LD(A(), D()); pc = pc_start + 1; break;
case 0x7B:  // 7B 1 4 ---- LD(A,E)
  ii.LD(A(), E()); pc = pc_start + 1; break;
case 0x7C:  // 7C 1 4 ---- LD(A,H)
  ii.LD(A(), H()); pc = pc_start + 1; break;
case 0x7D:  // 7D 1 4 ---- LD(A,L)
  ii.LD(A(), L()); pc = pc_start + 1; break;
case 0x7E:  // 7E 1 8 ---- LD(A,Load(HL))
  ii.LD(A(), Load(HL())); pc = pc_start + 1; break;
case 0x7F:  // 7F 1 4 ---- LD(A,A)
  ii.LD(A(), A()); pc = pc_start + 1; break;
case 0x80:  // 80 1 4 Z0HC ADD(A,B)
  ii.ADD(A(), B()); pc = pc_start + 1; break;
case 0x81:  // 81 1 4 Z0HC ADD(A,C)
  ii.ADD(A(), C()); pc = pc_start + 1; break;
case 0x82:  // 82 1 4 Z0HC ADD(A,D)
  ii.ADD(A(), D()); pc = pc_start + 1; break;
case 0x83:  // 83 1 4 Z0HC ADD(A,E)
  ii.ADD(A(), E()); pc = pc_start + 1; break;
case 0x84:  // 84 1 4 Z0HC ADD(A,H)
  ii.ADD(A(), H()); pc = pc_start + 1; break;
case 0x85:  // 85 1 4 Z0HC ADD(A,L)
  ii.ADD(A(), L()); pc = pc_start + 1; break;
case 0x86:  // 86 1 8 Z0HC ADD(A,Load(HL))
  ii.ADD(A(), Load(HL())); pc = pc_start + 1; break;
case 0x87:  // 87 1 4 Z0HC ADD(A,A)
  ii.ADD(A(), A()); pc = pc_start + 1; break;
case 0x88:  // 88 1 4 Z0HC ADC(A,B)
  ii.ADC(A(), B()); pc = pc_start + 1; break;
case 0x89:  // 89 1 4 Z0HC ADC(A,C)
  ii.ADC(A(), C()); pc = pc_start + 1; break;
case 0x8A:  // 8A 1 4 Z0HC ADC(A,D)
  ii.ADC(A(), D()); pc = pc_start + 1; break;
case 0x8B:  // 8B 1 4 Z0HC ADC(A,E)
  ii.ADC(A(), E()); pc = pc_start + 1; break;
case 0x8C:  // 8C 1 4 Z0HC ADC(A,H)
  ii.ADC(A(), H()); pc = pc_start + 1; break;
case 0x8D:  // 8D 1 4 Z0HC ADC(A,L)
  ii.ADC(A(), L()); pc = pc_start + 1; break;
case 0x8E:  // 8E 1 8 Z0HC ADC(A,Load(HL))
  ii.ADC(A(), Load(HL())); pc = pc_start + 1; break;
case 0x8F:  // 8F 1 4 Z0HC ADC(A,A)
  ii.ADC(A(), A()); pc = pc_start + 1; break;
case 0x90:  // 90 1 4 Z1HC SUB(B)
  ii.SUB(B()); pc = pc_start + 1; break;
case 0x91:  // 91 1 4 Z1HC SUB(C)
  ii.SUB(C()); pc = pc_start + 1; break;
case 0x92:  // 92 1 4 Z1HC SUB(D)
  ii.SUB(D()); pc = pc_start + 1; break;
case 0x93:  // 93 1 4 Z1HC SUB(E)
  ii.SUB(E()); pc = pc_start + 1; break;
case 0x94:  // 94 1 4 Z1HC SUB(H)
  ii.SUB(H()); pc = pc_start + 1; break;
case 0x95:  // 95 1 4 Z1HC SUB(L)
  ii.SUB(L()); pc = pc_start + 1; break;
case 0x96:  // 96 1 8 Z1HC SUB(Load(HL))
  ii.SUB(Load(HL())); pc = pc_start + 1; break;
case 0x97:  // 97 1 4 Z1HC SUB(A)
  ii.SUB(A()); pc = pc_start + 1; break;
case 0x98:  // 98 1 4 Z1HC SBC(B)
  ii.SBC(B()); pc = pc_start + 1; break;
case 0x99:  // 99 1 4 Z1HC SBC(C)
  ii.SBC(C()); pc = pc_start + 1; break;
case 0x9A:  // 9A 1 4 Z1HC SBC(D)
  ii.SBC(D()); pc = pc_start + 1; break;
case 0x9B:  // 9B 1 4 Z1HC SBC(E)
  ii.SBC(E()); pc = pc_start + 1; break;
case 0x9C:  // 9C 1 4 Z1HC SBC(H)
  ii.SBC(H()); pc = pc_start + 1; break;
case 0x9D:  // 9D 1 4 Z1HC SBC(L)
  ii.SBC(L()); pc = pc_start + 1; break;
case 0x9E:  // 9E 1 8 Z1HC SBC(Load(HL))
  ii.SBC(Load(HL())); pc = pc_start + 1; break;
case 0x9F:  // 9F 1 4 Z1HC SBC(A)
  ii.SBC(A()); pc = pc_start + 1; break;
case 0xA0:  // A0 1 4 Z010 AND(B)
  ii.AND(B()); pc = pc_start + 1; break;
case 0xA1:  // A1 1 4 Z010 AND(C)
  ii.AND(C()); pc = pc_start + 1; break;
case 0xA2:  // A2 1 4 Z010 AND(D)
  ii.AND(D()); pc = pc_start + 1; break;
case 0xA3:  // A3 1 4 Z010 AND(E)
  ii.AND(E()); pc = pc_start + 1; break;
case 0xA4:  // A4 1 4 Z010 AND(H)
  ii.AND(H()); pc = pc_start + 1; break;
case 0xA5:  // A5 1 4 Z010 AND(L)
  ii.AND(L()); pc = pc_start + 1; break;
case 0xA6:  // A6 1 8 Z010 AND(Load(HL))
  ii.AND(Load(HL())); pc = pc_start + 1; break;
case 0xA7:  // A7 1 4 Z010 AND(A)
  ii.AND(A()); pc = pc_start + 1; break;
case 0xA8:  // A8 1 4 Z000 XOR(B)
  ii.XOR(B()); pc = pc_start + 1; break;
case 0xA9:  // A9 1 4 Z000 XOR(C)
  ii.XOR(C()); pc = pc_start + 1; break;
case 0xAA:  // AA 1 4 Z000 XOR(D)
  ii.XOR(D()); pc = pc_start + 1; break;
case 0xAB:  // AB 1 4 Z000 XOR(E)
  ii.XOR(E()); pc = pc_start + 1; break;
case 0xAC:  // AC 1 4 Z000 XOR(H)
  ii.XOR(H()); pc = pc_start + 1; break;
case 0xAD:  // AD 1 4 Z000 XOR(L)
  ii.XOR(L()); pc = pc_start + 1; break;
case 0xAE:  // AE 1 8 Z000 XOR(Load(HL))
  ii.XOR(Load(HL())); pc = pc_start + 1; break;
case 0xAF:  // AF 1 4 Z000 XOR(A)
  ii.XOR(A()); pc = pc_start + 1; break;
case 0xB0:  // B0 1 4 Z000 OR(B)
  ii.OR(B()); pc = pc_start + 1; break;
case 0xB1:  // B1 1 4 Z000 OR(C)
  ii.OR(C()); pc = pc_start + 1; break;
case 0xB2:  // B2 1 4 Z000 OR(D)
  ii.OR(D()); pc = pc_start + 1; break;
case 0xB3:  // B3 1 4 Z000 OR(E)
  ii.OR(E()); pc = pc_start + 1; break;
case 0xB4:  // B4 1 4 Z000 OR(H)
  ii.OR(H()); pc = pc_start + 1; break;
case 0xB5:  // B5 1 4 Z000 OR(L)
  ii.OR(L()); pc = pc_start + 1; break;
case 0xB6:  // B6 1 8 Z000 OR(Load(HL))
  ii.OR(Load(HL())); pc = pc_start + 1; break;
case 0xB7:  // B7 1 4 Z000 OR(A)
  ii.OR(A()); pc = pc_start + 1; break;
case 0xB8:  // B8 1 4 Z1HC CP(B)
  ii.CP(B()); pc = pc_start + 1; break;
case 0xB9:  // B9 1 4 Z1HC CP(C)
  ii.CP(C()); pc = pc_start + 1; break;
case 0xBA:  // BA 1 4 Z1HC CP(D)
  ii.CP(D()); pc = pc_start + 1; break;
case 0xBB:  // BB 1 4 Z1HC CP(E)
  ii.CP(E()); pc = pc_start + 1; break;
case 0xBC:  // BC 1 4 Z1HC CP(H)
  ii.CP(H()); pc = pc_start + 1; break;
case 0xBD:  // BD 1 4 Z1HC CP(L)
  ii.CP(L()); pc = pc_start + 1; break;
case 0xBE:  // BE 1 8 Z1HC CP(Load(HL))
  ii.CP(Load(HL())); pc = pc_start + 1; break;
case 0xBF:  // BF 1 4 Z1HC CP(A)
  ii.CP(A()); pc = pc_start + 1; break;
case 0xC0:  // C0 1 20/8 ---- RET(CNZ)
  ii.RET(CNZ()); pc = pc_start + 1; break;
case 0xC1:  // C1 1 12 ---- POP(BC)
  ii.POP(BC()); pc = pc_start + 1; break;
case 0xC2:  // C2 3 16/12 ---- JP(CNZ,Imm16)
  ii.JP(CNZ(), Imm16()); pc = pc_start + 3; break;
case 0xC3:  // C3 3 16 ---- JP(CT,Imm16)
  ii.JP(CT(), Imm16()); pc = pc_start + 3; break;
case 0xC4:  // C4 3 24/12 ---- CALL(CNZ,Imm16)
  ii.CALL(CNZ(), Imm16()); pc = pc_start + 3; break;
case 0xC5:  // C5 1 16 ---- PUSH(BC)
  ii.PUSH(BC()); pc = pc_start + 1; break;
case 0xC6:  // C6 2 8 Z0HC ADD(A,Imm8)
  ii.ADD(A(), Imm8()); pc = pc_start + 2; break;
case 0xC7:  // C7 1 16 ---- RST(0x00)
  ii.RST({OperandType::IMM8, {(u8)0x00}}); pc = pc_start + 1; break;
case 0xC8:  // C8 1 20/8 ---- RET(CZ)
  ii.RET(CZ()); pc = pc_start + 1; break;
case 0xC9:  // C9 1 16 ---- RET(CT)
  ii.RET(CT()); pc = pc_start + 1; break;
case 0xCA:  // CA 3 16/12 ---- JP(CZ,Imm16)
  ii.JP(CZ(), Imm16()); pc = pc_start + 3; break;
case 0xCB:  // CB 1 4 ---- PREFIX___
  _log("    ");
  log("fallback", "PREFIX___"); // fallback
  error = 1;
 pc = pc_start + 1; break;
case 0xCC:  // CC 3 24/12 ---- CALL(CZ,Imm16)
  ii.CALL(CZ(), Imm16()); pc = pc_start + 3; break;
case 0xCD:  // CD 3 24 ---- CALL(CT,Imm16)
  ii.CALL(CT(), Imm16()); pc = pc_start + 3; break;
case 0xCE:  // CE 2 8 Z0HC ADC(A,Imm8)
  ii.ADC(A(), Imm8()); pc = pc_start + 2; break;
case 0xCF:  // CF 1 16 ---- RST(0x08)
  ii.RST({OperandType::IMM8, {(u8)0x08}}); pc = pc_start + 1; break;
case 0xD0:  // D0 1 20/8 ---- RET(CNC)
  ii.RET(CNC()); pc = pc_start + 1; break;
case 0xD1:  // D1 1 12 ---- POP(DE)
  ii.POP(DE()); pc = pc_start + 1; break;
case 0xD2:  // D2 3 16/12 ---- JP(CNC,Imm16)
  ii.JP(CNC(), Imm16()); pc = pc_start + 3; break;
case 0xD4:  // D4 3 24/12 ---- CALL(CNC,Imm16)
  ii.CALL(CNC(), Imm16()); pc = pc_start + 3; break;
case 0xD5:  // D5 1 16 ---- PUSH(DE)
  ii.PUSH(DE()); pc = pc_start + 1; break;
case 0xD6:  // D6 2 8 Z1HC SUB(Imm8)
  ii.SUB(Imm8()); pc = pc_start + 2; break;
case 0xD7:  // D7 1 16 ---- RST(0x10)
  ii.RST({OperandType::IMM8, {(u8)0x10}}); pc = pc_start + 1; break;
case 0xD8:  // D8 1 20/8 ---- RET(CC)
  ii.RET(CC()); pc = pc_start + 1; break;
case 0xD9:  // D9 1 16 ---- RETI(CT)
  ii.RETI(CT()); pc = pc_start + 1; break;
case 0xDA:  // DA 3 16/12 ---- JP(CC,Imm16)
  ii.JP(CC(), Imm16()); pc = pc_start + 3; break;
case 0xDC:  // DC 3 24/12 ---- CALL(CC,Imm16)
  ii.CALL(CC(), Imm16()); pc = pc_start + 3; break;
case 0xDE:  // DE 2 8 Z1HC SBC(Imm8)
  ii.SBC(Imm8()); pc = pc_start + 2; break;
case 0xDF:  // DF 1 16 ---- RST(0x18)
  ii.RST({OperandType::IMM8, {(u8)0x18}}); pc = pc_start + 1; break;
case 0xE0:  // E0 2 12 ---- LD(IO(Imm8),A)
  ii.LD(IO(Imm8()), A()); pc = pc_start + 2; break;
case 0xE1:  // E1 1 12 ---- POP(HL)
  ii.POP(HL()); pc = pc_start + 1; break;
case 0xE2:  // E2 2 8 ---- LD(IO(C),A)
  ii.LD(IO(C()), A()); pc = pc_start + 2; break;
case 0xE5:  // E5 1 16 ---- PUSH(HL)
  ii.PUSH(HL()); pc = pc_start + 1; break;
case 0xE6:  // E6 2 8 Z010 AND(Imm8)
  ii.AND(Imm8()); pc = pc_start + 2; break;
case 0xE7:  // E7 1 16 ---- RST(0x20)
  ii.RST({OperandType::IMM8, {(u8)0x20}}); pc = pc_start + 1; break;
case 0xE8:  // E8 2 16 00HC ADD(SP,ImmI8)
  ii.ADD(SP(), ImmI8()); pc = pc_start + 2; break;
case 0xE9:  // E9 1 4 ---- JP(CT,Load(HL))
  ii.JP(CT(), Load(HL())); pc = pc_start + 1; break;
case 0xEA:  // EA 3 16 ---- LD(Load(Imm16),A)
  ii.LD(Load(Imm16()), A()); pc = pc_start + 3; break;
case 0xEE:  // EE 2 8 Z000 XOR(Imm8)
  ii.XOR(Imm8()); pc = pc_start + 2; break;
case 0xEF:  // EF 1 16 ---- RST(0x28)
  ii.RST({OperandType::IMM8, {(u8)0x28}}); pc = pc_start + 1; break;
case 0xF0:  // F0 2 12 ---- LD(A,IO(Imm8))
  ii.LD(A(), IO(Imm8())); pc = pc_start + 2; break;
case 0xF1:  // F1 1 12 ZNHC POP(AF)
  ii.POP(AF()); pc = pc_start + 1; break;
case 0xF2:  // F2 2 8 ---- LD(A,IO(C))
  ii.LD(A(), IO(C())); pc = pc_start + 2; break;
case 0xF3:  // F3 1 4 ---- DI()
  ii.DI(); pc = pc_start + 1; break;
case 0xF5:  // F5 1 16 ---- PUSH(AF)
  ii.PUSH(AF()); pc = pc_start + 1; break;
case 0xF6:  // F6 2 8 Z000 OR(Imm8)
  ii.OR(Imm8()); pc = pc_start + 2; break;
case 0xF7:  // F7 1 16 ---- RST(0x30)
  ii.RST({OperandType::IMM8, {(u8)0x30}}); pc = pc_start + 1; break;
case 0xF8:  // F8 2 12 00HC LD(HL,LoadSP(ImmI8))
  ii.LD(HL(), LoadSP(ImmI8())); pc = pc_start + 2; break;
case 0xF9:  // F9 1 8 ---- LD(SP,HL)
  ii.LD(SP(), HL()); pc = pc_start + 1; break;
case 0xFA:  // FA 3 16 ---- LD(A,Load(Imm16))
  ii.LD(A(), Load(Imm16())); pc = pc_start + 3; break;
case 0xFB:  // FB 1 4 ---- EI()
  ii.EI(); pc = pc_start + 1; break;
case 0xFE:  // FE 2 8 Z1HC CP(Imm8)
  ii.CP(Imm8()); pc = pc_start + 2; break;
case 0xFF:  // FF 1 16 ---- RST(0x38)
  ii.RST({OperandType::IMM8, {(u8)0x38}}); pc = pc_start + 1; break;
case 0x100:  // 100 2 8 Z00C RLC(B)
  ii.RLC(B()); pc = pc_start + 2; break;
case 0x101:  // 101 2 8 Z00C RLC(C)
  ii.RLC(C()); pc = pc_start + 2; break;
case 0x102:  // 102 2 8 Z00C RLC(D)
  ii.RLC(D()); pc = pc_start + 2; break;
case 0x103:  // 103 2 8 Z00C RLC(E)
  ii.RLC(E()); pc = pc_start + 2; break;
case 0x104:  // 104 2 8 Z00C RLC(H)
  ii.RLC(H()); pc = pc_start + 2; break;
case 0x105:  // 105 2 8 Z00C RLC(L)
  ii.RLC(L()); pc = pc_start + 2; break;
case 0x106:  // 106 2 16 Z00C RLC(Load(HL))
  ii.RLC(Load(HL())); pc = pc_start + 2; break;
case 0x107:  // 107 2 8 Z00C RLC(A)
  ii.RLC(A()); pc = pc_start + 2; break;
case 0x108:  // 108 2 8 Z00C RRC(B)
  ii.RRC(B()); pc = pc_start + 2; break;
case 0x109:  // 109 2 8 Z00C RRC(C)
  ii.RRC(C()); pc = pc_start + 2; break;
case 0x10A:  // 10A 2 8 Z00C RRC(D)
  ii.RRC(D()); pc = pc_start + 2; break;
case 0x10B:  // 10B 2 8 Z00C RRC(E)
  ii.RRC(E()); pc = pc_start + 2; break;
case 0x10C:  // 10C 2 8 Z00C RRC(H)
  ii.RRC(H()); pc = pc_start + 2; break;
case 0x10D:  // 10D 2 8 Z00C RRC(L)
  ii.RRC(L()); pc = pc_start + 2; break;
case 0x10E:  // 10E 2 16 Z00C RRC(Load(HL))
  ii.RRC(Load(HL())); pc = pc_start + 2; break;
case 0x10F:  // 10F 2 8 Z00C RRC(A)
  ii.RRC(A()); pc = pc_start + 2; break;
case 0x110:  // 110 2 8 Z00C RL(B)
  ii.RL(B()); pc = pc_start + 2; break;
case 0x111:  // 111 2 8 Z00C RL(C)
  ii.RL(C()); pc = pc_start + 2; break;
case 0x112:  // 112 2 8 Z00C RL(D)
  ii.RL(D()); pc = pc_start + 2; break;
case 0x113:  // 113 2 8 Z00C RL(E)
  ii.RL(E()); pc = pc_start + 2; break;
case 0x114:  // 114 2 8 Z00C RL(H)
  ii.RL(H()); pc = pc_start + 2; break;
case 0x115:  // 115 2 8 Z00C RL(L)
  ii.RL(L()); pc = pc_start + 2; break;
case 0x116:  // 116 2 16 Z00C RL(Load(HL))
  ii.RL(Load(HL())); pc = pc_start + 2; break;
case 0x117:  // 117 2 8 Z00C RL(A)
  ii.RL(A()); pc = pc_start + 2; break;
case 0x118:  // 118 2 8 Z00C RR(B)
  ii.RR(B()); pc = pc_start + 2; break;
case 0x119:  // 119 2 8 Z00C RR(C)
  ii.RR(C()); pc = pc_start + 2; break;
case 0x11A:  // 11A 2 8 Z00C RR(D)
  ii.RR(D()); pc = pc_start + 2; break;
case 0x11B:  // 11B 2 8 Z00C RR(E)
  ii.RR(E()); pc = pc_start + 2; break;
case 0x11C:  // 11C 2 8 Z00C RR(H)
  ii.RR(H()); pc = pc_start + 2; break;
case 0x11D:  // 11D 2 8 Z00C RR(L)
  ii.RR(L()); pc = pc_start + 2; break;
case 0x11E:  // 11E 2 16 Z00C RR(Load(HL))
  ii.RR(Load(HL())); pc = pc_start + 2; break;
case 0x11F:  // 11F 2 8 Z00C RR(A)
  ii.RR(A()); pc = pc_start + 2; break;
case 0x120:  // 120 2 8 Z00C SLA(B)
  ii.SLA(B()); pc = pc_start + 2; break;
case 0x121:  // 121 2 8 Z00C SLA(C)
  ii.SLA(C()); pc = pc_start + 2; break;
case 0x122:  // 122 2 8 Z00C SLA(D)
  ii.SLA(D()); pc = pc_start + 2; break;
case 0x123:  // 123 2 8 Z00C SLA(E)
  ii.SLA(E()); pc = pc_start + 2; break;
case 0x124:  // 124 2 8 Z00C SLA(H)
  ii.SLA(H()); pc = pc_start + 2; break;
case 0x125:  // 125 2 8 Z00C SLA(L)
  ii.SLA(L()); pc = pc_start + 2; break;
case 0x126:  // 126 2 16 Z00C SLA(Load(HL))
  ii.SLA(Load(HL())); pc = pc_start + 2; break;
case 0x127:  // 127 2 8 Z00C SLA(A)
  ii.SLA(A()); pc = pc_start + 2; break;
case 0x128:  // 128 2 8 Z000 SRA(B)
  ii.SRA(B()); pc = pc_start + 2; break;
case 0x129:  // 129 2 8 Z000 SRA(C)
  ii.SRA(C()); pc = pc_start + 2; break;
case 0x12A:  // 12A 2 8 Z000 SRA(D)
  ii.SRA(D()); pc = pc_start + 2; break;
case 0x12B:  // 12B 2 8 Z000 SRA(E)
  ii.SRA(E()); pc = pc_start + 2; break;
case 0x12C:  // 12C 2 8 Z000 SRA(H)
  ii.SRA(H()); pc = pc_start + 2; break;
case 0x12D:  // 12D 2 8 Z000 SRA(L)
  ii.SRA(L()); pc = pc_start + 2; break;
case 0x12E:  // 12E 2 16 Z000 SRA(Load(HL))
  ii.SRA(Load(HL())); pc = pc_start + 2; break;
case 0x12F:  // 12F 2 8 Z000 SRA(A)
  ii.SRA(A()); pc = pc_start + 2; break;
case 0x130:  // 130 2 8 Z000 SWAP(B)
  ii.SWAP(B()); pc = pc_start + 2; break;
case 0x131:  // 131 2 8 Z000 SWAP(C)
  ii.SWAP(C()); pc = pc_start + 2; break;
case 0x132:  // 132 2 8 Z000 SWAP(D)
  ii.SWAP(D()); pc = pc_start + 2; break;
case 0x133:  // 133 2 8 Z000 SWAP(E)
  ii.SWAP(E()); pc = pc_start + 2; break;
case 0x134:  // 134 2 8 Z000 SWAP(H)
  ii.SWAP(H()); pc = pc_start + 2; break;
case 0x135:  // 135 2 8 Z000 SWAP(L)
  ii.SWAP(L()); pc = pc_start + 2; break;
case 0x136:  // 136 2 16 Z000 SWAP(Load(HL))
  ii.SWAP(Load(HL())); pc = pc_start + 2; break;
case 0x137:  // 137 2 8 Z000 SWAP(A)
  ii.SWAP(A()); pc = pc_start + 2; break;
case 0x138:  // 138 2 8 Z00C SRL(B)
  ii.SRL(B()); pc = pc_start + 2; break;
case 0x139:  // 139 2 8 Z00C SRL(C)
  ii.SRL(C()); pc = pc_start + 2; break;
case 0x13A:  // 13A 2 8 Z00C SRL(D)
  ii.SRL(D()); pc = pc_start + 2; break;
case 0x13B:  // 13B 2 8 Z00C SRL(E)
  ii.SRL(E()); pc = pc_start + 2; break;
case 0x13C:  // 13C 2 8 Z00C SRL(H)
  ii.SRL(H()); pc = pc_start + 2; break;
case 0x13D:  // 13D 2 8 Z00C SRL(L)
  ii.SRL(L()); pc = pc_start + 2; break;
case 0x13E:  // 13E 2 16 Z00C SRL(Load(HL))
  ii.SRL(Load(HL())); pc = pc_start + 2; break;
case 0x13F:  // 13F 2 8 Z00C SRL(A)
  ii.SRL(A()); pc = pc_start + 2; break;
case 0x140:  // 140 2 8 Z01- BIT(0,B)
  ii.BIT({OperandType::IMM8, {(u8)0}}, B()); pc = pc_start + 2; break;
case 0x141:  // 141 2 8 Z01- BIT(0,C)
  ii.BIT({OperandType::IMM8, {(u8)0}}, C()); pc = pc_start + 2; break;
case 0x142:  // 142 2 8 Z01- BIT(0,D)
  ii.BIT({OperandType::IMM8, {(u8)0}}, D()); pc = pc_start + 2; break;
case 0x143:  // 143 2 8 Z01- BIT(0,E)
  ii.BIT({OperandType::IMM8, {(u8)0}}, E()); pc = pc_start + 2; break;
case 0x144:  // 144 2 8 Z01- BIT(0,H)
  ii.BIT({OperandType::IMM8, {(u8)0}}, H()); pc = pc_start + 2; break;
case 0x145:  // 145 2 8 Z01- BIT(0,L)
  ii.BIT({OperandType::IMM8, {(u8)0}}, L()); pc = pc_start + 2; break;
case 0x146:  // 146 2 16 Z01- BIT(0,Load(HL))
  ii.BIT({OperandType::IMM8, {(u8)0}}, Load(HL())); pc = pc_start + 2; break;
case 0x147:  // 147 2 8 Z01- BIT(0,A)
  ii.BIT({OperandType::IMM8, {(u8)0}}, A()); pc = pc_start + 2; break;
case 0x148:  // 148 2 8 Z01- BIT(1,B)
  ii.BIT({OperandType::IMM8, {(u8)1}}, B()); pc = pc_start + 2; break;
case 0x149:  // 149 2 8 Z01- BIT(1,C)
  ii.BIT({OperandType::IMM8, {(u8)1}}, C()); pc = pc_start + 2; break;
case 0x14A:  // 14A 2 8 Z01- BIT(1,D)
  ii.BIT({OperandType::IMM8, {(u8)1}}, D()); pc = pc_start + 2; break;
case 0x14B:  // 14B 2 8 Z01- BIT(1,E)
  ii.BIT({OperandType::IMM8, {(u8)1}}, E()); pc = pc_start + 2; break;
case 0x14C:  // 14C 2 8 Z01- BIT(1,H)
  ii.BIT({OperandType::IMM8, {(u8)1}}, H()); pc = pc_start + 2; break;
case 0x14D:  // 14D 2 8 Z01- BIT(1,L)
  ii.BIT({OperandType::IMM8, {(u8)1}}, L()); pc = pc_start + 2; break;
case 0x14E:  // 14E 2 16 Z01- BIT(1,Load(HL))
  ii.BIT({OperandType::IMM8, {(u8)1}}, Load(HL())); pc = pc_start + 2; break;
case 0x14F:  // 14F 2 8 Z01- BIT(1,A)
  ii.BIT({OperandType::IMM8, {(u8)1}}, A()); pc = pc_start + 2; break;
case 0x150:  // 150 2 8 Z01- BIT(2,B)
  ii.BIT({OperandType::IMM8, {(u8)2}}, B()); pc = pc_start + 2; break;
case 0x151:  // 151 2 8 Z01- BIT(2,C)
  ii.BIT({OperandType::IMM8, {(u8)2}}, C()); pc = pc_start + 2; break;
case 0x152:  // 152 2 8 Z01- BIT(2,D)
  ii.BIT({OperandType::IMM8, {(u8)2}}, D()); pc = pc_start + 2; break;
case 0x153:  // 153 2 8 Z01- BIT(2,E)
  ii.BIT({OperandType::IMM8, {(u8)2}}, E()); pc = pc_start + 2; break;
case 0x154:  // 154 2 8 Z01- BIT(2,H)
  ii.BIT({OperandType::IMM8, {(u8)2}}, H()); pc = pc_start + 2; break;
case 0x155:  // 155 2 8 Z01- BIT(2,L)
  ii.BIT({OperandType::IMM8, {(u8)2}}, L()); pc = pc_start + 2; break;
case 0x156:  // 156 2 16 Z01- BIT(2,Load(HL))
  ii.BIT({OperandType::IMM8, {(u8)2}}, Load(HL())); pc = pc_start + 2; break;
case 0x157:  // 157 2 8 Z01- BIT(2,A)
  ii.BIT({OperandType::IMM8, {(u8)2}}, A()); pc = pc_start + 2; break;
case 0x158:  // 158 2 8 Z01- BIT(3,B)
  ii.BIT({OperandType::IMM8, {(u8)3}}, B()); pc = pc_start + 2; break;
case 0x159:  // 159 2 8 Z01- BIT(3,C)
  ii.BIT({OperandType::IMM8, {(u8)3}}, C()); pc = pc_start + 2; break;
case 0x15A:  // 15A 2 8 Z01- BIT(3,D)
  ii.BIT({OperandType::IMM8, {(u8)3}}, D()); pc = pc_start + 2; break;
case 0x15B:  // 15B 2 8 Z01- BIT(3,E)
  ii.BIT({OperandType::IMM8, {(u8)3}}, E()); pc = pc_start + 2; break;
case 0x15C:  // 15C 2 8 Z01- BIT(3,H)
  ii.BIT({OperandType::IMM8, {(u8)3}}, H()); pc = pc_start + 2; break;
case 0x15D:  // 15D 2 8 Z01- BIT(3,L)
  ii.BIT({OperandType::IMM8, {(u8)3}}, L()); pc = pc_start + 2; break;
case 0x15E:  // 15E 2 16 Z01- BIT(3,Load(HL))
  ii.BIT({OperandType::IMM8, {(u8)3}}, Load(HL())); pc = pc_start + 2; break;
case 0x15F:  // 15F 2 8 Z01- BIT(3,A)
  ii.BIT({OperandType::IMM8, {(u8)3}}, A()); pc = pc_start + 2; break;
case 0x160:  // 160 2 8 Z01- BIT(4,B)
  ii.BIT({OperandType::IMM8, {(u8)4}}, B()); pc = pc_start + 2; break;
case 0x161:  // 161 2 8 Z01- BIT(4,C)
  ii.BIT({OperandType::IMM8, {(u8)4}}, C()); pc = pc_start + 2; break;
case 0x162:  // 162 2 8 Z01- BIT(4,D)
  ii.BIT({OperandType::IMM8, {(u8)4}}, D()); pc = pc_start + 2; break;
case 0x163:  // 163 2 8 Z01- BIT(4,E)
  ii.BIT({OperandType::IMM8, {(u8)4}}, E()); pc = pc_start + 2; break;
case 0x164:  // 164 2 8 Z01- BIT(4,H)
  ii.BIT({OperandType::IMM8, {(u8)4}}, H()); pc = pc_start + 2; break;
case 0x165:  // 165 2 8 Z01- BIT(4,L)
  ii.BIT({OperandType::IMM8, {(u8)4}}, L()); pc = pc_start + 2; break;
case 0x166:  // 166 2 16 Z01- BIT(4,Load(HL))
  ii.BIT({OperandType::IMM8, {(u8)4}}, Load(HL())); pc = pc_start + 2; break;
case 0x167:  // 167 2 8 Z01- BIT(4,A)
  ii.BIT({OperandType::IMM8, {(u8)4}}, A()); pc = pc_start + 2; break;
case 0x168:  // 168 2 8 Z01- BIT(5,B)
  ii.BIT({OperandType::IMM8, {(u8)5}}, B()); pc = pc_start + 2; break;
case 0x169:  // 169 2 8 Z01- BIT(5,C)
  ii.BIT({OperandType::IMM8, {(u8)5}}, C()); pc = pc_start + 2; break;
case 0x16A:  // 16A 2 8 Z01- BIT(5,D)
  ii.BIT({OperandType::IMM8, {(u8)5}}, D()); pc = pc_start + 2; break;
case 0x16B:  // 16B 2 8 Z01- BIT(5,E)
  ii.BIT({OperandType::IMM8, {(u8)5}}, E()); pc = pc_start + 2; break;
case 0x16C:  // 16C 2 8 Z01- BIT(5,H)
  ii.BIT({OperandType::IMM8, {(u8)5}}, H()); pc = pc_start + 2; break;
case 0x16D:  // 16D 2 8 Z01- BIT(5,L)
  ii.BIT({OperandType::IMM8, {(u8)5}}, L()); pc = pc_start + 2; break;
case 0x16E:  // 16E 2 16 Z01- BIT(5,Load(HL))
  ii.BIT({OperandType::IMM8, {(u8)5}}, Load(HL())); pc = pc_start + 2; break;
case 0x16F:  // 16F 2 8 Z01- BIT(5,A)
  ii.BIT({OperandType::IMM8, {(u8)5}}, A()); pc = pc_start + 2; break;
case 0x170:  // 170 2 8 Z01- BIT(6,B)
  ii.BIT({OperandType::IMM8, {(u8)6}}, B()); pc = pc_start + 2; break;
case 0x171:  // 171 2 8 Z01- BIT(6,C)
  ii.BIT({OperandType::IMM8, {(u8)6}}, C()); pc = pc_start + 2; break;
case 0x172:  // 172 2 8 Z01- BIT(6,D)
  ii.BIT({OperandType::IMM8, {(u8)6}}, D()); pc = pc_start + 2; break;
case 0x173:  // 173 2 8 Z01- BIT(6,E)
  ii.BIT({OperandType::IMM8, {(u8)6}}, E()); pc = pc_start + 2; break;
case 0x174:  // 174 2 8 Z01- BIT(6,H)
  ii.BIT({OperandType::IMM8, {(u8)6}}, H()); pc = pc_start + 2; break;
case 0x175:  // 175 2 8 Z01- BIT(6,L)
  ii.BIT({OperandType::IMM8, {(u8)6}}, L()); pc = pc_start + 2; break;
case 0x176:  // 176 2 16 Z01- BIT(6,Load(HL))
  ii.BIT({OperandType::IMM8, {(u8)6}}, Load(HL())); pc = pc_start + 2; break;
case 0x177:  // 177 2 8 Z01- BIT(6,A)
  ii.BIT({OperandType::IMM8, {(u8)6}}, A()); pc = pc_start + 2; break;
case 0x178:  // 178 2 8 Z01- BIT(7,B)
  ii.BIT({OperandType::IMM8, {(u8)7}}, B()); pc = pc_start + 2; break;
case 0x179:  // 179 2 8 Z01- BIT(7,C)
  ii.BIT({OperandType::IMM8, {(u8)7}}, C()); pc = pc_start + 2; break;
case 0x17A:  // 17A 2 8 Z01- BIT(7,D)
  ii.BIT({OperandType::IMM8, {(u8)7}}, D()); pc = pc_start + 2; break;
case 0x17B:  // 17B 2 8 Z01- BIT(7,E)
  ii.BIT({OperandType::IMM8, {(u8)7}}, E()); pc = pc_start + 2; break;
case 0x17C:  // 17C 2 8 Z01- BIT(7,H)
  ii.BIT({OperandType::IMM8, {(u8)7}}, H()); pc = pc_start + 2; break;
case 0x17D:  // 17D 2 8 Z01- BIT(7,L)
  ii.BIT({OperandType::IMM8, {(u8)7}}, L()); pc = pc_start + 2; break;
case 0x17E:  // 17E 2 16 Z01- BIT(7,Load(HL))
  ii.BIT({OperandType::IMM8, {(u8)7}}, Load(HL())); pc = pc_start + 2; break;
case 0x17F:  // 17F 2 8 Z01- BIT(7,A)
  ii.BIT({OperandType::IMM8, {(u8)7}}, A()); pc = pc_start + 2; break;
case 0x180:  // 180 2 8 ---- RES(0,B)
  ii.RES({OperandType::IMM8, {(u8)0}}, B()); pc = pc_start + 2; break;
case 0x181:  // 181 2 8 ---- RES(0,C)
  ii.RES({OperandType::IMM8, {(u8)0}}, C()); pc = pc_start + 2; break;
case 0x182:  // 182 2 8 ---- RES(0,D)
  ii.RES({OperandType::IMM8, {(u8)0}}, D()); pc = pc_start + 2; break;
case 0x183:  // 183 2 8 ---- RES(0,E)
  ii.RES({OperandType::IMM8, {(u8)0}}, E()); pc = pc_start + 2; break;
case 0x184:  // 184 2 8 ---- RES(0,H)
  ii.RES({OperandType::IMM8, {(u8)0}}, H()); pc = pc_start + 2; break;
case 0x185:  // 185 2 8 ---- RES(0,L)
  ii.RES({OperandType::IMM8, {(u8)0}}, L()); pc = pc_start + 2; break;
case 0x186:  // 186 2 16 ---- RES(0,Load(HL))
  ii.RES({OperandType::IMM8, {(u8)0}}, Load(HL())); pc = pc_start + 2; break;
case 0x187:  // 187 2 8 ---- RES(0,A)
  ii.RES({OperandType::IMM8, {(u8)0}}, A()); pc = pc_start + 2; break;
case 0x188:  // 188 2 8 ---- RES(1,B)
  ii.RES({OperandType::IMM8, {(u8)1}}, B()); pc = pc_start + 2; break;
case 0x189:  // 189 2 8 ---- RES(1,C)
  ii.RES({OperandType::IMM8, {(u8)1}}, C()); pc = pc_start + 2; break;
case 0x18A:  // 18A 2 8 ---- RES(1,D)
  ii.RES({OperandType::IMM8, {(u8)1}}, D()); pc = pc_start + 2; break;
case 0x18B:  // 18B 2 8 ---- RES(1,E)
  ii.RES({OperandType::IMM8, {(u8)1}}, E()); pc = pc_start + 2; break;
case 0x18C:  // 18C 2 8 ---- RES(1,H)
  ii.RES({OperandType::IMM8, {(u8)1}}, H()); pc = pc_start + 2; break;
case 0x18D:  // 18D 2 8 ---- RES(1,L)
  ii.RES({OperandType::IMM8, {(u8)1}}, L()); pc = pc_start + 2; break;
case 0x18E:  // 18E 2 16 ---- RES(1,Load(HL))
  ii.RES({OperandType::IMM8, {(u8)1}}, Load(HL())); pc = pc_start + 2; break;
case 0x18F:  // 18F 2 8 ---- RES(1,A)
  ii.RES({OperandType::IMM8, {(u8)1}}, A()); pc = pc_start + 2; break;
case 0x190:  // 190 2 8 ---- RES(2,B)
  ii.RES({OperandType::IMM8, {(u8)2}}, B()); pc = pc_start + 2; break;
case 0x191:  // 191 2 8 ---- RES(2,C)
  ii.RES({OperandType::IMM8, {(u8)2}}, C()); pc = pc_start + 2; break;
case 0x192:  // 192 2 8 ---- RES(2,D)
  ii.RES({OperandType::IMM8, {(u8)2}}, D()); pc = pc_start + 2; break;
case 0x193:  // 193 2 8 ---- RES(2,E)
  ii.RES({OperandType::IMM8, {(u8)2}}, E()); pc = pc_start + 2; break;
case 0x194:  // 194 2 8 ---- RES(2,H)
  ii.RES({OperandType::IMM8, {(u8)2}}, H()); pc = pc_start + 2; break;
case 0x195:  // 195 2 8 ---- RES(2,L)
  ii.RES({OperandType::IMM8, {(u8)2}}, L()); pc = pc_start + 2; break;
case 0x196:  // 196 2 16 ---- RES(2,Load(HL))
  ii.RES({OperandType::IMM8, {(u8)2}}, Load(HL())); pc = pc_start + 2; break;
case 0x197:  // 197 2 8 ---- RES(2,A)
  ii.RES({OperandType::IMM8, {(u8)2}}, A()); pc = pc_start + 2; break;
case 0x198:  // 198 2 8 ---- RES(3,B)
  ii.RES({OperandType::IMM8, {(u8)3}}, B()); pc = pc_start + 2; break;
case 0x199:  // 199 2 8 ---- RES(3,C)
  ii.RES({OperandType::IMM8, {(u8)3}}, C()); pc = pc_start + 2; break;
case 0x19A:  // 19A 2 8 ---- RES(3,D)
  ii.RES({OperandType::IMM8, {(u8)3}}, D()); pc = pc_start + 2; break;
case 0x19B:  // 19B 2 8 ---- RES(3,E)
  ii.RES({OperandType::IMM8, {(u8)3}}, E()); pc = pc_start + 2; break;
case 0x19C:  // 19C 2 8 ---- RES(3,H)
  ii.RES({OperandType::IMM8, {(u8)3}}, H()); pc = pc_start + 2; break;
case 0x19D:  // 19D 2 8 ---- RES(3,L)
  ii.RES({OperandType::IMM8, {(u8)3}}, L()); pc = pc_start + 2; break;
case 0x19E:  // 19E 2 16 ---- RES(3,Load(HL))
  ii.RES({OperandType::IMM8, {(u8)3}}, Load(HL())); pc = pc_start + 2; break;
case 0x19F:  // 19F 2 8 ---- RES(3,A)
  ii.RES({OperandType::IMM8, {(u8)3}}, A()); pc = pc_start + 2; break;
case 0x1A0:  // 1A0 2 8 ---- RES(4,B)
  ii.RES({OperandType::IMM8, {(u8)4}}, B()); pc = pc_start + 2; break;
case 0x1A1:  // 1A1 2 8 ---- RES(4,C)
  ii.RES({OperandType::IMM8, {(u8)4}}, C()); pc = pc_start + 2; break;
case 0x1A2:  // 1A2 2 8 ---- RES(4,D)
  ii.RES({OperandType::IMM8, {(u8)4}}, D()); pc = pc_start + 2; break;
case 0x1A3:  // 1A3 2 8 ---- RES(4,E)
  ii.RES({OperandType::IMM8, {(u8)4}}, E()); pc = pc_start + 2; break;
case 0x1A4:  // 1A4 2 8 ---- RES(4,H)
  ii.RES({OperandType::IMM8, {(u8)4}}, H()); pc = pc_start + 2; break;
case 0x1A5:  // 1A5 2 8 ---- RES(4,L)
  ii.RES({OperandType::IMM8, {(u8)4}}, L()); pc = pc_start + 2; break;
case 0x1A6:  // 1A6 2 16 ---- RES(4,Load(HL))
  ii.RES({OperandType::IMM8, {(u8)4}}, Load(HL())); pc = pc_start + 2; break;
case 0x1A7:  // 1A7 2 8 ---- RES(4,A)
  ii.RES({OperandType::IMM8, {(u8)4}}, A()); pc = pc_start + 2; break;
case 0x1A8:  // 1A8 2 8 ---- RES(5,B)
  ii.RES({OperandType::IMM8, {(u8)5}}, B()); pc = pc_start + 2; break;
case 0x1A9:  // 1A9 2 8 ---- RES(5,C)
  ii.RES({OperandType::IMM8, {(u8)5}}, C()); pc = pc_start + 2; break;
case 0x1AA:  // 1AA 2 8 ---- RES(5,D)
  ii.RES({OperandType::IMM8, {(u8)5}}, D()); pc = pc_start + 2; break;
case 0x1AB:  // 1AB 2 8 ---- RES(5,E)
  ii.RES({OperandType::IMM8, {(u8)5}}, E()); pc = pc_start + 2; break;
case 0x1AC:  // 1AC 2 8 ---- RES(5,H)
  ii.RES({OperandType::IMM8, {(u8)5}}, H()); pc = pc_start + 2; break;
case 0x1AD:  // 1AD 2 8 ---- RES(5,L)
  ii.RES({OperandType::IMM8, {(u8)5}}, L()); pc = pc_start + 2; break;
case 0x1AE:  // 1AE 2 16 ---- RES(5,Load(HL))
  ii.RES({OperandType::IMM8, {(u8)5}}, Load(HL())); pc = pc_start + 2; break;
case 0x1AF:  // 1AF 2 8 ---- RES(5,A)
  ii.RES({OperandType::IMM8, {(u8)5}}, A()); pc = pc_start + 2; break;
case 0x1B0:  // 1B0 2 8 ---- RES(6,B)
  ii.RES({OperandType::IMM8, {(u8)6}}, B()); pc = pc_start + 2; break;
case 0x1B1:  // 1B1 2 8 ---- RES(6,C)
  ii.RES({OperandType::IMM8, {(u8)6}}, C()); pc = pc_start + 2; break;
case 0x1B2:  // 1B2 2 8 ---- RES(6,D)
  ii.RES({OperandType::IMM8, {(u8)6}}, D()); pc = pc_start + 2; break;
case 0x1B3:  // 1B3 2 8 ---- RES(6,E)
  ii.RES({OperandType::IMM8, {(u8)6}}, E()); pc = pc_start + 2; break;
case 0x1B4:  // 1B4 2 8 ---- RES(6,H)
  ii.RES({OperandType::IMM8, {(u8)6}}, H()); pc = pc_start + 2; break;
case 0x1B5:  // 1B5 2 8 ---- RES(6,L)
  ii.RES({OperandType::IMM8, {(u8)6}}, L()); pc = pc_start + 2; break;
case 0x1B6:  // 1B6 2 16 ---- RES(6,Load(HL))
  ii.RES({OperandType::IMM8, {(u8)6}}, Load(HL())); pc = pc_start + 2; break;
case 0x1B7:  // 1B7 2 8 ---- RES(6,A)
  ii.RES({OperandType::IMM8, {(u8)6}}, A()); pc = pc_start + 2; break;
case 0x1B8:  // 1B8 2 8 ---- RES(7,B)
  ii.RES({OperandType::IMM8, {(u8)7}}, B()); pc = pc_start + 2; break;
case 0x1B9:  // 1B9 2 8 ---- RES(7,C)
  ii.RES({OperandType::IMM8, {(u8)7}}, C()); pc = pc_start + 2; break;
case 0x1BA:  // 1BA 2 8 ---- RES(7,D)
  ii.RES({OperandType::IMM8, {(u8)7}}, D()); pc = pc_start + 2; break;
case 0x1BB:  // 1BB 2 8 ---- RES(7,E)
  ii.RES({OperandType::IMM8, {(u8)7}}, E()); pc = pc_start + 2; break;
case 0x1BC:  // 1BC 2 8 ---- RES(7,H)
  ii.RES({OperandType::IMM8, {(u8)7}}, H()); pc = pc_start + 2; break;
case 0x1BD:  // 1BD 2 8 ---- RES(7,L)
  ii.RES({OperandType::IMM8, {(u8)7}}, L()); pc = pc_start + 2; break;
case 0x1BE:  // 1BE 2 16 ---- RES(7,Load(HL))
  ii.RES({OperandType::IMM8, {(u8)7}}, Load(HL())); pc = pc_start + 2; break;
case 0x1BF:  // 1BF 2 8 ---- RES(7,A)
  ii.RES({OperandType::IMM8, {(u8)7}}, A()); pc = pc_start + 2; break;
case 0x1C0:  // 1C0 2 8 ---- SET(0,B)
  ii.SET({OperandType::IMM8, {(u8)0}}, B()); pc = pc_start + 2; break;
case 0x1C1:  // 1C1 2 8 ---- SET(0,C)
  ii.SET({OperandType::IMM8, {(u8)0}}, C()); pc = pc_start + 2; break;
case 0x1C2:  // 1C2 2 8 ---- SET(0,D)
  ii.SET({OperandType::IMM8, {(u8)0}}, D()); pc = pc_start + 2; break;
case 0x1C3:  // 1C3 2 8 ---- SET(0,E)
  ii.SET({OperandType::IMM8, {(u8)0}}, E()); pc = pc_start + 2; break;
case 0x1C4:  // 1C4 2 8 ---- SET(0,H)
  ii.SET({OperandType::IMM8, {(u8)0}}, H()); pc = pc_start + 2; break;
case 0x1C5:  // 1C5 2 8 ---- SET(0,L)
  ii.SET({OperandType::IMM8, {(u8)0}}, L()); pc = pc_start + 2; break;
case 0x1C6:  // 1C6 2 16 ---- SET(0,Load(HL))
  ii.SET({OperandType::IMM8, {(u8)0}}, Load(HL())); pc = pc_start + 2; break;
case 0x1C7:  // 1C7 2 8 ---- SET(0,A)
  ii.SET({OperandType::IMM8, {(u8)0}}, A()); pc = pc_start + 2; break;
case 0x1C8:  // 1C8 2 8 ---- SET(1,B)
  ii.SET({OperandType::IMM8, {(u8)1}}, B()); pc = pc_start + 2; break;
case 0x1C9:  // 1C9 2 8 ---- SET(1,C)
  ii.SET({OperandType::IMM8, {(u8)1}}, C()); pc = pc_start + 2; break;
case 0x1CA:  // 1CA 2 8 ---- SET(1,D)
  ii.SET({OperandType::IMM8, {(u8)1}}, D()); pc = pc_start + 2; break;
case 0x1CB:  // 1CB 2 8 ---- SET(1,E)
  ii.SET({OperandType::IMM8, {(u8)1}}, E()); pc = pc_start + 2; break;
case 0x1CC:  // 1CC 2 8 ---- SET(1,H)
  ii.SET({OperandType::IMM8, {(u8)1}}, H()); pc = pc_start + 2; break;
case 0x1CD:  // 1CD 2 8 ---- SET(1,L)
  ii.SET({OperandType::IMM8, {(u8)1}}, L()); pc = pc_start + 2; break;
case 0x1CE:  // 1CE 2 16 ---- SET(1,Load(HL))
  ii.SET({OperandType::IMM8, {(u8)1}}, Load(HL())); pc = pc_start + 2; break;
case 0x1CF:  // 1CF 2 8 ---- SET(1,A)
  ii.SET({OperandType::IMM8, {(u8)1}}, A()); pc = pc_start + 2; break;
case 0x1D0:  // 1D0 2 8 ---- SET(2,B)
  ii.SET({OperandType::IMM8, {(u8)2}}, B()); pc = pc_start + 2; break;
case 0x1D1:  // 1D1 2 8 ---- SET(2,C)
  ii.SET({OperandType::IMM8, {(u8)2}}, C()); pc = pc_start + 2; break;
case 0x1D2:  // 1D2 2 8 ---- SET(2,D)
  ii.SET({OperandType::IMM8, {(u8)2}}, D()); pc = pc_start + 2; break;
case 0x1D3:  // 1D3 2 8 ---- SET(2,E)
  ii.SET({OperandType::IMM8, {(u8)2}}, E()); pc = pc_start + 2; break;
case 0x1D4:  // 1D4 2 8 ---- SET(2,H)
  ii.SET({OperandType::IMM8, {(u8)2}}, H()); pc = pc_start + 2; break;
case 0x1D5:  // 1D5 2 8 ---- SET(2,L)
  ii.SET({OperandType::IMM8, {(u8)2}}, L()); pc = pc_start + 2; break;
case 0x1D6:  // 1D6 2 16 ---- SET(2,Load(HL))
  ii.SET({OperandType::IMM8, {(u8)2}}, Load(HL())); pc = pc_start + 2; break;
case 0x1D7:  // 1D7 2 8 ---- SET(2,A)
  ii.SET({OperandType::IMM8, {(u8)2}}, A()); pc = pc_start + 2; break;
case 0x1D8:  // 1D8 2 8 ---- SET(3,B)
  ii.SET({OperandType::IMM8, {(u8)3}}, B()); pc = pc_start + 2; break;
case 0x1D9:  // 1D9 2 8 ---- SET(3,C)
  ii.SET({OperandType::IMM8, {(u8)3}}, C()); pc = pc_start + 2; break;
case 0x1DA:  // 1DA 2 8 ---- SET(3,D)
  ii.SET({OperandType::IMM8, {(u8)3}}, D()); pc = pc_start + 2; break;
case 0x1DB:  // 1DB 2 8 ---- SET(3,E)
  ii.SET({OperandType::IMM8, {(u8)3}}, E()); pc = pc_start + 2; break;
case 0x1DC:  // 1DC 2 8 ---- SET(3,H)
  ii.SET({OperandType::IMM8, {(u8)3}}, H()); pc = pc_start + 2; break;
case 0x1DD:  // 1DD 2 8 ---- SET(3,L)
  ii.SET({OperandType::IMM8, {(u8)3}}, L()); pc = pc_start + 2; break;
case 0x1DE:  // 1DE 2 16 ---- SET(3,Load(HL))
  ii.SET({OperandType::IMM8, {(u8)3}}, Load(HL())); pc = pc_start + 2; break;
case 0x1DF:  // 1DF 2 8 ---- SET(3,A)
  ii.SET({OperandType::IMM8, {(u8)3}}, A()); pc = pc_start + 2; break;
case 0x1E0:  // 1E0 2 8 ---- SET(4,B)
  ii.SET({OperandType::IMM8, {(u8)4}}, B()); pc = pc_start + 2; break;
case 0x1E1:  // 1E1 2 8 ---- SET(4,C)
  ii.SET({OperandType::IMM8, {(u8)4}}, C()); pc = pc_start + 2; break;
case 0x1E2:  // 1E2 2 8 ---- SET(4,D)
  ii.SET({OperandType::IMM8, {(u8)4}}, D()); pc = pc_start + 2; break;
case 0x1E3:  // 1E3 2 8 ---- SET(4,E)
  ii.SET({OperandType::IMM8, {(u8)4}}, E()); pc = pc_start + 2; break;
case 0x1E4:  // 1E4 2 8 ---- SET(4,H)
  ii.SET({OperandType::IMM8, {(u8)4}}, H()); pc = pc_start + 2; break;
case 0x1E5:  // 1E5 2 8 ---- SET(4,L)
  ii.SET({OperandType::IMM8, {(u8)4}}, L()); pc = pc_start + 2; break;
case 0x1E6:  // 1E6 2 16 ---- SET(4,Load(HL))
  ii.SET({OperandType::IMM8, {(u8)4}}, Load(HL())); pc = pc_start + 2; break;
case 0x1E7:  // 1E7 2 8 ---- SET(4,A)
  ii.SET({OperandType::IMM8, {(u8)4}}, A()); pc = pc_start + 2; break;
case 0x1E8:  // 1E8 2 8 ---- SET(5,B)
  ii.SET({OperandType::IMM8, {(u8)5}}, B()); pc = pc_start + 2; break;
case 0x1E9:  // 1E9 2 8 ---- SET(5,C)
  ii.SET({OperandType::IMM8, {(u8)5}}, C()); pc = pc_start + 2; break;
case 0x1EA:  // 1EA 2 8 ---- SET(5,D)
  ii.SET({OperandType::IMM8, {(u8)5}}, D()); pc = pc_start + 2; break;
case 0x1EB:  // 1EB 2 8 ---- SET(5,E)
  ii.SET({OperandType::IMM8, {(u8)5}}, E()); pc = pc_start + 2; break;
case 0x1EC:  // 1EC 2 8 ---- SET(5,H)
  ii.SET({OperandType::IMM8, {(u8)5}}, H()); pc = pc_start + 2; break;
case 0x1ED:  // 1ED 2 8 ---- SET(5,L)
  ii.SET({OperandType::IMM8, {(u8)5}}, L()); pc = pc_start + 2; break;
case 0x1EE:  // 1EE 2 16 ---- SET(5,Load(HL))
  ii.SET({OperandType::IMM8, {(u8)5}}, Load(HL())); pc = pc_start + 2; break;
case 0x1EF:  // 1EF 2 8 ---- SET(5,A)
  ii.SET({OperandType::IMM8, {(u8)5}}, A()); pc = pc_start + 2; break;
case 0x1F0:  // 1F0 2 8 ---- SET(6,B)
  ii.SET({OperandType::IMM8, {(u8)6}}, B()); pc = pc_start + 2; break;
case 0x1F1:  // 1F1 2 8 ---- SET(6,C)
  ii.SET({OperandType::IMM8, {(u8)6}}, C()); pc = pc_start + 2; break;
case 0x1F2:  // 1F2 2 8 ---- SET(6,D)
  ii.SET({OperandType::IMM8, {(u8)6}}, D()); pc = pc_start + 2; break;
case 0x1F3:  // 1F3 2 8 ---- SET(6,E)
  ii.SET({OperandType::IMM8, {(u8)6}}, E()); pc = pc_start + 2; break;
case 0x1F4:  // 1F4 2 8 ---- SET(6,H)
  ii.SET({OperandType::IMM8, {(u8)6}}, H()); pc = pc_start + 2; break;
case 0x1F5:  // 1F5 2 8 ---- SET(6,L)
  ii.SET({OperandType::IMM8, {(u8)6}}, L()); pc = pc_start + 2; break;
case 0x1F6:  // 1F6 2 16 ---- SET(6,Load(HL))
  ii.SET({OperandType::IMM8, {(u8)6}}, Load(HL())); pc = pc_start + 2; break;
case 0x1F7:  // 1F7 2 8 ---- SET(6,A)
  ii.SET({OperandType::IMM8, {(u8)6}}, A()); pc = pc_start + 2; break;
case 0x1F8:  // 1F8 2 8 ---- SET(7,B)
  ii.SET({OperandType::IMM8, {(u8)7}}, B()); pc = pc_start + 2; break;
case 0x1F9:  // 1F9 2 8 ---- SET(7,C)
  ii.SET({OperandType::IMM8, {(u8)7}}, C()); pc = pc_start + 2; break;
case 0x1FA:  // 1FA 2 8 ---- SET(7,D)
  ii.SET({OperandType::IMM8, {(u8)7}}, D()); pc = pc_start + 2; break;
case 0x1FB:  // 1FB 2 8 ---- SET(7,E)
  ii.SET({OperandType::IMM8, {(u8)7}}, E()); pc = pc_start + 2; break;
case 0x1FC:  // 1FC 2 8 ---- SET(7,H)
  ii.SET({OperandType::IMM8, {(u8)7}}, H()); pc = pc_start + 2; break;
case 0x1FD:  // 1FD 2 8 ---- SET(7,L)
  ii.SET({OperandType::IMM8, {(u8)7}}, L()); pc = pc_start + 2; break;
case 0x1FE:  // 1FE 2 16 ---- SET(7,Load(HL))
  ii.SET({OperandType::IMM8, {(u8)7}}, Load(HL())); pc = pc_start + 2; break;
case 0x1FF:  // 1FF 2 8 ---- SET(7,A)
  ii.SET({OperandType::IMM8, {(u8)7}}, A()); pc = pc_start + 2; break;
