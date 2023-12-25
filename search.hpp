#ifndef SEARCH_H
#define SEARCH_H

#include <stdint.h>

#include "chess.hpp"
#include "eval.hpp"
using namespace chess;

extern uint64_t nodes;

Value search(Board &board, int depth, Value alpha, Value beta);


#endif
