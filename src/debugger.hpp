#pragma once
#include "base.hpp"
#include "memory_mapper.hpp"

struct Debugger {
  MemoryMapper * mmu;
  u16 pc = 0;
};
