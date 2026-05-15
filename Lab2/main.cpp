#include <cstddef>
#include <iostream>
#include <mpi.h>

#include "Board.h"
#include "Comp.h"

struct Config {
    std::size_t width;
    std::size_t height;
    std::size_t depth;
    std::size_t minimax_levels;
};

int main(int argc, char* argv[]) {
    MPI_Init(NULL, NULL);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    Config config;

    if (rank == 0) {
        std::size_t solo_depth = -1;

        std::cout << "Board width: ";
        std::cin >> config.width;
        std::cout << "Board height: ";
        std::cin >> config.height;
        std::cout << "Search depth: ";
        std::cin >> config.depth;
        std::cout << "Minimax levels: ";
        std::cin >> config.minimax_levels;

        if (solo_depth != -1) {
            std::cout << "Solo depth: ";
            std::cin >> solo_depth;
        }

        for (int i = 0 ; i < world_size ; i++) {
            MPI_Send(&config, sizeof(config), MPI_BYTE, i, MPI_ANY_TAG, MPI_COMM_WORLD);
        }
        
        Board board(config.width, config.height);
        Comp comp(config.depth, config.minimax_levels);
        std::size_t col;
        MoveResult move_result;
    
        std::size_t move_count;
    
        while (true) {
            std::cout << board;
            
            move_count++;
            if (move_count == config.width * config.height) {
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
                break;
            }
            
            move_count++;
            if (move_count == config.width * config.height) {
                std::cout << "Tied!\n";
                break;
            }
            
            // TODO: do solo or parallel
            if (comp.move(board)) {
                std::cout << board << "Computer wins!\n";
                break;
            }
        }

        // TODO: FREE others
    } else {
        MPI_Recv(&config, sizeof(config), MPI_BYTE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        Board board(config.width, config.height);
        Comp comp(config.depth, config.minimax_levels);

        while (true) {
            // TODO:
            // Ask for work
            // If nore work
                // Break
            // Do work
            // Send result
        }
    }


    MPI_Finalize();

    return 0;
}