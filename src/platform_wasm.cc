#include "base.hpp"
#include "platform.hh"
#include "emulator.hpp"

// functions we need to provide since WASM is a freestanding environment

size_t sslen(const char * s) {
  for(size_t i = 0;; i++, s++)
    if (!*s)
      return i;
}

extern "C" void *memset(void *dest, int c, size_t n) {
  u8 * d = (u8 *)dest;
  u8 * e = d + n;
  while(d < e) *d++ = c;
  return dest;
}

extern "C" void *memcpy(void *dest, const void *src, size_t n) {
  u8 * d = (u8 *)dest;
  u8 * s = (u8 *)src;
  u8 * e = d + n;
  while(d < e) *d++ = *s++;
  return dest;
}


extern "C" u8 * memory;
const size_t PAGE_SIZE = 64 * 1024;
u8 *wasm_allocate(size_t size) {
  log("wasm-allocate", (int)size);
  size_t page_start = __builtin_wasm_memory_grow(0, (size - 1) / PAGE_SIZE + 1);
  return memory + PAGE_SIZE * page_start;
}

void * operator new[](size_t size) {
  return wasm_allocate(size);
}
void * operator new(size_t size) {
  return wasm_allocate(size);
}

extern "C" {
  emulator_t * WASM_EXPORT get_emulator() {return new emulator_t {}; }
  void WASM_EXPORT set_breakpoint(emulator_t * e, u16 addr) {e->debug.set_breakpoint(addr);}
  void WASM_EXPORT clear_breakpoint(emulator_t * e, u16 addr) {e->debug.clear_breakpoint(addr);}
  void WASM_EXPORT button_down(emulator_t * e, u16 button) { e->joypad.button_down((Buttons)button); }
  void WASM_EXPORT button_up(emulator_t * e, u16 button) { e->joypad.button_up((Buttons)button); }

  void WASM_EXPORT step_instruction(emulator_t * e) {
    e->printer.pc = e->decoder.pc_start;
    e->printer.decode();
    e->_runner.dump();
    e->single_step();
  }
  
  void WASM_EXPORT continue_instr(emulator_t * e) {
    e->debug.is_debugging = false;
    e->debug.is_stepping = false;
  }
  
  void WASM_EXPORT step_frame(emulator_t * e) {
    #define CLOCK_HZ 8200000
    #define FPS 60
    e->step(CLOCK_HZ / FPS);
  }

  u8* WASM_EXPORT get_rom_area(emulator_t * e, u32 len) {
    return new u8[len];
  }
  
  void WASM_EXPORT reset(emulator_t * e, u8 * cart, u32 len) {
    e->load_cart(cart, len);
    e->decoder.pc = 0;
    e->decoder.pc_start = 0;
  }
}
