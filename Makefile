all:
	g++ -std=c++17 -O3 -flto -DNDEBUG -march=native *.cpp
	#g++ -std=c++17 -O3 -flto -march=native *.cpp
