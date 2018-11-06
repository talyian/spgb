#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <memory.h>

typedef uint8_t u8;
typedef uint16_t u16;

struct Memory {
  u8 bios[0x100];
  u8 buf[0x10000];
  u8 &bios_unloaded = buf[0xFF50];

  void load(u8 * src, size_t len, size_t start) {
    u8 * dest = buf + start;
    for(auto i=0u; i<len; i++) { *dest++ = *src++; }
  }

  u8 &operator[] (u16 addr) {
    if (!bios_unloaded && addr < 0x100) { return bios[addr]; }
    return buf[addr];
  };

  Memory() {
    memset(buf, 0, sizeof buf);
  }
};
