//http://web.archive.org/web/20070809015843/www.seanet.com/%7Ebrucemo/topics/hashing.htm
#ifndef TT_H
#define TT_H

#include <cstdlib>
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

uint32_t hash_size;
HASHE* hash_table; //smaller hash table (I was using 2^24 before! that's 400MB!)

//Allocate hash table (size_mb is size of hash table in mb)
//Returns true if allocation succeeded, false if it failed
bool alloc_hash(uint32_t size_mb)
{
    size_mb *= 1000000 / sizeof(HASHE); //now size_mb is number of elements
    free(hash_table);
    hash_table = (HASHE*)malloc(size_mb * sizeof(HASHE));
    if (hash_table != NULL) //check for success
    {
        hash_size = size_mb; //this has to be the number of elements
        return true;
    }
    return false; //failure: return false
}

//look at https://gitlab.com/mhouppin/stash-bot/-/blob/8ec0469cdcef022ee1bc304299f7c0e3e2674652/sources/tt/tt_probe.c
HASHE* ProbeHash(Board &board, int8_t ply)
{
    U64 curhash = board.hash();
    HASHE* phashe = &hash_table[curhash % hash_size];

    if (phashe->key == curhash) //hit
    {
        if (abs(phashe->val) > 32256) //mate score
        {
            //adjust based on ply
            if (phashe->val < 0)
                phashe->val += ply; //new_score = old_score + new_ply - old_ply
            else
                phashe->val -= ply; //same backwards
        }

        return phashe; //return entry
    }

    return nullptr; //miss (crashes if gets dereferenced)
}

//extremely basic always replace scheme (doesn't even check if it was the same node previously)
//look at https://gitlab.com/mhouppin/stash-bot/-/blob/8ec0469cdcef022ee1bc304299f7c0e3e2674652/sources/tt/tt_save.c
void RecordHash(Board &board, int8_t depth, int32_t val, uint8_t flags, const Move &best_move, int8_t ply)
{
    //unimportant if not doing persistent hash table; but needed when i will do that
    if (val == INT32_MAX || val == -INT32_MAX) return; //don't store panic bogus in TT!

    //DO NOT STORE when close to a draw!
    if (board.halfMoveClock() > 60) return;

    if (abs(val) > 32256) //mate score
    {
        //adjust based on ply
        if (val < 0)
            val -= ply; //new_score = old_score + new_ply - old_ply
        else
            val += ply; //same backwards
    }

    U64 curhash = board.hash();
    HASHE* phashe = &hash_table[curhash % hash_size];

    phashe->key = curhash;
    phashe->best = best_move.move();
    phashe->val = val;
    phashe->flags = flags; //TODO: tweak a bit
    phashe->depth = depth;
}

#endif
