#include "Comp.h"

#include "Utility.h"

Comp::Comp(std::size_t max_depth, std::size_t solo_depth) :
        max_depth(max_depth),
        solo_depth(solo_depth) {}

double Comp::move_recursive(Board& board, std::size_t depth) {
    bool player_move = depth % 2 == 1;
    std::size_t width = board.get_width();

    MoveResult move_result;
    
    std::size_t valid_move_count = 0;
    double sum_scores = 0.0;

    for (std::size_t i = 0 ; i < width ; i++) {
        move_result = board.place(i, player_move);

        if (move_result == MoveResult::Invalid) continue;
        
        if (move_result == MoveResult::Win) {
            board.remove(i);
            return player_move ? -1.0 : 1.0;
        }
        else if (depth != max_depth) sum_scores += move_recursive(board, depth + 1);
        // else sum_scores += 0;

        valid_move_count++;
        board.remove(i);
    }

    return valid_move_count > 0 ? sum_scores / valid_move_count : 0.0;
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

bool Comp::save_result_parallel(
    int task_id,
    double result,
    std::size_t width,
    std::vector<std::vector<double>> &buffer_results,
    std::vector<std::vector<std::size_t>> &buffer_counts,
    std::size_t start_depth,
    bool is_valid
) {
    std::size_t depth = (start_depth == -1 ? solo_depth + 1 : start_depth);
    buffer_results[depth][task_id] = result;

    depth--;
    int prev_task_id = task_id;
    task_id /= width;

    if (is_valid) buffer_results[depth][task_id] += 1.0; // We have one more subresult for that node
    buffer_counts[depth][task_id] += 1; // We've cleared one more child for that node

    if (buffer_counts[0][0] == width) return true;

    while (true) { // Aggregate up
        if (buffer_counts[depth][task_id] == width) { // We have all the subresults
            double sum = 0.0;
            std::size_t start_idx = width * (prev_task_id / width);
            for (std::size_t i = 0 ; i < width ; i++) {
                sum += buffer_results[depth + 1][start_idx + i];
            }

            buffer_results[depth][task_id] = sum / buffer_results[depth][task_id];
            
            depth--;
            prev_task_id = task_id;
            task_id /= width;
            
            buffer_results[depth][task_id] += 1.0;
            buffer_counts[depth][task_id] += 1;
            if (buffer_counts[0][0] == width) return true; // We have the final result
        } else return false;
    }

    return false;
}

void Comp::move_recursive_parallel(
    Board &board,
    std::vector<std::vector<double>> &buffer_results,
    std::vector<std::vector<std::size_t>> &buffer_counts,
    std::size_t depth
) {
    bool player_move = depth % 2 == 1;
    std::size_t width = board.get_width();

    MoveResult move_result;
    MPI_Status status;
    double result;

    for (std::size_t i = 0 ; i < width ; i++) {
        move_result = board.place(i, player_move);

        if (move_result == MoveResult::Invalid) {
            save_result_parallel(i, 0.0, width, buffer_results, buffer_counts, depth, false);
            continue;
        }
        
        if (move_result == MoveResult::Win) {
            save_result_parallel(i , (player_move ? -1.0 : 1.0), width, buffer_results, buffer_counts, depth);
        } else if (depth != max_depth) {
            if (depth == solo_depth) {
                MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                if (status.MPI_TAG == TAG_SPECIAL) { // If asking for the first task
                    MPI_Recv(nullptr, 0, MPI_BYTE, status.MPI_SOURCE, TAG_SPECIAL, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                } else { // We're getting a result
                    MPI_Recv(&result, 1, MPI_DOUBLE, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                }

                // Send new task:
                MPI_Send(board.get_spots_data(), board.get_width() * board.get_height(), MPI_CHAR, status.MPI_SOURCE, i, MPI_COMM_WORLD);
                MPI_Send(board.get_heights_data(), board.get_width(), MPI_CHAR, status.MPI_SOURCE, i, MPI_COMM_WORLD);

                if (status.MPI_TAG != TAG_SPECIAL) { // Store result
                    save_result_parallel(status.MPI_TAG, result, width, buffer_results, buffer_counts);
                }
            } else {
                move_recursive_parallel(board, buffer_results, buffer_counts, depth + 1);
            }
        }

        board.remove(i);
    }
}

bool Comp::move_parallel(Board &board) {
    MPI_Status status;

    std::size_t width = board.get_width();

    // Aggregating result from buffer[solo_depth + 1] to buffer[0]
    // Buffer results keeps the results and counts of valid results
    // Buffer coutns keeps the count of completed children for a node
    std::vector<std::vector<double>> buffer_results;
    std::vector<std::vector<std::size_t>> buffer_counts;
    std::size_t size = 1;
    for (std::size_t i = 0 ; i <= solo_depth + 1 ; i++) {
        buffer_results.push_back(std::vector<double>(size, 0.0));
        if (i != solo_depth + 1) buffer_counts.push_back(std::vector<std::size_t>(size, 0));
        size *= width;
    }

    double result;

    for (std::size_t i = 0 ; i < width ; i++) {
        MoveResult move_result = board.place(i, false);
        if (move_result == MoveResult::Invalid) {
            save_result_parallel(i, 0.0, width, buffer_results, buffer_counts, 0, false);
            continue;
        }
        else if (move_result == MoveResult::Win) { // Could return true immediately, but would complicate things with the active workers
            save_result_parallel(i, 1.0, width, buffer_results, buffer_counts, 0);
            board.remove(i);
            continue;
        }
        
        if (max_depth > 0) {
            if (solo_depth == 0) {
                MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                if (status.MPI_TAG == TAG_SPECIAL) { // If asking for the first task
                    MPI_Recv(nullptr, 0, MPI_BYTE, status.MPI_SOURCE, TAG_SPECIAL, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                } else { // We're getting a result
                    MPI_Recv(&result, 1, MPI_DOUBLE, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                }

                // Send new task:
                MPI_Send(board.get_spots_data(), board.get_width() * board.get_height(), MPI_CHAR, status.MPI_SOURCE, i, MPI_COMM_WORLD);
                MPI_Send(board.get_heights_data(), board.get_width(), MPI_CHAR, status.MPI_SOURCE, i, MPI_COMM_WORLD);

                if (status.MPI_TAG != TAG_SPECIAL) { // Store result
                    save_result_parallel(status.MPI_TAG, result, width, buffer_results, buffer_counts);
                }
            } else {
                move_recursive_parallel(board, buffer_results, buffer_counts);
            }
        };

        board.remove(i);
    }

    // Wait for everyone to finish
    while (true) {
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if (status.MPI_TAG == TAG_SPECIAL) {
            MPI_Recv(nullptr, 0, MPI_BYTE, status.MPI_SOURCE, TAG_SPECIAL, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        } else {
            MPI_Recv(&result, 1, MPI_DOUBLE, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
            if (save_result_parallel(status.MPI_TAG, result, width, buffer_results, buffer_counts)) {
                double best_score = -2;
                std::size_t best_move = -1;

                for (std::size_t i = 0 ; i < width ; i++) {
                    double score = buffer_results[1][i];
                    if (score > best_score) {
                        best_score = score;
                        best_move = i;
                    }
                }

                return board.place(best_move, false) == MoveResult::Win;
            }
        }
    };
}