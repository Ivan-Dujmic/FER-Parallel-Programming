#pragma once

#include <cstddef>
#include <mpi.h>

#include "Board.h"

class Comp {
private:
    std::size_t max_depth;
    std::size_t solo_depth; // Depth at which the conductor sends out tasks to workers

    public:
    Comp(std::size_t max_depth, std::size_t solo_depth = -1);
    
    double move_recursive(Board &board, std::size_t depth = 1);
    bool move(Board &board);

    bool save_result_parallel(
        int task_id,
        double result,
        std::size_t width,
        std::vector<std::vector<double>> &buffer_results,
        std::vector<std::vector<std::size_t>> &buffer_counts,
        std::size_t start_depth = -1,
        bool is_valid = true
    );
    void move_recursive_parallel(
        Board &board,
        std::vector<std::vector<double>> &buffer_results,
        std::vector<std::vector<std::size_t>> &buffer_counts,
        std::size_t depth = 1
    );
    bool move_parallel(Board &board);
};