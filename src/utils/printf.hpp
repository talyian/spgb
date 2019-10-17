#include "../str.hpp"
#include <stdio.h>

template<class T> size_t _printf_1(T x);
template<> size_t _printf_1(const char * x) { printf("%s", x); return 0; }
template<> size_t _printf_1(u32 x) { printf("%d", x); return 0; }
template<> size_t _printf_1(i32 x) { printf("%d", x); return 0; }
template<> size_t _printf_1(u64 x) { printf("%lld", x); return 0; }
template<> size_t _printf_1(i64 x) { printf("%lld", x); return 0; }
template<> size_t _printf_1(f32 x) { printf("%f", x); return 0; }
template<> size_t _printf_1(f64 x) { printf("%f", x); return 0; }
template<> size_t _printf_1(u16 x) { printf("%04x", x); return 0; }
template<> size_t _printf_1(u8 x) { printf("%02x", x); return 0; }
template<> size_t _printf_1(i8 x) { printf("%d", x); return 0; }
size_t _print_0(str &format) {
  u8 * b = format.data;
  int s = format.size;
  while(s-- > 0) {
    if (*b == '%') {
      b++;
      if (*b == '%') { }
      else
        break;
    }
    putchar(*b);
    b++;
  }
  format.data = b;
  format.size = s;
  
  return 0;
}

size_t _print_f(str &format) {
  printf("%.*s", format.size, format.data);
  return 0;
}

template<class T>
size_t _print_f(str &f, T x) {
  _print_0(f);
  _printf_1(x);
  _print_f(f);
  return 0;
}

template<class T, class ... TS>
size_t _print_f(str &f, T x, TS ... xs) {
  _print_0(f);
  _printf_1(x);
  _print_f(f, xs ...);
  return 0;
}

template<class ... TS>
size_t print_f(const str f, TS ... xs) {
  str f_copy = f;
  return _print_f(f_copy, xs ...);
}
  
