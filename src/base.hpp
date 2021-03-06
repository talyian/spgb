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

extern "C" void *memset(void *dest, int c, size_t n) throw ();
