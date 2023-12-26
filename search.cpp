#include <cstdint>
#include <ctime>
#include "chess.hpp"
#include "search.hpp"
#include "eval.hpp"
#include "order.hpp" //move scoring
#include "posix.hpp" //kbhit equivalent on linux

uint64_t nodes = 0;
clock_t search_end_time;
bool panic = false;

Move killers[MAX_DEPTH][2];

Value search(Board& board, int depth, Value alpha, Value beta)
{
    if (panic || !(nodes & 0x3FF)) //check for panic
        if (panic || clock() > search_end_time || kbhit())
        {
            panic = true;
            //using this special value to (hopefully) allow for better partial search results
            return PANIC_VALUE; //PANIC; this stays -infinity when taking opposite!
            //however we should be careful when adding/subtracting from scores
        }

    if (depth == 0)
        return eval(board);

    Movelist moves;
    movegen::legalmoves(moves, board);

    if (moves.size() == 0) //no legal moves
        return board.inCheck() ? -INT32_MAX : DRAW; //return checkmate or stalemate
    if (board.isRepetition(1) || board.isHalfMoveDraw())
        return DRAW;

    score_moves(board, moves, 0, killers[depth]); //TODO: when implementing TT, put the move HERE!
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

            if (cur_score > beta) //beta cutoff (fail soft)
            {
                //killer move update: quiet move; avoid duplicate killers
                //(TODO: test if better or worse)
                if (!board.isCapture(move) && move != killers[depth][0])
                {
                    killers[depth][1] = killers[depth][0];
                    killers[depth][0] = move;
                }
                return cur_score;
            }
        }
    }

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
