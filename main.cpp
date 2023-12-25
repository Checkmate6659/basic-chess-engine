#include <ctime>
#include <time.h>

#include "chess.hpp"
#include "eval.hpp"

using namespace chess;

uint64_t perft(Board& board, int depth) {
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
        nodes += perft(board, depth - 1);
        board.unmakeMove(move);
    }

    return nodes;
}

int main () {
    Board board = Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    init_tables();
    std::cout << eval(board) << std::endl;

    Movelist moves;
    movegen::legalmoves(moves, board);

    for (const auto &move : moves) {
        std::cout << uci::moveToUci(move) << std::endl;
    }

    clock_t start = clock();
    uint64_t nodes = perft(board, 5);
    clock_t end = clock();
    std::cout << nodes << std::endl;
    std::cout << nodes * CLOCKS_PER_SEC / (end - start) << std::endl;

    return 0;
}