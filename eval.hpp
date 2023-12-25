#ifndef EVAL_H
#define EVAL_H

#include <stdint.h>

#include "chess.hpp"
using namespace chess;

typedef int32_t Value;

void init_tables();
Value eval(Board board);

#endif
