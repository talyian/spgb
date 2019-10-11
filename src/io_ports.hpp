#pragma once

#include "base.hpp"

struct IoPorts {
  const static u16 JOYP = 0xFF00;
  const static u16 DMA = 0xFF46;
  u8 data[0x80];
  IoPorts() {
    data[0x4D] = 0xFF;
  }
};
