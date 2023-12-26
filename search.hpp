#ifndef SEARCH_H
#define SEARCH_H

//#define __NOISY_DRAW //add a bit of noise to draw score to avoid repetition

#include <iostream>
#include <stdint.h>

#include "chess.hpp"
#include "eval.hpp"
using namespace chess;

#define PANIC_VALUE INT32_MIN
#ifdef __NOISY_DRAW
#define DRAW ((nodes & 3) - 1) //pseudo-random in [[-1, 2]]
#else
#define DRAW 0 //deterministic; more chance of repetition
#endif

extern uint64_t nodes;
#define MAX_DEPTH 127

// void hash_unit_test(Board &board);

Value quiesce(Board &board, Value alpha, Value beta);
Value search(Board &board, int depth, Value alpha, Value beta);
Move search_root(Board &board, int alloc_time_ms, int depth);


#endif
