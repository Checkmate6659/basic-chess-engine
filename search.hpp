#ifndef SEARCH_H
#define SEARCH_H

#include <stdint.h>

#include "chess.hpp"
#include "eval.hpp"
using namespace chess;

uint64_t search(Board &board, int depth);


#endif
