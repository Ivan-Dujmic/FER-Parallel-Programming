#include <cstddef>
#include <iostream>
#include <mpi.h>

#include "Utility.h"
#include "Board.h"
#include "Comp.h"

struct Config {
    std::size_t width;
    std::size_t height;
    std::size_t depth;
    std::size_t solo_depth;
};

int main(int argc, char* argv[]) {
    MPI_Init(NULL, NULL);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    MPI_Status status;

    Config config;

    std::vector<char> spots;
    std::vector<std::size_t> heights;

    if (rank == 0) {
        std::cout << "Board width: ";
        std::cin >> config.width;
        std::cout << "Board height: ";
        std::cin >> config.height;
        std::cout << "Search depth: ";
        std::cin >> config.depth;
        if (world_size > 1) {
            std::cout << "Solo depth: ";
            std::cin >> config.solo_depth;
        } else {
            config.solo_depth = -1;
        }

        for (int i = 0 ; i < world_size ; i++) {
            MPI_Send(&config, sizeof(config), MPI_BYTE, i, 0, MPI_COMM_WORLD);
        }
        
        Board board(config.width, config.height);
        Comp comp(config.depth, config.solo_depth);
        
        std::size_t col;
        MoveResult move_result;
    
        std::size_t move_count;
    
        while (true) {
            std::cout << board;
            
            move_count++;
            if (move_count == config.width * config.height) {
                std::cout << "Tied!\n";
                break;
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
            
            if (world_size > 1 ? comp.move_parallel(board) : comp.move(board)) {
                std::cout << board << "Computer wins!\n";
                break;
            }
        }

        // Free the workers
        for (int i = 1 ; i < world_size ; i++) {
            MPI_Send(nullptr, 0, MPI_BYTE, i, TAG_SPECIAL, MPI_COMM_WORLD);
        }
    } else {
        MPI_Recv(&config, sizeof(config), MPI_BYTE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // Get config

        Board board(config.width, config.height);
        Comp comp(config.depth);
        spots.reserve(config.width * config.height);
        heights.reserve(config.width);

        while (true) {
            MPI_Send(nullptr, 0, MPI_BYTE, 0, TAG_SPECIAL, MPI_COMM_WORLD); // Ask for the first task
            while (true) {
                MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status); // Wait for a task
                if (status.MPI_TAG == TAG_SPECIAL) { // No more tasks
                    MPI_Recv(nullptr, 0, MPI_BYTE, 0, TAG_SPECIAL, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    break;
                }

                // Obtain task resources:
                MPI_Recv(spots.data(), config.width * config.height, MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                int task_id = status.MPI_TAG;
                MPI_Recv(heights.data(), config.width, MPI_BYTE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                board.set_board(std::move(spots), std::move(heights));

                double result = comp.move_recursive(board, config.solo_depth + 1); // Perform task
                MPI_Send(&result, 1, MPI_DOUBLE, 0, task_id, MPI_COMM_WORLD); // Send result
            }
        }
    }

    MPI_Finalize();

    return 0;
}