/*
Korištenjem MPI funkcija Send i Recv
napišite niz instrukcija koji će sve elemente zadane kružne liste
postaviti na srednju vrijednost toga i dvaju susjednih elemenata
(indeksi i, i+1, i-1; posljednji element povezan je s prvim i obrnuto).
Svaki MPI proces ima u lokalnoj memoriji samo jedan element liste koji je realna vrijednost.
Program treba jamčiti ispravnost rada bez obzira na veličinu poruka
(ne smije doći do potpunog zastoja zbog redoslijeda slanja i primanja)!
*/

/*
Using MPI functions Send and Recv,
write a sequence of instructions that will set every element of a given circular list
to the average value of itself and its two neighboring elements
(indices i, i+1, i−1; the last element is connected to the first and vice versa).
Each MPI process has only one element of the list in its local memory, which is a real (floating-point) value.
The program must guarantee correct execution regardless of message size
(i.e., it must not result in a deadlock due to the ordering of send and receive operations).
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

    std::mt19937 rng(std::random_device{}() + rank);
    std::uniform_real_distribution<double> dist(0.0, 10.0);
    double myNumber = dist(rng);
    std::cout << rank << "'s number is " << myNumber << std::endl;

    double nbNum1;
    double nbNum2;

    int rankRight = (rank + 1) % worldSize;
    int rankLeft = (rank + worldSize - 1) % worldSize;

    if (rank % 2) { // Odd
        MPI_Recv(&nbNum1, 1, MPI_DOUBLE, rankLeft, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&nbNum2, 1, MPI_DOUBLE, rankRight, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Send(&myNumber, 1, MPI_DOUBLE, rankLeft, 0, MPI_COMM_WORLD);
        MPI_Send(&myNumber, 1, MPI_DOUBLE, rankRight, 0, MPI_COMM_WORLD);
    } else { // Even
        MPI_Send(&myNumber, 1, MPI_DOUBLE, rankRight, 0, MPI_COMM_WORLD);
        MPI_Send(&myNumber, 1, MPI_DOUBLE, rankLeft, 0, MPI_COMM_WORLD);
        MPI_Recv(&nbNum2, 1, MPI_DOUBLE, rankRight, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&nbNum1, 1, MPI_DOUBLE, rankLeft, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    double avg = (myNumber + nbNum1 + nbNum2) / 3;
    std::cout << rank << "'s avg is " << avg << std::endl;

    MPI_Finalize();
    return 0;
}