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
    int8_t depth;
    uint8_t flags;
    int32_t val;
    uint16_t best;
} HASHE;

#define HASH_SIZE (1<<24) //constant for now (TODO: make it an option!)
HASHE hash_table[HASH_SIZE]; //big hash table!

//look at https://gitlab.com/mhouppin/stash-bot/-/blob/8ec0469cdcef022ee1bc304299f7c0e3e2674652/sources/tt/tt_probe.c
HASHE* ProbeHash(Board &board)
{
    U64 curhash = board.hash();
    HASHE* phashe = &hash_table[curhash % HASH_SIZE];

    if (phashe->key == curhash) //hit
        return phashe; //return entry
    return nullptr; //miss (crashes if gets dereferenced)
}

//extremely basic always replace scheme (doesn't even check if it was the same node previously)
//look at https://gitlab.com/mhouppin/stash-bot/-/blob/8ec0469cdcef022ee1bc304299f7c0e3e2674652/sources/tt/tt_save.c
void RecordHash(Board &board, int8_t depth, int32_t val, uint8_t flags, const Move &best_move)
{
    //unimportant if not doing persistent hash table; but needed when i will do that
    if (val == INT32_MAX || val == -INT32_MAX) return; //don't store panic bogus in TT!

    U64 curhash = board.hash();
    HASHE* phashe = &hash_table[curhash % HASH_SIZE];

    phashe->key = curhash;
    phashe->best = best_move.move();
    phashe->val = val;
    phashe->flags = flags; //TODO: tweak a bit
    phashe->depth = depth;
}

#endif
