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

    std::size_t col;
    MoveResult move_result;

    while (true) {
        std::cout << board;
        do {
            std::cout << "Your move: ";
            std::cin >> col;
            move_result = board.place(col, true);
        } while (move_result == MoveResult::Invalid);

        
        std::cout << board;

        if (move_result == MoveResult::Win) {
            std::cout << "Player wins!\n";
            return 0;
        }
        
        if (comp.move(board)) {
            std::cout << board << "Computer wins!\n";
            return 0;
        }
    }
}