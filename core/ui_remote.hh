#pragma once
#include "memory.hh"
#include "registers.hh"

// socket info
#ifdef __linux__
#include "platform_unix.hh"
#elif _WIN32
#include "platform_win32.hh"
#endif

// A Display listening on 33445
struct Display {
  udp_socket sender { "127.0.0.1", "33445" };
  u8 t = 'l';
  u8 buf[160 * 144 / 4];
  void set(u8 x, u8 y, u8 col) {
    u16 u = (u16)y * 160 + x;
    col &= 0x3;
    u8 p4 = 0x1 << (6 - 2 * (u % 4)); // why?
    buf[u / 4] &= ~(3 * p4);
    buf[u / 4] |= col * p4;
  }
  void send() {
    sender.send(&t, 160 * 144 /4 + 1);
  }
} __attribute__((packed));

// A keypad sending to UDP Socket 33446
struct Keypad {
  udp_socket receiver {"127.0.0.1", "33446", true};
  Registers &reg;
  Memory &mem;
  u8 keystate;

  Keypad(Registers &reg, Memory &mem) : reg(reg), mem(mem) {
    keystate = 0xFF;
  }
  u16 stepcounter = 0;
  void Step() {
    u8 msg[2];
    // only read from the buffer once every 4 ms;
    if (stepcounter++ % 4096== 0) {
      if (receiver.read((char *)msg, 2)) {
        keystate = ~msg[1];
        mem[0xFF0F] |= (1 << 4); // Keypad Interrupt
      }
    }
    u8 p14 = !(mem[0xFF00] & 0x10);
    u8 p15 = !(mem[0xFF00] & 0x20);
    if (p14) { mem[0xFF00] = 0x30 | (keystate >> 4); return; }
    if (p15) { mem[0xFF00] = 0x30 | (keystate & 0xF); return; }
  }
};
