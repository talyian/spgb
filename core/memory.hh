#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstddef>
typedef uint8_t u8;
typedef uint16_t u16;

struct Memory {

  u8 bios[0x100];
  u8 buf[0x10000];

  bool in_bios = true;

  void load(u8 * src, size_t len, size_t start) {
    u8 * dest = buf + start;
    for(auto i=0u; i<len; i++) { *dest++ = *src++; }
  }

  u8 &operator[] (u16 addr) {
    if (in_bios && addr < 0x100) { return bios[addr]; }
    return buf[addr];
  };
};
