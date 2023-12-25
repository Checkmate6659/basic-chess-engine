#include <ctime>
#include <time.h>

#include "chess.hpp"
#include "search.hpp"
#include "eval.hpp"
using namespace chess;

int main () {
    init_tables();

    //Board board = Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    Board board = Board("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ");

    std::cout << eval(board) << std::endl;

    Movelist moves;
    movegen::legalmoves(moves, board);

    for (const auto &move : moves) {
        std::cout << uci::moveToUci(move) << std::endl;
    }

    clock_t start = clock();
    Value score = search(board, 4, -INT32_MAX, INT32_MAX);
    clock_t end = clock();
    std::cout << score << std::endl;
    std::cout << nodes << std::endl;
    std::cout << nodes * CLOCKS_PER_SEC / (end - start) << std::endl;

    return 0;
}