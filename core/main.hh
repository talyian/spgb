#pragma once
#include <cstdio>

#include "registers.hh"
#include "memory.hh"
#include "instructions.hh"

#include <chrono>
#include <thread>
void usleep(int64_t usec);

#include <random>
std::int32_t random_int();

void __attribute__((noreturn)) abort(uint32_t i);
