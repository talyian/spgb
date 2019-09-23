#include "memory.hh"

u8 &Memory::operator [](u16 addr) {
  if (!exit_bios && addr < 0x100) { return bios[addr]; }
  return mem[addr];
}

Memory::Memory(
    FILE * bios_file,
    FILE * cartridge)
  {
    memset(&mem, 0, 0x10000);
    if (bios_file) {
      if (fread(bios, 1, 0x100, bios_file) != 0x100) {
        fprintf(stderr, "could not find bios\n");
        abort();
      }
    }

    if (cartridge) {
      fseek(cartridge, 0, SEEK_END);
      auto buflen = ftell(cartridge);
      fseek(cartridge, 0, 0);
      fread(&mem, 1, (buflen > 0x10000 ? 0x10000 : buflen), cartridge);
    }
    mem[0xFF46] = 0xFF;
}
