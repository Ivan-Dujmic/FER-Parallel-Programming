#include <mpi.h>
#include <iostream>
#include <random>
#include <unistd.h>

int main() {
    MPI_Init(NULL, NULL);

    int worldSize;
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    std::mt19937 rng(std::random_device{}() + rank);
    std::uniform_int_distribution<int> dist(3, 7);

    for (int i = dist(rng) ; i > 0 ; i--) {
        // ANSWER
        sleep(1);
    }

    MPI_Finalize();

    return 0;
}