#pragma once

#include <cstddef>

#include "Board.h"

class Comp {
private:
    std::size_t max_depth;

    double move_recursive(Board &board, std::size_t depth = 1);

public:
    Comp(std::size_t max_depth);

    bool move(Board &board);
};