#ifndef EVAL_H
#define EVAL_H

#include <stdint.h>

#include "chess.hpp"
using namespace chess;

typedef int32_t Value;
#define PANIC_VALUE INT32_MIN

void init_tables();
Value eval(Board board);

#endif
