#include "Comp.h"

#include "Utility.h"

/* Idea for parallel work balancing:
   The most important thing to achieve is perfect load-balancing at the bottom-most depth.
   We can decide up to which depth we want to split the work/nodes (move_recursive) between workers (processes), let that be 'd_split'.
   Let the current depth be 'd'.
   Let the branching factor (board width) be 'B'.
   Let the number of workers be 'n'.
   Each node has an assigned worker (it's id/index/rank) and we'll call the worker of the node we are in 'w_curr'.
   The way to decide which workers need to go in a specific recursion branch is the following:
    - receptive_field = B ** (d_split - d)
    - n_send = ( receptive_field > n ) ? n : receptive_field
    - w_branch = w_curr

    for each branch:
    - w_end = (w_branch + n_send) % n
    - w_send[] = [w_curr : w_end].wrap_around
    -> accept w_send[] in that branch with w_branch doing the work of the immediate node (root of that branch)
    - w_next = (w_curr + receptive_field) % n
    - w_branch = w_next

   Example node workers for B=3, d_split=3, n=7:
d=0                      0
d=1        0             2             4
d=2    0   3   6     2   5   1     4   0   3
d=3   012 345 601   234 560 123   456 012 345
*/ 

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
        // No need to use the approximately_equal for doubles in the minimax part as we are always working with the exact three values
        
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