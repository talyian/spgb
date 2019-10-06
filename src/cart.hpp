#include "base.hpp"
#include "wasm_host.hpp"

struct Cart {
  u8 * data;
  u32 len; // expected to be between 32K and 4M
  Cart(u8 * data, u32 len) : data(data), len(len) {
    if (data == 0) return;
    _log("cart");
    _logs((const char *)data + 0x134, 0x10);
    if (data[0x143] == 0x80) _log("CGB");
    else if (data[0x143] == 0xC0) _log("CBG-only");
    else _log("DMG");
    _log("cart-type");
    _log(data[0x147]);
    _log("rom-size");
    _log(data[0x148]);
    _log("ram-size");
    _log(data[0x149]);
    _showlog();
  }
};
