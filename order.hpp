#include "chess.hpp"
#include <array>
#include <cstdint>
using namespace chess;

//Give a score to all the moves (don't order them immediately!)
inline void score_moves(Board &board, Movelist &moves, uint16_t tt_move)
{
    //WARNING: move scores in chess-library are int16_t, so careful with 32-bit hist
    //Also it goes from -32768 to 32767; there are negative values!
    for (int i = 0; i < moves.size(); i++) {
        const auto move = moves[i];

        if (board.isCapture(moves[i]))
        {
            PieceType victim = board.at<PieceType>(moves[i].to());
            PieceType aggressor = board.at<PieceType>(moves[i].from());
            moves[i].setScore(0x4000 + (int)victim * 16 - (int)aggressor);
        }
        else
        {
            moves[i].setScore(-32000);
        }
    }
}

//swap moves[i] with the best scored move after i
inline void pick_move(Movelist &moves, int i)
{
    Move moves_i = moves[i];
    int16_t best_score = moves_i.score();
    int best_index = i;

    //search for highest-scored move after this one
    for (int j = i + 1; j < moves.size(); j++)
    {
        const auto move = moves[j];
        if (move.score() > best_score)
        {
            best_score = move.score();
            best_index = j;
        }
    }

    //swap it in!
    moves[i] = moves[best_index];
    moves[best_index] = moves_i;
}
