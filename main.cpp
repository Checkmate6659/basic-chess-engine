#include <cstdint>
#include <ctime>
#include <functional>
#include <iostream>
#include <time.h>

#include "chess.hpp"
#include "search.hpp"
#include "eval.hpp"
using namespace chess;

#define ENGINE_NAME "BasicChessEngine v1.0"

#define EXTRA_DELAY 10 //time to account for communication and panic delay (in ms)

int main()
{
    init_tables();
    Board board = Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    //hash_unit_test(board);

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
            break;
            case 'g': //go command
            char engine_color = (board.sideToMove() == Color::WHITE) ? 'w' : 'b';
            unsigned engtime = 0; //engine's think time
            unsigned enginc = 0; //engine's increment
            unsigned movestogo = 30; //default to 30
            unsigned depth = MAX_DEPTH; //default depth = max
            unsigned movetime = 0; //no set movetime = 0 (special value)
            //load parameters
            input_stream >> command;
            while (input_stream)
            {
                if (command[0] == 'd') //depth
                {
                    input_stream >> command;
                    depth = std::stoi(command);
                }
                else if (command[0] == engine_color) //wtime, btime, winc, binc
                {
                    if (command[1] == 't') //wtime, btime
                    {
                        input_stream >> command;
                        engtime = std::stoi(command);
                    }
                    else //winc, binc
                    {
                        input_stream >> command;
                        enginc = std::stoi(command);
                    }
                }
                else if (command[4] == 't') //movetime
                {
                    input_stream >> command;
                    movetime = std::stoi(command);
                }
                else //movestogo
                {
                    input_stream >> command;
                    movestogo = std::stoi(command);
                }
                input_stream >> command;
            }
            int alloc_time = movetime ? //time management (all in milliseconds)
                movetime :
                (engtime / movestogo + enginc)
            - EXTRA_DELAY; //account for communication delays
            //pass it on to this function in search.cpp
            Move best_move = search_root(board, alloc_time, depth);
            //print best move
            std::cout << "bestmove " << uci::moveToUci(best_move) << std::endl;
        }
    }

    return 0;
}
