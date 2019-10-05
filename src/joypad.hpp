#include "base.hpp"
#include "memory_mapper.hpp"

enum Buttons {
  RIGHT = 0,
  LEFT,
  UP,
  DOWN,
  A,
  B,
  SELECT,
  START,
};

struct Joypad {
  MemoryMapper &mmu;
  Joypad(MemoryMapper &mmu): mmu(mmu) { }
  u8 buttons = 0xFF;

  void tick() {
    u8 val = mmu.get(IO::JOYP);
    if (val & 0x10) {
      // log("read 20", (buttons >> 4));
      // reading START/SEL/A/B status
      mmu.set(IO::JOYP, (buttons >> 4));
    }
    else if (val & 0x20) {
      // log("read 10", (buttons & 0xF));
      // reading arrow status
      mmu.set(IO::JOYP, (buttons & 0xF));
    }
  }

  void button_down(Buttons button) {
    // turn off the relevant bit if the button is down
    buttons &= ~(1 << (int)button);
    // log("button down", (u16) button, buttons);
  }
  void button_up(Buttons button) {
    // turn on the relevant bit when button is released
    buttons |= (1 << (int)button);
    // log("button up", (u16) button, buttons);
  }
};
  
