#pragma once
#include <memory.h>

// simple implementation of a vector
template <class T>
struct vector {
  T * buf = 0;
  size_t size = 0, capacity = 0;
  void push_back(T && x) {
    if (size >= capacity) resize();
    buf[size++] = x;
  }
  void resize() {
    if (capacity > 1) capacity *= 2; else { capacity = 4; }
    buf = (T *)realloc(buf, (1 + capacity) * sizeof (T));
  }
  T &operator [] (size_t idx) { return buf[idx]; }
  T &back() { return buf[size-1]; }
  T &pop_back() { return buf[size--]; }
  T *begin() { return buf; }
  T *end() { return buf + size; }
  T *find(const T & ref) {
    for(T *start = begin(), *stop = end(); start < stop; start++)
      if (*start == ref) return start;
    return end();
  }
  void clear() { size = 0; }
};
