#pragma once

#include "../base.hpp"

struct IoPorts {
  const static u16 JOYP = 0xFF00;
  const static u16 DMA = 0xFF46;
  const static u16 IF = 0xFF0F;
  const static u16 IE = 0xFFFF;
  u8 data[0x80];

  IoPorts() {
    memset(data, 0, sizeof(data));
    data[0x4D] = 0xFF;
  }
};
