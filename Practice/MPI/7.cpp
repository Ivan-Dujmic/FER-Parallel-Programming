/*
U MPI programu u nekom trenutku svih N procesa treba obaviti kritični odsječak.
Svaki proces zna svoj redni broj ulaska u K.O.,
ali ne zna redne brojeve ostalih procesa.
Korištenjem MPI funkcija Send i Recv
napisati odsječak programa logaritamske složenosti (po pitanju broja poslanih poruka)
koji će omogućiti da svaki proces sazna indeks svog neposrednog prethodnika i sljedbenika.
*/

/*
In an MPI program, at some point all N processes must execute a critical section.
Each process knows its ordinal number of entry into the critical section,
but does not know the ordinal numbers of the other processes.
Using the MPI functions Send and Recv,
write a program segment with logarithmic complexity (with respect to the number of sent messages)
that will allow each process to determine the index of its immediate predecessor and successor.
*/

#include <mpi.h>
#include <iostream>
#include <sstream>
#include <random>
#include <algorithm>
#include <vector>
#include <climits>

int bit_floor(int x) {
    if (x <= 0) return 0;

    unsigned ux = static_cast<unsigned>(x);
    int bits = sizeof(unsigned) * CHAR_BIT;

    return static_cast<int>(1u << (bits - 1 - __builtin_clz(ux)));
}

bool is_submask(int x, int y) {
    return (x & ~y) == 0;
}

int main()  {
    MPI_Init(nullptr, nullptr);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int ticket;

    // Setup
    if (rank == 0) {
        std::vector<int> tickets;
        tickets.reserve(world_size);
        for (int i = 0 ; i < world_size ; i++) {
            tickets.push_back(i);
        }

        std::mt19937 rng(std::random_device{}());
        std::shuffle(tickets.begin(), tickets.end(), rng);

        std::cout << "Generated tickets:";
        for (int t : tickets) {
            std::cout << ' ' << t;
        }
        std::cout << "\n\n";

        ticket = tickets[0];

        for (int i = 1 ; i < world_size ; i++) {
            MPI_Send(&tickets[i], 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
    } else {
        MPI_Recv(&ticket, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, nullptr);
    }

    std::vector<int> tickets(world_size, -1);
    tickets[rank] = ticket;

    std::vector<int> other_tickets(world_size, 0);

    for (int mask = 1 ; mask < world_size ; mask <<= 1) {
        int partner = rank ^ mask;
        if (partner < world_size) {
            if (rank < partner) { // Receive then send
                MPI_Recv(other_tickets.data(), world_size, MPI_INT, partner, MPI_ANY_TAG, MPI_COMM_WORLD, nullptr);
                for (int i = 0 ; i < world_size ; i++) { // Update with new info
                    if (tickets[i] != -1) continue; // I already know that
                    tickets[i] = other_tickets[i];
                }
                MPI_Send(tickets.data(), world_size, MPI_INT, partner, 0, MPI_COMM_WORLD);
            } else { // Send then receive
                MPI_Send(tickets.data(), world_size, MPI_INT, partner, 0, MPI_COMM_WORLD);
                MPI_Recv(other_tickets.data(), world_size, MPI_INT, partner, MPI_ANY_TAG, MPI_COMM_WORLD, nullptr);
                for (int i = 0 ; i < world_size ; i++) { // Update with new info
                    if (tickets[i] != -1) continue; // I already know that
                    tickets[i] = other_tickets[i];
                }
            }
        }
    }

    /*
    Only the processes with ranks that are submasks of world_size-1 have full info
        Submask here means that ranks don't have set bits where world_size-1 doesn't have set bits
    */
   if (world_size - bit_floor(world_size) != 0) { // If imperfect cube
        if (!is_submask(rank, world_size - 1)) { // I don't have full info
            MPI_Recv(other_tickets.data(), world_size, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, nullptr);
                for (int i = 0 ; i < world_size ; i++) { // Update with new info
                    if (tickets[i] != -1) continue; // I already know that
                    tickets[i] = other_tickets[i];
                }
        }

        for (int mask = 1 ; mask < world_size ; mask <<= 1) {
            int partner = rank ^ mask;
            if (partner >= world_size) break;
            
            if (partner < rank || // Someone else will send them the full info (0 always has full info)
                is_submask(partner, world_size - 1) // They already have full info
            ) continue;

            MPI_Send(tickets.data(), world_size, MPI_INT, partner, 0, MPI_COMM_WORLD);
        }
   }

    std::ostringstream msg;
    msg << rank << " knows:";
    for (int t : tickets) {
        msg << ' ' << t;
    }
    msg << '\n';
    std::cout << msg.str();

    MPI_Finalize();
    return 0;
}