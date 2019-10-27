#pragma once

#include "../base.hpp"

struct str {
  u8 * data;
  u32 size = 0;
  str() : data(0), size(0) { }
  str(u8 * data, u32 size) : data(data), size(size) { }
  template<size_t N> str(const char (&s)[N]) : data((u8*)s), size(N - 1) { }
};

str make_str(const char *);

namespace logs {
void _log(str);
}
using namespace logs;
