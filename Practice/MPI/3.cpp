/*
Korištenjem MPI funkcija Send i Recv napisati odsječak programa (proizvoljne složenosti)
koji će za N procesa ostvariti funkciju MPI_Barrier,
tj. postići da svi procesi moraju doći do istog odsječka
prije nego bilo koji proces može nastaviti s izvođenjem.
*/

/*
Using MPI functions Send and Recv, write a code segment (of arbitrary complexity) that,
for N processes, implements the functionality of MPI_Barrier — 
i.e., ensures that all processes must reach the same point
before any process can continue execution.
*/

#include <mpi.h>
#include <iostream>

int main() {
    MPI_Init(NULL, NULL);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int worldSize;
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

    if (rank == 0) { // Conductor
        //WORK

        { // BARRIER
            for (int i = 0 ; i < worldSize - 1 ; i++) {
                MPI_Recv(nullptr, 0, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            for (int i = 1 ; i < worldSize ; i++) {
                MPI_Send(nullptr, 0, MPI_INT, i, 0, MPI_COMM_WORLD);
            }
        }

        std::cout << rank << " has finished waiting for everyone!" << std::endl;
    } else { // Worker
        // WORK

        { // BARRIER
            MPI_Send(nullptr, 0, MPI_INT, 0, 0, MPI_COMM_WORLD);
            MPI_Recv(nullptr, 0, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        std::cout << rank << " has finished waiting for everyone!" << std::endl;
    }

    MPI_Finalize();
    return 0;
}