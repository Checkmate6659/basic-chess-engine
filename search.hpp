#ifndef SEARCH_H
#define SEARCH_H

#include <iostream>
#include <stdint.h>

#include "chess.hpp"
#include "eval.hpp"
using namespace chess;

extern uint64_t nodes;
#define MAX_DEPTH 127

Value search(Board &board, int depth, Value alpha, Value beta);
Move search_root(Board &board, unsigned alloc_time_ms, int depth);


#endif
