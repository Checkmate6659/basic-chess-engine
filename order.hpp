#ifndef ORDER_H
#define ORDER_H
#include "chess.hpp"
#include <array>
#include <cstdint>
using namespace chess;

//Give a score to all the moves (don't order them immediately!)
inline void score_moves(Board &board, Movelist &moves, Move &tt_move, Move* cur_killers)
{
    //WARNING: move scores in chess-library are int16_t, so careful with 32-bit hist
    //Also it goes from -32768 to 32767; there are negative values!
    for (int i = 0; i < moves.size(); i++) {
        const auto move = moves[i];

        if (moves[i] == tt_move) //TT MOVE!!!
            moves[i].setScore(0x7FFF); //highest score
        //score promotions! (no library function for that tho); only up queen promos a lot
        else if (
            moves[i].promotionType() == PieceType::QUEEN //promotes to a queen (might be enough by itself?)
            && board.at<PieceType>(moves[i].from()) == PieceType::PAWN //moved a pawn
            && moves[i].to().rank() % 7 == 0) //to a back rank
        {
            //like MVV-LVA really
            PieceType victim = board.at<PieceType>(moves[i].to());
            moves[i].setScore(0x7FF8 + (int)victim); //really high score!
        }
        else if (board.isCapture(moves[i]))
        {
            //MVV-LVA
            PieceType victim = board.at<PieceType>(moves[i].to());
            PieceType aggressor = board.at<PieceType>(moves[i].from());
            moves[i].setScore(0x7810 + (int)victim * 16 - (int)aggressor);
        }
        else if (move == cur_killers[0])
        {
            moves[i].setScore(0x7801);
        }
        else if (move == cur_killers[1])
        {
            moves[i].setScore(0x7800);
        }
        else
        {
            moves[i].setScore(-32000);
        }
    }
}

//Same for qsearch
inline void score_moves_quiesce(Board &board, Movelist &moves)
{
    for (int i = 0; i < moves.size(); i++) {
        const auto move = moves[i];

        if (board.isCapture(moves[i]))
        {
            //MVV-LVA
            PieceType victim = board.at<PieceType>(moves[i].to());
            PieceType aggressor = board.at<PieceType>(moves[i].from());
            moves[i].setScore(0x4010 + (int)victim * 16 - (int)aggressor);
        }
        else //could get rid of this; keeping it in for safety
        {
            moves[i].setScore(-32000);
        }
    }
}

//swap moves[i] with the best scored move after i
//TODO: look at Movelist.sort(int index = 0)
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
#endif
