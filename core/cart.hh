
#include "memory.hh"

struct CartHeader {
  u8 entry_point[4];
  u8 nintendo_logo[0x30];
  u8 title[15];
  u8 is_color_gb; // 0x80 for color
  u8 licensee[2];
  u8 is_super_gb; // 0x3 for super, 0x0 for game boy
  u8 cartridge_type;
  u8 rom_size;
  u8 ram_size;
  u8 destination_code;
  u8 licensee_code;
  u8 mask_rom_version;
  u8 complement_check;
  u8 checksum[2];
};
