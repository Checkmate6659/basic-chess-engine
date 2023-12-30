#include <cstdint>
#include <ctime>
#include <termios.h>
#include "chess.hpp"
#include "search.hpp"
#include "eval.hpp"
#include "order.hpp" //move scoring
#include "posix.hpp" //kbhit equivalent on linux
#include "tt.hpp"

uint64_t nodes = 0;
clock_t search_end_time;
bool panic = false;

Move killers[MAX_DEPTH][2];

//clear hash table AND killers
void clear_hash()
{
    for (int i = 0; i < MAX_DEPTH; i++) killers[i][0] = killers[i][1] = Move::NO_MOVE;
    for (int i = 0; i < hash_size; i++) hash_table[i] = {};
}

Value quiesce(Board &board, Value alpha, Value beta)
{
    //stand pat
    Value static_eval = eval(board);
    if (static_eval > alpha)
    {
        alpha = static_eval;
        if (alpha >= beta)
            return alpha; //no effect of fail soft here
    }

    Movelist moves;
    //only generate captures
    movegen::legalmoves<movegen::MoveGenType::CAPTURE>(moves, board);

    score_moves_quiesce(board, moves);
    for (int i = 0; i < moves.size(); i++) {
        pick_move(moves, i); //get the best-scored move to the index i
        const auto move = moves[i];

        board.makeMove(move);
        nodes++; //1 move made = 1 node
        Value cur_score = -quiesce(board, -beta, -alpha);
        board.unmakeMove(move);

        if (cur_score > alpha)
        {
            alpha = cur_score;
            if (cur_score >= beta) //beta cutoff (fail soft)
            {
                return cur_score; //no effect of fail soft here
            }
        }
    }

    return alpha;
}

Value search(Board& board, int depth, Value alpha, Value beta, SearchStack* ss)
{
    if (panic || !(nodes & 0xFFF)) //check for panic every 4096 nodes
        if (panic || clock() > search_end_time ||
        (!(nodes & 0xFFFF) && kbhit())) //make costly key check extra rare
        {
            panic = true;
            return PANIC_VALUE; //PANIC; NOTE: negating INT32_MIN is UB!!!
        }

    if (depth == 0)
        return quiesce(board, alpha, beta);

    //check repetition BEFORE probing (no worry about priority for repetition!!!)
    if (board.isRepetition(1))
        return DRAW;

    //probe hash table
    //https://gitlab.com/mhouppin/stash-bot/-/blob/8ec0469cdcef022ee1bc304299f7c0e3e2674652/sources/engine/search_bestmove.c
    Move tt_move = Move::NO_MOVE; //tt miss => it will stay like this

    HASHE* phashe = ProbeHash(board, ss->ply);
    if (phashe != nullptr /* && board.halfMoveClock() <= 60 */) //we have a hit
    {
        //entry has enough depth
        if (phashe->depth >= depth && ss->ply > 1) {
            if (phashe->flags == hashfEXACT) //exact hit! great
                return phashe->val;
            else if ((phashe->flags == hashfALPHA) && //window resizing!
                (phashe->val < beta))
                beta = phashe->val;
            else if ((phashe->flags == hashfBETA) &&
                (phashe->val > alpha)) //same
                alpha = phashe->val;

            if (alpha >= beta)
                return alpha; //hit with a bound
        } //phashe->depth >= depth

        //this is executed even when we can't return from search immediately
        tt_move = Move(phashe->best); //write best move out of there
    }

    Movelist moves;
    movegen::legalmoves(moves, board);
    //if (tt_move.move() && moves.find(tt_move) == -1) std::cout << "RED ALERT\n";

    if (moves.size() == 0) //no legal moves
        return board.inCheck() ? (ss->ply + 128 - INT32_MAX) : DRAW; //return checkmate or stalemate
    if (board.isHalfMoveDraw()) //repetitions or 50-move rule
        return DRAW;

    //score moves
    score_moves(board, moves, tt_move, killers[depth]); //TODO: when implementing TT, put the move HERE!

    Move best_move = Move::NO_MOVE; //for hash table (if fail low, best move unknown)
    for (int i = 0; i < moves.size(); i++) {
        pick_move(moves, i); //get the best-scored move to the index i
        const auto move = moves[i];

        board.makeMove(move);
        nodes++; //1 move made = 1 node
        ss->ply++;
        Value cur_score = -search(board, depth - 1, -beta, -alpha, ss);
        ss->ply--;
        board.unmakeMove(move);

        //score checks probably useless! (still, avoid storing PANIC_VALUE in TT!)
        if (panic || cur_score == PANIC_VALUE || cur_score == -PANIC_VALUE) return PANIC_VALUE;

        if (cur_score > alpha)
        {
            alpha = cur_score;
            best_move = move; //update best move

            if (cur_score >= beta) //beta cutoff (fail soft)
            {
                //killer move update: quiet move; avoid duplicate killers
                //(TODO: test if better or worse)
                if (!board.isCapture(move) && move != killers[depth][0])
                {
                    killers[depth][1] = killers[depth][0];
                    killers[depth][0] = move;
                }

                //store in hash table (beta = lower bound flag)
                //why does fail soft give really bad results?
                RecordHash(board, depth, beta, hashfBETA, move, ss->ply);
                return cur_score; //fail soft here: no effect!
            }
        }
    }
    if (!panic && alpha != PANIC_VALUE && alpha != -PANIC_VALUE)
    {
        //Storing tt_move instead of best_move when failing low makes like 0 change
        uint8_t hashf = (best_move == Move::NO_MOVE) ? hashfALPHA : hashfEXACT;
        RecordHash(board, depth, alpha, hashf, best_move, ss->ply);
    }
    return alpha;
}

Move search_root(Board &board, int alloc_time_ms, int depth)
{
    //convert from ms to clock ticks; set this up for panic return
    clock_t start_time = clock();
    clock_t alloc_time_clk = alloc_time_ms * CLOCKS_PER_SEC / 1000;
    search_end_time = start_time + alloc_time_clk;

    if (depth != MAX_DEPTH) //"go depth ..." command
        search_end_time = (uint64_t)((clock_t)(-1)) >> 1; //maximum value of a clock_t

    nodes = 0; //reset node count
    panic = false; //reset panic flag

    Movelist moves;
    movegen::legalmoves(moves, board);
    Move best_move = moves[0]; //default move

    int8_t cur_depth = 0; //starting depth - 1 (may need to be increased)

    while (++cur_depth <= depth && !panic)
    {
        //iterate over all legal moves, try find the best one
        Value best_score = -INT32_MAX; //lower than EVERYTHING (even than value of -infinity)
        Move cur_best_move = best_move;
        SearchStack ss = { 1 }; //dont inc and dec it every time in search_root's loop
        for (int i = 0; i < moves.size(); i++) {
            const auto move = moves[i];

            nodes++;
            board.makeMove(move);

            Value cur_score;
            //check for draw in main loop as well (to avoid the bot just walking into a draw)
            if (board.isRepetition(1) || board.isHalfMoveDraw()) //repetitions or 50-move rule
                cur_score = 0;
            else cur_score = -search(board, cur_depth - 1, 1 - INT32_MAX, INT32_MAX - 1, &ss);

            board.unmakeMove(move);

            if (cur_score > best_score) //new best move
            {
                cur_best_move = move;
                best_score = cur_score;
            }
        }
        //TODO: partial search results
        if (!panic) best_move = cur_best_move;

        if (!panic) //fix bogus best_score (has to change with partial search results)
        {
            //print out all the juicy info
            uint32_t curtime = (clock() - start_time) * 1000 / CLOCKS_PER_SEC;
            std::cout << "info depth " << (int)cur_depth << " score cp " << best_score <<
                " nodes " << nodes << " time " << curtime <<
                " nps " << (curtime ? (nodes * 1000 / curtime) : 0) << " pv ";
            std::cout << uci::moveToUci(best_move) << " "; //print best move (not in TT)

            Board pv_board(board.getFen()); //copy board to avoid destroying it
            pv_board.makeMove(best_move); //make best move

            //extract PV from TT
            U64 hash = pv_board.hash();
            HASHE *phashe = ProbeHash(pv_board, 0);
            while (phashe != nullptr)
            {
                Move pv_move = phashe->best;
                std::cout << uci::moveToUci(pv_move) << " "; //print pv move
                pv_board.makeMove(pv_move); //push the move
                phashe = ProbeHash(pv_board, 0); //next tt entry
            }

            std::cout << std::endl; //print newline
        }
    }

    return best_move;
}
