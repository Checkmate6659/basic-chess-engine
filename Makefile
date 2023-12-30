#NOTE: as of chess-library v0.6.12, chess-library compiles with clang, but segfaults
all:
	#g++ -std=c++17 -O3 -Ofast -flto -DNDEBUG -march=native *.cpp
	#g++ -std=c++17 -O3 -Ofast -flto -s -DNDEBUG -march=native *.cpp
	clang++ -std=c++17 -O3 -Ofast -flto -s -DNDEBUG -march=native *.cpp
	#g++ -std=c++17 -O3 -flto -march=native *.cpp

#g++ -std=c++17 -O3 -Ofast -flto -DNDEBUG -march=native -c chess.hpp -o chess-library.o
