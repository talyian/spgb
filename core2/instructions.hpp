#include "base.hpp"

enum OperandType { REG8, REG16, IMM8, IMM16, IO };
struct Operand {
  OperandType type;
  union {
    u8 val8;
    u16 val16;
  };
};

