#include "Board.h"

std::ostream& operator<<(std::ostream& os, const Board& b) {
    for (std::size_t i = b.height ; i > 0 ; i--) {
        for (std::size_t j = 0 ; j < b.width ; j++) {
            os << ' ' << b.spots[j][i - 1];
        }
        os << '\n';
    }
    for (std::size_t i = 0 ; i < b.width ; i++) {
        os << ' ' << i;
    }
    os << '\n';

    return os;
}

Board::Board(std::size_t width, std::size_t height, char symbol_empty, char symbol_player, char symbol_comp) : 
    width(width),
    height(height),
    spots(width, std::vector<char>(height, symbol_empty)),
    symbol_empty(symbol_empty),
    symbol_player(symbol_player),
    symbol_comp(symbol_comp)
{}

MoveResult Board::place(std::size_t col, bool player_move) {
    if (heights[col] >= height || col >= width) return MoveResult::Invalid;

    char symbol = (player_move ? symbol_player : symbol_comp);

    // Place
    std::size_t row = heights[col];
    spots[col][row] = symbol;
    heights[col]++;

    std::size_t count;

    // Win checking...

    // Horizontal
    count = 0;
    for (std::size_t i = col ; i > 0 ; i--) {
        if (spots[i - 1][row] != symbol) break;
        count++;
        if (count == 3) return MoveResult::Win;
    }
    for (std::size_t i = col + 1 ; i < width - 1 ; i++) {
        if (spots[i][row] != symbol) break;
        count++;
        if (count == 3) return MoveResult::Win;
    }

    // Vertical
    count = 0;
    for (std::size_t i = row ; i > 0 ; i--) {
        if (spots[col][i - 1] != symbol) break;
        count++;
        if (count == 3) return MoveResult::Win;
    }
    for (std::size_t i = row + 1 ; i < height - 1 ; i++) {
        if (spots[col][i] != symbol) break;
        count++;
        if (count == 3) return MoveResult::Win;
    }

    // Ascending diagonal
    count = 0;
    for (std::size_t i = col, j = row ; i > 0 and j > 0 ; i--, j--) {
        if (spots[i - 1][j - 1] != symbol) break;
        count++;
        if (count == 3) return MoveResult::Win;
    }
    for (std::size_t i = col + 1, j = row + 1 ; i < width - 1 and j < height; i++, j++) {
        if (spots[i][j] != symbol) break;
        count++;
        if (count == 3) return MoveResult::Win;
    }

    // Descending diagonal
    count = 0;
    for (std::size_t i = col, j = row + 1 ; i > 0 and j < height ; i--, j++) {
        if (spots[i - 1][j] != symbol) break;
        count++;
        if (count == 3) return MoveResult::Win;
    }
    for (std::size_t i = col + 1, j = row ; i < width - 1 and j > 0; i++, j--) {
        if (spots[i][j - 1] != symbol) break;
        count++;
        if (count == 3) return MoveResult::Win;
    }

    return MoveResult::Continue;
}

bool Board::remove(std::size_t col) {
    std::size_t row = heights[col] - 1;
    if (col >= width || row >= height) return false;
    spots[col][row] = symbol_empty;
    heights[col]--;
    return true;
}