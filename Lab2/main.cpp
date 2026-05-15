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
    std::size_t solo_depth;
};

enum Tags {
    ASK, // Ask for work
    TASK, // Task resources
    RESULT, // Give result
    FINISH // No more work
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
        std::cout << "Minimax levels: ";
        std::cin >> config.minimax_levels;
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
        Comp comp(config.depth, config.minimax_levels);
        spots.reserve(config.width * config.height);
        heights.reserve(config.width);
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
            
            // TODO: Do solo or parallel
            if (comp.move(board)) {
                std::cout << board << "Computer wins!\n";
                break;
            }
        }

        for (int i = 0 ; i < world_size ; i++) {
            MPI_Send(nullptr, 0, MPI_BYTE, i, Tags::FINISH, MPI_COMM_WORLD);
        }
    } else {
        MPI_Recv(&config, sizeof(config), MPI_BYTE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        Board board(config.width, config.height);
        Comp comp(config.depth, config.minimax_levels);
        spots.reserve(config.width * config.height);
        heights.reserve(config.width);
        while (true) {
            MPI_Send(nullptr, 0, MPI_BYTE, 0, Tags::ASK, MPI_COMM_WORLD);
            MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if (status.MPI_TAG == Tags::FINISH) {
                MPI_Recv(nullptr, 0, MPI_BYTE, 0, Tags::FINISH, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                break;
            }
            MPI_Recv(spots.data(), config.width * config.height, MPI_CHAR, 0, Tags::TASK, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(heights.data(), config.width, MPI_BYTE, 0, Tags::TASK, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            board.set_board(std::move(spots), std::move(heights));
            double result = comp.move_recursive(board, config.solo_depth + 1);
            MPI_Send(&result, 1, MPI_DOUBLE, 0, Tags::RESULT, MPI_COMM_WORLD);
        }
    }


    MPI_Finalize();

    return 0;
}