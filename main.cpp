#include <ctime>
#include <functional>
#include <iostream>
#include <time.h>

#include "chess.hpp"
#include "search.hpp"
#include "eval.hpp"
using namespace chess;

#define ENGINE_NAME "BasicChessEngine v1.0"

int main () {
    init_tables();

    //Board board = Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    Board board = Board("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ");
    std::cout << board.isRepetition(1); //count==1 => 2fold repetition
    board.makeMove(uci::uciToMove(board, "f3e3"));
    board.makeMove(uci::uciToMove(board, "e7f8"));
    board.makeMove(uci::uciToMove(board, "e3f3"));
    board.makeMove(uci::uciToMove(board, "f8e7"));
    std::cout << board.isRepetition(1) << std::endl;

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

    //UCI loop
    while(true)
    {
		std::string input_string;
		std::getline(std::cin, input_string);
		std::stringstream input_stream(input_string);
		std::string command;
		input_stream >> command;

        switch (command[0])
        {
            case 'u': //uci (ucinewgame is here too; don't care)
            std::cout << "id name " ENGINE_NAME "\nid author Enigma\n";
            //TODO: if there are options, add them HERE!
            std::cout << "uciok\n";
            break;
            case 'i': //isready
            std::cout << "readyok\n";
            break;
            case 'q': //quit
            return 0;
            case 'p': //position
            input_stream >> command; //get new word
            if (command[0] == 's') //position startpos ...
                board.setFen(constants::STARTPOS); //handy
            else //position fen <FEN> ... (ONLY ACCEPTS FEN WITH COUNTERS! otherwise it freaks out)
            {
                std::string fen_string;
                for (int i = 0; i < 6; i++) //fen string has 6 "words"
                {
                    input_stream >> command;
                    fen_string.append(command);
                    if(i != 5) fen_string.append(" "); //spaces between words
                }
                board.setFen(fen_string);
            }
            //load moves after fen string (or startpos)
            input_stream >> command;
            if (command[0] != 'm') break; //no moves to load
            input_stream >> command;
            while (input_stream)
            {
                board.makeMove(uci::uciToMove(board, command)); //make move
                input_stream >> command; //load next move in
            }
            //std::cout << board.getFen() << std::endl; //DEBUG

            // break;
            // case 'g': //go command
        }
    }

    return 0;
}
