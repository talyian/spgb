#include "base.hpp"
#include "emulator/io_ports.hpp"

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
  Joypad(IoPorts &io) : io(io), JOYP(io.data[0x00]) {}
  IoPorts &io;
  u8 &JOYP;
  u8 buttons = 0xFF;
  void tick() {
    if (JOYP & 0x10) {
      JOYP = buttons >> 4;
    } else if (JOYP & 0x20) {
      JOYP = buttons & 0xF;
    }
  }
  void button_down(Buttons button) { buttons &= ~(1 << (i8)button); }
  void button_up(Buttons button) { buttons |= (1 << (i8)button); }
};
