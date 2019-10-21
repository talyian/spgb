#pragma once

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef char i8;
typedef short i16;
typedef int i32;
typedef long long i64;

typedef float f32;
typedef double f64;

using size_t = decltype(sizeof(char));

// the `flag<offset> struct` allows us to access
// a single bit inside of the F register like a boolean.
// "fl.Z = 1; fl.C = 1;" is equivalent to "registers.F & 0b10010000";
template<int offset> struct bit {
  u8 &r;
  bit(bit &&) = delete;
  bit(bit &) = delete;
  bit(u8 & _register) : r(_register) { }
  void operator=(bool n) { r ^= (((r >> offset) & 1) ^ n) << offset; }
  operator bool() { 
    return (r >> offset) & 1; 
  }
};
