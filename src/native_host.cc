#include "base.hpp"
#include "wasm_host.hpp"
#include <stdio.h>

// imports
extern "C" {
  void _logf(double v) { printf("%f ", v); }
  void _logx8(u8 v) { printf("%02x ", v); }
  void _logx16(u16 v) { printf("%04x ", v); }
  void _logs(const char * s, u32 len) { printf("%.*s ", len, s); }
  void _showlog() { printf("\n"); }
}

int main() {
  emu_main();
}
