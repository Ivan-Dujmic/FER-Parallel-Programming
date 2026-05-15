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
    bool move_parallel(Board &board);
};