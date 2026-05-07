#include "Comp.h"

#include "Utility.h"

Comp::Comp(std::size_t max_depth) :
    max_depth(max_depth)
{}

double Comp::move_recursive(Board& board, std::size_t depth) {
    bool comp_move = depth % 2 == 0;

    return 0;
}

bool Comp::move(Board &board) {
    std::size_t width = board.get_width();

    double best_score = -2;
    std::size_t best_move = 0; 

    for (std::size_t i = 0 ; i < width ; i++) {
        if (board.place(i, false) == MoveResult::Win) return true;

        if (max_depth > 0) {
            double score = move_recursive(board);
            if (approximately_equal(score, 1.0)) return true;

            if (score > best_score) {
                best_score = score;
                best_move = i;
            }
        }

        board.remove(i);
    }

    return board.place(best_move, false) == MoveResult::Win;
}