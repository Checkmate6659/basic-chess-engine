#include "search.hpp"
#include "eval.hpp"


uint64_t nodes = 0;

Value search(Board& board, int depth, Value alpha, Value beta) {
    if (depth == 0)
    {
        nodes++;
        return eval(board);
    }

    Movelist moves;
    movegen::legalmoves(moves, board);

    if (moves.size() == 0) //no legal moves
        return -INT32_MAX * board.inCheck(); //return checkmate or stalemate accordingly

    for (int i = 0; i < moves.size(); i++) {
        const auto move = moves[i];

        board.makeMove(move);
        Value cur_score = -search(board, depth - 1, -beta, -alpha);
        board.unmakeMove(move);

        if (cur_score > alpha)
        {
            alpha = cur_score;

            if (cur_score > beta) //beta cutoff (fail soft)
                return cur_score;
        }
    }

    return alpha;
}

