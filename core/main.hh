#pragma once
#include <cstdio>

#include "registers.hh"
#include "memory.hh"
#include "instructions.hh"

#include <chrono>
#include <thread>
void usleep(int64_t usec) {
  std::this_thread::sleep_for(std::chrono::microseconds(usec));
}

#include <random>
std::random_device rd;
std::int32_t random_int() {
  return rd();
}

void __attribute__((noreturn)) abort(uint32_t i);
