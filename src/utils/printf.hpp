#include "../base.hpp"
#include "str.hpp"

char output_buf[1024];
char * end = output_buf;

template<class T> void _printf_1(T x);
template<> void _printf_1(const char * x) { while(*x) *end++ = *x++; }
template<> void _printf_1(const str &x) {
  auto p = x.data;
  for(u32 i=0; i < x.size; i++) *end++ = *p++;
}
template<class T> void HEX(u32 N, T x) {
  if (x < 0) { *end++ = '-'; x = -x; }
  end += N;
  for(u32 i=0; i<N; i++) {
    *(--end) = "0123456789ABCDEF"[x % 0x10];
    x /= 0x10; };
  end += N;
}
template<class T> void DEC(T x) {
  if (x < 0) { *end++ = '-'; x = -x; }
  u32 digits = 0;
  T x1 = x;
  while(x1) { digits++; x1 /= 10; }
  digits += !digits;
  
  end += digits;
  for(u32 i=0; i<digits; i++) {
    *(--end) = "0123456789ABCDEF"[x % 10];
    x /= 10; };
  end += digits;
}
template<class T> void FL(T x) {
  if (x < 0) { *end++ = '-'; x = -x; }
  i64 i = x;
  if (i > x) i--;
  DEC(i);
  x -= i;
  *end++ = '.';
  for(int j = 0; j < 6; j++) {
    x *= 10;
    i = x;
    *end++ = '0' + i;
    x -= i;
    if (x < 0.00001) return;
  }
}
template<> void _printf_1(u32 x) { HEX(8, x); }
template<> void _printf_1(i32 x) { HEX(8, x); }
template<> void _printf_1(u64 x) { HEX(8, x); }
template<> void _printf_1(i64 x) { HEX(8, x); }
template<> void _printf_1(f32 x) { FL(x); }
template<> void _printf_1(f64 x) { FL(x); }
template<> void _printf_1(u16 x) { HEX(4, x); }
template<> void _printf_1(u8 x) { HEX(2, x); }
template<> void _printf_1(i8 x) { HEX(2, x); }

void _print_0(str &format) {
  u8 * b = format.data;
  int s = format.size;
  while(s-- > 0) {
    if (*b == '%') {
      b++;
      if (*b == '%') { *end++ = *b++; }
      else break;
    }
    *end++ = *b++;
  }
  format.data = b;
  format.size = s;
}

size_t _print_f(str &format) {
  int s = format.size;
  u8 * b = format.data;
  while(s-- > 0)
    *end++ = *b++;
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
  _print_f(f_copy, xs ...);
  _logs(output_buf, (u32)(end-output_buf));
  _showlog();
  end = output_buf;
  return 0;
}
