#include "../gb/lib_gb.hpp"
#include "../gb/emulator.hpp"

#include <stdio.h>

void spgb_push_frame(u32 cat, u8 * data, u32 len) { }
void spgb_logs(const char *, u32 len) { }
void spgb_logx8(u8) { }
void spgb_showlog() { }

u8 serial_out[256];
u8 serial_pos = 0;
void spgb_serial_putc(u8 c) {
  serial_out[serial_pos++] = c;
}
void spgb_logx16(u16) { }
void spgb_stop() { }
void spgb_logf(f64) { }
void spgb_logx32(u32) { }
void spgb_logp(void *) { }
u32  spgb_get_timestamp() { return 0;  }

const char * test_name = 0;
#define TESTCASE(name) test_name = __FUNCTION__;
#define TEST(t){ \
  bool value = t(); \
  success_count += value; \
  test_count += 1;                                        \
  printf("%s    %s\n", test_name, value ? "OK" : "FAIL");}

void run_code(emulator_t &emu, str ss, u32 stop_addr) {
  emu.load_cart(ss.data, ss.size);
  emu._executor.PC = 0;
  emu.mmu.BiosLock = 1;
  while(emu._executor.PC < ss.size && emu._executor.PC != stop_addr) {
    // printf("Running %04x    %02x\n", emu._executor.PC, emu.mmu.get(emu._executor.PC));
    emu.single_step();
  }
}

bool test_simple_ld() {
  TESTCASE("test simple LD");
  emulator_t emu;
  str ss(
    "\xAF" // XOR A
    "\x3C\x3C\x3C" // INC A (x3)
    "\x87\x87\x87\x87" // ADD A,A (x4)
  );
  run_code(emu, ss, -1);
  return emu.cpu.registers.A == 0x30;
}

bool test_push_ret() {
  TESTCASE("test simple LD");
  emulator_t emu;
  run_code(emu,
           "\xAF" // XOR A
           "\x01\x12\x00" //2BC = 0x12
           "\xC5" // PUSH BC
           "\x01\x10\x00" // BC = 0x10
           "\xC5" // PUSH BC
           "\xC5" // PUSH BC
           "\x0\x0\x0\x0\x0\x0" // nops
           "\x3C" // INC A
           "\xC9" // RET
           "\x04" // INC B 
           , -1);
  if (emu.mmu.get(0x10) != 0x3C) return false;
  return emu.cpu.registers.A == 3 && emu.cpu.registers.B == 1;
}

int main() {
  u32 test_count = 0;
  u32 success_count = 0;

  TEST(test_simple_ld);
  TEST(test_push_ret);

  printf("\n%d/%d passed\n", success_count, test_count);
  return success_count - test_count;
}
