/*
U MPI programu svaki proces ima lokalnu vrijednost u varijabli x.
Korištenjem MPI funkcija Send i Recv
napisati odsječak programa logaritamske složenosti (po pitanju broja poslanih poruka)
koji će za N procesa izračunati minimum svih lokalnih vrijednosti,
tako da svi procesi znaju rezultat.
*/

/*
In an MPI program, each process has a local value in the variable x.
Using MPI functions Send and Recv
write a code snippet of logarithmic complexity (in terms of the number of messages sent)
that will, for N processes, compute the minimum of all local values,
so that all processes know the result.
*/

#include <mpi.h>
#include <iostream>
#include <random>
#include <cmath>

int main() {
    MPI_Init(NULL, NULL);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int worldSize;
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

    if (worldSize < 1) {
        std::cout << "Need at least 1 process!\n";
        return -1;
    }

    int myNumber, otherNumber;

    std::mt19937 rng(std::random_device{}() + rank);
    std::uniform_int_distribution<int> dist(0, 100);
    myNumber = dist(rng);
    std::cout << rank << "'s number is " << myNumber << std::endl;

    int mask = 1;
    while (mask < worldSize) {
        int partner = rank ^ mask;
        if (partner < worldSize) {
            if (rank < partner) {
                MPI_Recv(&otherNumber, 1, MPI_INT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                if (otherNumber < myNumber) myNumber = otherNumber;
            } else {
                MPI_Send(&myNumber, 1, MPI_INT, partner, 0, MPI_COMM_WORLD);
            }
        }
        mask <<= 1;
    }

    if (rank == 0) std::cout << "Minimum is " << myNumber << std::endl;

    MPI_Finalize();
    return 0;
}