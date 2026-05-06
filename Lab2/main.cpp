#include <cstddef>
#include <iostream>
// #include <mpi.h>
// #include <random>
// #include <unistd.h>

#define WIDTH 7 // Playfield width
#define HEIGHT 6 // Playfield height
#define DEPTH 7 // Tree state space search depth
#define SYMBOL_COM 'O' // Computer's playing symbol
#define SYMBOL_PLAYER 'X' // Player's playing symbol
#define SYMBOL_EMPTY '.' // Empty slot symbol

enum class MoveResult {
    Invalid,
    Win,
    Continue
};

class Board {
friend std::ostream& operator<<(std::ostream&, const Board&);

private:
    char spots[WIDTH][HEIGHT];
    std::size_t heights[WIDTH];
    
public:
    Board() {
        for (std::size_t i = 0 ; i < WIDTH ; i++) {
            heights[i] = 0;

            for (std::size_t j = 0 ; j < HEIGHT ; j++) {
                spots[i][j] = SYMBOL_EMPTY;
            }
        }
    }

    MoveResult place(std::size_t col, bool player_move) {
        if (heights[col] >= HEIGHT) return MoveResult::Invalid;

        char symbol = (player_move ? SYMBOL_PLAYER : SYMBOL_COM);

        std::size_t row = heights[col];
        spots[col][row] = symbol;
        heights[col]++;

        std::size_t count;

        // Horizontal
        count = 0;
        for (std::size_t i = col ; i > 0 ; i--) {
            if (spots[i][row] != symbol) break;
            count++;
            if (count == 3) return MoveResult::Win;
        }
        for (std::size_t i = col ; i < WIDTH - 1 ; i++) {
            if (spots[i][row] != symbol) break;
            count++;
            if (count == 3) return MoveResult::Win;
        }

        // Vertical
        count = 0;
        for (std::size_t i = row ; i > 0 ; i--) {
            if (spots[col][i] != symbol) break;
            count++;
            if (count == 3) return MoveResult::Win;
        }
        for (std::size_t i = row ; i < HEIGHT - 1 ; i++) {
            if (spots[col][i] != symbol) break;
            count++;
            if (count == 3) return MoveResult::Win;
        }

        // Ascending diagonal
        count = 0;
        for (std::size_t i = col, j = row ; i > 0 and j > 0 ; i--, j--) {
            if (spots[i][j] != symbol) break;
            count++;
            if (count == 3) return MoveResult::Win;
        }
        for (std::size_t i = col, j = row ; i < WIDTH - 1 and j < HEIGHT; i++, j++) {
            if (spots[i][j] != symbol) break;
            count++;
            if (count == 3) return MoveResult::Win;
        }

        // Descending diagonal
        count = 0;
        for (std::size_t i = col, j = row ; i > 0 and j < HEIGHT ; i--, j++) {
            if (spots[i][j] != symbol) break;
            count++;
            if (count == 3) return MoveResult::Win;
        }
        for (std::size_t i = col, j = row ; i < WIDTH - 1 and j > 0; i++, j--) {
            if (spots[i][j] != symbol) break;
            count++;
            if (count == 3) return MoveResult::Win;
        }

        return MoveResult::Continue;
    }
};

std::ostream& operator<<(std::ostream& os, const Board& b) {
    for (std::size_t i = HEIGHT ; i > 0 ; i--) {
        for (std::size_t j = 0 ; j < WIDTH ; j++) {
            os << ' ' << b.spots[j][i - 1];
        }
        os << '\n';
    }
    for (std::size_t i = 0 ; i < WIDTH ; i++) {
        os << ' ' << i;
    }
    os << '\n';

    return os;
}

double move_recursive(char (&playfield)[WIDTH][HEIGHT], std::size_t depth = 1) {
    
}

std::size_t move(char playfield[WIDTH][HEIGHT]) {

}

int main() {
    Board board;

    int col;
    while (true) {
        std::cout << board;
        std::cin >> col;
        board.place(col, true);
        std::cout << board;
        std::cin >> col;
        board.place(col, true);
    }

    return 0;
}