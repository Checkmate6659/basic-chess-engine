#include "search.hpp"

uint64_t search(Board& board, int depth) {
    Movelist moves;
    movegen::legalmoves(moves, board);

    // if (depth == 1) { //>210Mnps with bulk counting
    //     return moves.size();
    // }
    if (depth == 0) { //with this, it drops to 8Mnps
        //eval(board); //drops to 4Mnps (TODO: optimize PSQT calculation)
        return 1;
    }

    uint64_t nodes = 0;

    for (int i = 0; i < moves.size(); i++) {
        const auto move = moves[i];
        board.makeMove(move);
        nodes += search(board, depth - 1);
        board.unmakeMove(move);
    }

    return nodes;
}

