//http://web.archive.org/web/20070809015843/www.seanet.com/%7Ebrucemo/topics/hashing.htm
#ifndef TT_H
#define TT_H

#include <stdint.h>
#include "chess.hpp"
typedef uint64_t U64;
using namespace chess;

#define    hashfEXACT   0
#define    hashfALPHA   1
#define    hashfBETA    2

typedef struct
{
    U64 key;
    uint8_t depth;
    uint8_t flags;
    int32_t val;
    uint16_t best;
} HASHE;

#define HASH_SIZE (1<<24) //constant for now (TODO: make it an option!)
HASHE hash_table[HASH_SIZE]; //big hash table!

int32_t ProbeHash(Board &board, uint8_t depth, int32_t alpha, int32_t beta, Move &tt_move)
{
    U64 curhash = board.hash();
    HASHE* phashe = &hash_table[curhash % HASH_SIZE];

    if (phashe->key == curhash) { //TT hit
        if (phashe->depth >= depth) {
            if (phashe->flags == hashfEXACT)
                return phashe->val;
            if ((phashe->flags == hashfALPHA) && //TODO: window resizing!
                (phashe->val <= alpha))
                return alpha; //TODO: fail soft
            if ((phashe->flags == hashfBETA) &&
                (phashe->val >= beta)) //same
                return beta;
        }
        //this is executed even when we can't return from search immediately
        tt_move = Move(phashe->best); //write best move out of there
    }
    return INT32_MIN; //used as panic value again; returned if cannot replace
}

//extremely basic always replace scheme (doesn't even check if it was the same node previously)
void RecordHash(Board &board, uint8_t depth, int32_t val, uint8_t flags, Move &best_move)
{
    U64 curhash = board.hash();
    HASHE * phashe = &hash_table[curhash % HASH_SIZE];

    phashe->key = curhash;
    phashe->best = best_move.move();
    phashe->val = val;
    phashe->flags = flags;
    phashe->depth = depth;
}

#endif
