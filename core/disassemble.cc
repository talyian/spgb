#include "instructions.hh"
#include "utils_vector.hh"

// void Disassemble(OpParser<OpPrinter> &printer, u16 start, u16 end) {
// }
void Disassemble(OpParser<OpPrinter> &printer) {
  auto &mem = printer.mem;
  auto &reg = printer.registers;
  u8 opRET = 0xC9, opRETI = 0xD9, opCALLn = 0xCD, opJPn = 0xC3, opJPHL = 0xE9;

  reg.PC = 0x100;

  vector<span> visited;
  visited.push_back({0xFF00, 0xFFFF});
  visited.push_back({reg.PC, 0});
  vector<u16> todo;

  todo.push_back(0x40);
  todo.push_back(0x60);
  mem[0xFF50] = 1;
START_FUNC:
  while(true) {
    // if (reg.PC >= 0x100) break;

    printer.Step();

    if (mem[reg._PC] == 0x18 || // JR
        mem[reg._PC] == 0x20 ||
        mem[reg._PC] == 0x28 ||
        mem[reg._PC] == 0x30 ||
        mem[reg._PC] == 0x38) { // conditional JR
      todo.push_back(reg.PC + (int8_t)mem[reg._PC + 1]);
    }

    if (mem[reg._PC] == 0xC2 ||
        mem[reg._PC] == 0xCA ||
        mem[reg._PC] == 0xD2 ||
        mem[reg._PC] == 0xDA || // conditional JP
        mem[reg._PC] == 0xC4 ||
        mem[reg._PC] == 0xCC ||
        mem[reg._PC] == 0xD4 ||
        mem[reg._PC] == 0xDC || // conditional Call
        mem[reg._PC] == opCALLn ||
        mem[reg._PC] == opJPn
      ) {
      todo.push_back(mem[reg._PC+1] + 256 * mem[reg._PC + 2]);
    }

    if (mem[reg._PC] == opRET) break;
    if (mem[reg._PC] == opRETI) break;
    if (mem[reg._PC] == opJPn) break;
    if (mem[reg._PC] == opJPHL) break;
    if (mem[reg._PC] == 0x18) break;
    if (mem[reg._PC] == 0xC7) break; // section: RST calls
    if (mem[reg._PC] == 0xD7) break;
    if (mem[reg._PC] == 0xE7) break;
    if (mem[reg._PC] == 0xF7) break;
    if (mem[reg._PC] == 0xCF) break;
    if (mem[reg._PC] == 0xDF) break;
    if (mem[reg._PC] == 0xEF) break;
    if (mem[reg._PC] == 0xFF) break;
  }
  visited.back().end = reg.PC;
  for(auto && x : todo) {
    bool found = 0;
    for(auto && v : visited)
      if (v.start <= x && x < v.end) { found = true; }
    if (found) {
      visited.push_back(span { x, 0 });
      reg.PC = reg._PC = x;
      goto START_FUNC;
    }
  }
}
