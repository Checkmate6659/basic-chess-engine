#ifndef ORDER_H
#define ORDER_H
#include "chess.hpp"
#include <algorithm>
#include <cstdint>
using namespace chess;

//history table (piece; to)
int16_t hist[12][64];

inline void boost_hist(Piece piece, Square to, int8_t depth)
{
    int boost = depth * depth; //simple depth² (experiment with this!); no overflows!
    int new_hist = hist[(int)piece][to.index()] + boost; //score not constrained yet!
    //constrain history score
    hist[(int)piece][to.index()] = std::min(0x7800, new_hist); //score can only go up here
    //IF INTRODUCING PENALTIES IN THIS FUNCTION, USE NEXT LINE!
    //hist[(int)piece][to.index()] = std::min(0x7800, std::max(-0x8000, new_hist));
}

inline void penal_hist(Piece piece, Square to, int8_t depth)
{
    int penalty = depth * depth; //simple depth² (experiment with this!); no overflows!
    int new_hist = hist[(int)piece][to.index()] - penalty; //score not constrained yet!
    //constrain history score
    hist[(int)piece][to.index()] = std::max(-0x8000, new_hist); //score can only go down here
    //IF INTRODUCING BOOSTS IN THIS FUNCTION, USE NEXT LINE!
    //hist[(int)piece][to.index()] = std::min(0x7800, std::max(-0x8000, new_hist));
}

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
            moves[i].setScore(0x7802);
        }
        else if (move == cur_killers[1])
        {
            moves[i].setScore(0x7801);
        }
        else
        {
            //moves[i].setScore(-32000);
            moves[i].setScore(hist //piece-to hist score (we have to cap it tho)
                [(int)board.at<Piece>(moves[i].from())]
                [moves[i].to().index()]);
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
