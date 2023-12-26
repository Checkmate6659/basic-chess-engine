#include <cstdint>
#include <ctime>
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


Value quiesce(Board &board, Value alpha, Value beta)
{
    //stand pat
    Value static_eval = eval(board);
    if (static_eval > alpha)
    {
        alpha = static_eval;
        if (alpha >= beta) return alpha;
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
                return cur_score;
            }
        }
    }

    return alpha;
}

Value search(Board& board, int depth, Value alpha, Value beta)
{
    if (panic || !(nodes & 0xFFF)) //check for panic every 4096 nodes
        if (panic || clock() > search_end_time ||
        (!(nodes & 0xFFFF) && kbhit())) //make costly key check extra rare
        {
            panic = true;
            //using this special value to (hopefully) allow for better partial search results
            return PANIC_VALUE; //PANIC; this stays -infinity when taking opposite!
            //however we should be careful when adding/subtracting from scores
        }

    //probe hash table
    Move tt_move = Move::NO_MOVE; //tt miss => it will stay like this
    Value tt_val = ProbeHash(board, depth, alpha, beta, tt_move);
    if (tt_val != INT32_MIN)
       return tt_val;

    //final hash flag to store position at
    uint8_t hashf = hashfALPHA;

    if (depth == 0)
        return quiesce(board, alpha, beta);

    Movelist moves;
    movegen::legalmoves(moves, board);

    if (moves.size() == 0) //no legal moves
        return board.inCheck() ? -INT32_MAX : DRAW; //return checkmate or stalemate
    if (board.isRepetition(1) || board.isHalfMoveDraw())
        return DRAW;

    Move best_move = Move::NO_MOVE; //for hash table (if fail low, best move unknown)
    score_moves(board, moves, tt_move, killers[depth]); //TODO: when implementing TT, put the move HERE!
    for (int i = 0; i < moves.size(); i++) {
        pick_move(moves, i); //get the best-scored move to the index i
        const auto move = moves[i];

        board.makeMove(move);
        nodes++; //1 move made = 1 node
        Value cur_score = -search(board, depth - 1, -beta, -alpha);
        board.unmakeMove(move);

        if (cur_score > alpha)
        {
            alpha = cur_score;
            best_move = move;
            hashf = hashfEXACT; //we have an exact score (unless beta cutoff)

            if (cur_score >= beta) //beta cutoff (fail soft)
            {
                //killer move update: quiet move; avoid duplicate killers
                //(TODO: test if better or worse)
                if (!board.isCapture(move) && move != killers[depth][0])
                {
                    killers[depth][1] = killers[depth][0];
                    killers[depth][0] = move;
                }

                //store in hash table (beta = lower bound flag) TODO: fail soft
                RecordHash(board, depth, beta, hashfBETA, move);
                return cur_score;
            }
        }
    }

    RecordHash(board, depth, beta, hashf, best_move);
    return alpha;
}

Move search_root(Board &board, unsigned alloc_time_ms, int depth)
{
    //convert from ms to clock ticks; set this up for panic return
    clock_t start_time = clock();
    search_end_time = start_time + alloc_time_ms * CLOCKS_PER_SEC / 1000;

    nodes = 0; //reset node count
    panic = false; //reset panic flag

    Movelist moves;
    movegen::legalmoves(moves, board);
    Move best_move = moves[0]; //default move

    int8_t cur_depth = 0; //starting depth - 1 (may need to be increased)

    while (++cur_depth <= depth && !panic)
    {
        //iterate over all legal moves, try find the best one
        Value best_score = INT32_MIN; //lower than EVERYTHING (even than value of -infinity)
        Move cur_best_move = best_move;
        for (int i = 0; i < moves.size(); i++) {
            const auto move = moves[i];

            nodes++;
            board.makeMove(move);
            Value cur_score = -search(board, cur_depth - 1, -INT32_MAX, INT32_MAX);
            board.unmakeMove(move);

            if (cur_score > best_score) //new best move
            {
                cur_best_move = move;
                best_score = cur_score;
            }
        }
        //TODO: partial search results
        if (!panic) best_move = cur_best_move;

        //print out all the juicy info (NOTE: on last iteration best score is bogus)
        uint32_t curtime = (clock() - start_time) * 1000 / CLOCKS_PER_SEC;
        std::cout << "info depth " << (int)cur_depth << " score cp " << best_score <<
            " nodes " << nodes << " time " << curtime <<
            " nps " << (curtime ? (nodes * 1000 / curtime) : 0) << " pv ";
        std::cout << uci::moveToUci(best_move) << std::endl; //for now only print the best move as PV
    }

    return best_move;
}
