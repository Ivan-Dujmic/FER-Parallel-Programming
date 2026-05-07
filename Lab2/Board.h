#pragma once

#include <iostream>
#include <cstddef>
#include <vector>

enum class MoveResult {
    Invalid,
    Win,
    Continue
};

class Board {
private:
    std::size_t width;
    std::size_t height;
    std::vector<std::vector<char>> spots;
    std::vector<std::size_t> heights; // Column heights
    char symbol_empty;
    char symbol_player;
    char symbol_comp;
    
public:
    Board(std::size_t width, std::size_t height, char symbol_empty = '.', char symbol_player = 'X', char symbol_comp = 'O');
    
    friend std::ostream& operator<<(std::ostream&, const Board&);

    MoveResult place(std::size_t col, bool player_move);
    bool remove(std::size_t col);
};