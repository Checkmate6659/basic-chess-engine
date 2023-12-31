#ifndef SEARCH_H
#define SEARCH_H

//#define __NOISY_DRAW //add a bit of noise to draw score to avoid repetition

#include <iostream>
#include <cstdint>
#include <ctime>

#include "chess.hpp"
#include "eval.hpp"
using namespace chess;

#define PANIC_VALUE INT32_MAX
#ifdef __NOISY_DRAW
#define DRAW ((nodes & 3) - 1) //pseudo-random in [[-1, 2]]
#else
#define DRAW 0 //deterministic; more chance of repetition
#endif

extern uint64_t nodes;
#define MAX_DEPTH 96 //can't be as high as 127! otherwise we can get infinite-looped!

typedef struct {
    int8_t ply;
} SearchStack;

bool alloc_hash(uint32_t size_mb); //repeating alloc_hash prototype here, to use in main
void clear_hash(); //WARNING: this costs a lot of time, as it clears the entire TT!
void clear_small_tables(); //killers, hist

Value quiesce(Board &board, Value alpha, Value beta, SearchStack* ss);
Value search(Board &board, int depth, Value alpha, Value beta);
Move search_root(Board &board, int alloc_time_ms, int depth);


#endif
