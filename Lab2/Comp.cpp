#include "Comp.h"

#include "Utility.h"

Comp::Comp(std::size_t max_depth, std::size_t minimax_levels) :
    max_depth(max_depth)
{
    if (minimax_levels > max_depth) this->minimax_levels = max_depth;
    else this->minimax_levels = minimax_levels;
}

double Comp::move_recursive(Board& board, std::size_t depth) {
    bool player_move = depth % 2 == 1;
    std::size_t width = board.get_width();

    MoveResult move_result;
    bool played = false;
    
    if (max_depth - depth < minimax_levels) { // Assume all optimal moves (minimax)
        double best_move = (player_move ? 1 : -1); // Initial best is worst
        for (std::size_t i = 0 ; i < width ; i++) {
            move_result = board.place(i, player_move);

            if (move_result == MoveResult::Invalid) continue;
            played = true;

            if (move_result == MoveResult::Win) {
                board.remove(i);
                return (player_move ? -1.0 : 1.0);
            }
            else if (depth != max_depth) {
                double score = move_recursive(board, depth + 1);
                if (score == 0.0) best_move = 0.0; // Better than losing
                else if (player_move) {
                    if (score == -1.0) {
                        board.remove(i);
                        return -1.0; // Optimal player would win
                    }
                } else if (score == 1.0) {
                    board.remove(i);
                    return 1.0; // Play your win
                }
            }
            else best_move = 0.0; // Neutral

            board.remove(i);
        }

        if (played) return best_move;
        else return 0.0; // Tied
    } else { // Assume all random moves (avg score)
        std::size_t valid_move_count = 0;
        double sum_scores = 0.0;

        for (std::size_t i = 0 ; i < width ; i++) {
            move_result = board.place(i, player_move);

            if (move_result == MoveResult::Invalid) continue;
            played = true;
            
            if (move_result == MoveResult::Win) sum_scores += (player_move ? -1.0 : 1.0);
            else if (depth != max_depth) sum_scores += move_recursive(board, depth + 1);
            // else sum_scores += 0;

            valid_move_count++;
            board.remove(i);
        }

        if (played) return sum_scores / valid_move_count;
        else return 0.0;
    }
}

bool Comp::move(Board &board) {
    std::size_t width = board.get_width();

    double best_score = -2;
    std::size_t best_move = -1; 

    for (std::size_t i = 0 ; i < width ; i++) {
        MoveResult move_result = board.place(i, false);
        if (move_result == MoveResult::Invalid) continue;
        else if (move_result == MoveResult::Win) return true; // Instant win
        
        if (max_depth > 0) {
            double score = move_recursive(board);
            if (approximately_equal(score, 1.0)) return false; // Guaranteed win

            if (score > best_score) {
                best_score = score;
                best_move = i;
            }
        } else best_move = i; // At least make a valid move

        board.remove(i);
    }

    return board.place(best_move, false) == MoveResult::Win;
}