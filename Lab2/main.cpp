#include <cstddef>
#include <iostream>

#include "Board.h"
#include "Comp.h"

int main(int argc, char* argv[]) {
    std::size_t width, height, depth, minimax_levels;
    std::cout << "Board width: ";
    std::cin >> width;
    std::cout << "Board height: ";
    std::cin >> height;
    std::cout << "Search depth: ";
    std::cin >> depth;
    std::cout << "Minimax levels: ";
    std::cin >> minimax_levels;

    Board board(width, height);
    Comp comp(depth, minimax_levels);

    std::size_t col;
    MoveResult move_result;

    std::size_t move_count;

    while (true) {
        std::cout << board;
        
        move_count++;
        if (move_count == width * height) {
            std::cout << "Tied!\n";
        }

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
        
        move_count++;
        if (move_count == width * height) {
            std::cout << "Tied!\n";
        }
        
        if (comp.move(board)) {
            std::cout << board << "Computer wins!\n";
            return 0;
        }
    }
}