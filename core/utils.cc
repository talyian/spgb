#include <cstdlib>
#include <cstdint>

void __attribute__((noreturn)) abort(uint32_t i) {
  printf("abort %d\n", i);
  exit(1);
}
