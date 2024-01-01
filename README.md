
# basic-chess-engine
This is a chess engine I made that I want to be as simple and bug-free as possible. It can be used as a base for other engines, to experiment with things like pruning/extensions, evaluation, or parallel search.

This engine incorporates these basic features:
- PeSTO tapered evaluation
- Alpha-Beta pruning
- QSearch
- MVV-LVA & queen promotion move ordering
- Killer and History heuristics
- Transposition table: move ordering, resized window

The following features are disabled, but can be enabled by commenting out code:
- Principal Variation Search (PVS)
- Internal Iterative Reduction

## Building
This chess engine can be built on Linux using G++ and Clang++. I have not done any testing with other compilers, or on Windows.
If you want to build on Windows, you will have to replace the `posix.hpp` file by the line `import <conio.h>` to use the Windows `kbhit` function.
