#pragma once

#include "registers.hh"

struct IO {
  u8 P1; // controller
  bool CtrlR() { return P1 & 0x10; }
  bool CtrlL() { return P1 & 0x20; }
  bool CtrlU() { return P1 & 0x40; }
  bool CtrlD() { return P1 & 0x80; }
  u8 SB; // serial transfer data
  u8 SC; // serial IO control
  void SerialSend(u8 value) { SB = value; SC = 0xF0; /* start transfer */ }
  u8 SerialRead() { return SB; }

} io;
