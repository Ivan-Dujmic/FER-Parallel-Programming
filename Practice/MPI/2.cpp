/*
U jednom trenutku rada paralelnog programa u n procesa se nalaze neki podaci.
Potrebno je odrediti najveći element od svih n podataka
i tu informaciju (vrijednost najvećega) proslijediti svim procesima.
Napisati algoritam koji će obaviti taj zadatak pomoću MPI funkcija MPI_Send i MPI_Recv.
Uputa: slanje i primanje poruka obaviti u obliku lanca u dva prolaza
(s lijeva na desno te potom s desna na lijevo po svim procesima).
*/

/*
At a certain point in the execution of a parallel program with n processes, some data are present.
It is necessary to determine the largest element among all n data
and to propagate this information (the value of the largest element) to all processes.
Write an algorithm that performs this task using the MPI functions MPI_Send and MPI_Recv.
Hint: Perform sending and receiving messages in the form of a chain in two passes
(first from left to right, then from right to left across all processes).
*/

#include <mpi.h>
#include <iostream>
#include <random>

int main() {
    MPI_Init(NULL, NULL);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int worldSize;
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

    if (worldSize == 1) {
        std::cout << "Need at least 2 processes!\n";
        return -1;
    }

    int myNumber, otherNumber;

    std::mt19937 rng(std::random_device{}() + rank);
    std::uniform_int_distribution<int> dist(0, 100);
    myNumber = dist(rng);
    std::cout << rank << "'s number is " << myNumber << '\n';

    if (rank == 0) {
        MPI_Send(&myNumber, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        MPI_Recv(&otherNumber, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::cout << rank << " knows that max is " << otherNumber << '\n';
    } else if (rank == worldSize - 1) {
        MPI_Recv(&otherNumber, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if (myNumber > otherNumber) otherNumber = myNumber;
        std::cout << rank << " knows that max is " << otherNumber << '\n';
        MPI_Send(&otherNumber, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD);
    } else {
        MPI_Recv(&otherNumber, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if (myNumber > otherNumber) otherNumber = myNumber;
        MPI_Send(&otherNumber, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
        MPI_Recv(&otherNumber, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::cout << rank << " knows that max is " << otherNumber << '\n';
        MPI_Send(&otherNumber, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}