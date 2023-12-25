all:
	g++ -std=c++17 -O3 -flto -DNDEBUG -march=native main.cpp
	#g++ -std=c++17 -O3 -flto -march=native main.cpp
