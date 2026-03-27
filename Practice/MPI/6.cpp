/*
U MPI programu u nekom trenutku pojavio se promatrani događaj unutar jednog MPI procesa.
Svi procesi znaju da se događaj pojavio,
ali nijedan drugi proces ne zna unutar kojeg procesa se pojavio događaj.
Korištenjem MPI funkcija Send i Recv
napisati odsječak programa logaritamske složenosti (po pitanju broja poslanih poruka)
koji će za N procesa omogućiti da svi procesi saznaju indeks izvorišnog procesa.
*/

/*
In an MPI program, at some point an observed event appeared within one MPI process.
All processes know that the event occurred,
but no other process knows in which process the event appeared.
Using MPI Send and Recv functions,
write a program snippet of logarithmic complexity (in terms of the number of messages sent)
that will allow all N processes to learn the index of the source process.
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

    if (worldSize < 2) {
        std::cout << "Need at least 2 processes!\n";
        return -1;
    }

    // Setup
    int source;
    if (rank == 0) {
        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> dist(0, worldSize - 1);
        source = dist(rng);
    }
    MPI_Bcast(&source, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    if (rank != source) source = -1;

    int p[] = {source, 1}; // Source and step

    // Algorithm
    if (source != -1) {
        std::cout << rank << " is the source" << std::endl;
        MPI_Send(p, 2, MPI_INT, (rank + 1) % worldSize, 0, MPI_COMM_WORLD);
    } else {
        MPI_Recv(p, 2, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::cout << rank << " knows that " << p[0] << " is the source" << std::endl;
        if (worldSize > p[1]) {
            int offset = (p[0] < rank) ? (rank - p[0]) : (worldSize - p[0] + rank); // My offset from source
            int step = p[1]; // Send rank step
            p[1] <<= 1;
            if (offset + step < worldSize) // If it doesn't already know
                MPI_Send(p, 2, MPI_INT, (rank + step) % worldSize, 0, MPI_COMM_WORLD); 
            if (offset + step + 1 < worldSize) // If it doesn't already know
                MPI_Send(p, 2, MPI_INT, (rank + step + 1) % worldSize, 0, MPI_COMM_WORLD); 
        }
    }

    MPI_Finalize();
    return 0;
}