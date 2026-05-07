#include <cstddef>
#include <iostream>
// #include <mpi.h>
// #include <random>
// #include <unistd.h>

#include "Board.h"
#include "Comp.h"

int main() {
    std::size_t width, height, depth;
    std::cout << "Board width: ";
    std::cin >> width;
    std::cout << "Board height: ";
    std::cin >> height;
    std::cout << "Search depth: ";
    std::cin >> depth;

    Board board(width, height);
    Comp comp(depth);

    int col;
    while (true) {
        std::cout << board;
        std::cin >> col;
        if (board.place(col, true) == MoveResult::Win) std::cout << "player wins\n";
        std::cout << board;
        std::cin >> col;
        if (board.place(col, false) == MoveResult::Win) std::cout << "comp wins\n";
    }

    return 0;
}