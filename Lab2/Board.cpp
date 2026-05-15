#include "Board.h"

Board::Board(std::size_t width, std::size_t height, char symbol_empty, char symbol_player, char symbol_comp) : 
    width(width),
    height(height),
    spots(width * height, symbol_empty),
    heights(width, 0),
    symbol_empty(symbol_empty),
    symbol_player(symbol_player),
    symbol_comp(symbol_comp)
{}

std::ostream& operator<<(std::ostream& os, const Board& b) {
    for (std::size_t i = b.height ; i > 0 ; i--) {
        for (std::size_t j = 0 ; j < b.width ; j++) {
            os << ' ' << b.spots[j * b.height + i - 1];
        }
        os << '\n';
    }
    for (std::size_t i = 0 ; i < b.width ; i++) {
        os << ' ' << i;
    }
    os << '\n';

    return os;
}

std::size_t Board::get_width() const {
    return width;
}

std::size_t Board::get_height() const {
    return height;
}

void Board::set_board(const std::vector<char> &&spots, const std::vector<std::size_t> &&heights) {
    this->spots = std::move(spots);
    this->heights = std::move(heights);
}

MoveResult Board::place(std::size_t col, bool player_move) {
    if (heights[col] >= height || col >= width) return MoveResult::Invalid;
    
    char symbol = (player_move ? symbol_player : symbol_comp);
    
    // Place
    std::size_t row = heights[col];
    spots[col * width + row] = symbol;
    heights[col]++;

    std::size_t count;

    // Win checking...

    // Horizontal
    count = 0;
    for (std::size_t i = col ; i > 0 ; i--) {
        if (spots[(i - 1) + width + row] != symbol) break;
        count++;
        if (count == 3) return MoveResult::Win;
    }
    for (std::size_t i = col + 1 ; i < width ; i++) {
        if (spots[i * width + row] != symbol) break;
        count++;
        if (count == 3) return MoveResult::Win;
    }

    // Vertical
    count = 0;
    for (std::size_t i = row ; i > 0 ; i--) {
        if (spots[col * width + i - 1] != symbol) break;
        count++;
        if (count == 3) return MoveResult::Win;
    }
    for (std::size_t i = row + 1 ; i < height ; i++) {
        if (spots[col * width + i] != symbol) break;
        count++;
        if (count == 3) return MoveResult::Win;
    }

    // Ascending diagonal
    count = 0;
    for (std::size_t i = col, j = row ; i > 0 && j > 0 ; i--, j--) {
        if (spots[(i - 1) * width + j - 1] != symbol) break;
        count++;
        if (count == 3) return MoveResult::Win;
    }
    for (std::size_t i = col + 1, j = row + 1 ; i < width && j < height; i++, j++) {
        if (spots[i * width + j] != symbol) break;
        count++;
        if (count == 3) return MoveResult::Win;
    }

    // Descending diagonal
    count = 0;
    for (std::size_t i = col, j = row + 1 ; i > 0 && j < height ; i--, j++) {
        if (spots[(i - 1) * width + j] != symbol) break;
        count++;
        if (count == 3) return MoveResult::Win;
    }
    for (std::size_t i = col + 1, j = row ; i < width && j > 0; i++, j--) {
        if (spots[i * width + j - 1] != symbol) break;
        count++;
        if (count == 3) return MoveResult::Win;
    }

    return MoveResult::Continue;
}

bool Board::remove(std::size_t col) {
    std::size_t row = heights[col] - 1;
    if (col >= width || row >= height) return false;
    spots[col * width + row] = symbol_empty;
    heights[col]--;
    return true;
}