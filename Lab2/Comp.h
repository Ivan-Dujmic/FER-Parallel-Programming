#pragma once

#include <cstddef>

#include "Board.h"

class Comp {
private:
    std::size_t max_depth;
    std::size_t minimax_levels; // On how many levels/depths/heights the algoirthm uses minimax, the rest assumes that moves are random

    double move_recursive(Board &board, std::size_t depth = 1);

public:
    Comp(std::size_t max_depth, std::size_t minimax_levels = 0);

    bool move(Board &board);
};