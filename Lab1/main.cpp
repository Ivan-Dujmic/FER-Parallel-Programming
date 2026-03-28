#include <mpi.h>
#include <iostream>
#include <random>
#include <unistd.h>

int main() {
    MPI_Init(NULL, NULL);

    int worldSize;
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

    if (worldSize < 2) {
        std::cout << "Need at least 2 processes!\n";
        exit(-1);
    }

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Assume counterclockwise is counting up ranks
    int rankLeft = (rank + worldSize - 1) % worldSize;
    int rankRight = (rank + 1) % worldSize;
    std::string indent(2 * rank, '\t');

    std::mt19937 rng(std::random_device{}() + rank);
    std::uniform_int_distribution<int> dist(4, 8);

    int flag; // Store flag
    MPI_Status status; // Store status
    MPI_Request request; // Store request
    char forkRequest; // Store received fork request

    // You send in the other person's perspective of left/right and receive in your perspective
    char forkLeft = 'L'; // What you send when you need someone's left or you're sending someone their left
    char forkRight = 'R'; // What you send when you need someone's right or you're sending someone their right

    bool hasLeft = (rank == 0); // Initially, only the zeroth has the right fork
    bool hasRight = (rank != (worldSize - 1)); // Initially, everyone except the last has the left fork

    bool leftDirty = true;
    bool rightDirty = true;

    // Does someone need my forks?
    bool requestLeft = false;
    bool requestRight = false;

    while (true) {
        std::cout << indent << "Thinking" << std::endl;
        for (int i = dist(rng) ; i > 0 ; i--) {
            for (int i = 0 ; i <= 1 ; i++) {
                MPI_Iprobe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &flag, &status);
                if (flag) {
                    int sender = status.MPI_SOURCE;
                    MPI_Recv(&forkRequest, 1, MPI_CHAR, sender, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    if (forkRequest == forkLeft) {
                        std::cout << indent << "Give left" << std::endl;
                        MPI_Send(&forkRight, 1, MPI_CHAR, sender, 0, MPI_COMM_WORLD);
                        hasLeft = false;
                    } else {
                        std::cout << indent << "Give right" << std::endl;
                        MPI_Send(&forkLeft, 1, MPI_CHAR, sender, 0, MPI_COMM_WORLD);
                        hasRight = false;
                    }
                } else {
                    break;
                }
            }
            sleep(1);
        }

        if (!hasLeft) {
            std::cout << indent << "Needs left" << std::endl;
            MPI_Isend(&forkRight, 1, MPI_CHAR, rankLeft, 0, MPI_COMM_WORLD, &request);
        }
        if (!hasRight) {
            std::cout << indent << "Needs right" << std::endl;
            MPI_Isend(&forkLeft, 1, MPI_CHAR, rankRight, 0, MPI_COMM_WORLD, &request);
        }

        while (!hasLeft || !hasRight) {
            MPI_Iprobe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &flag, &status);
            if (flag) {
                int sender = status.MPI_SOURCE;
                MPI_Recv(&forkRequest, 1, MPI_CHAR, sender, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                if (forkRequest == forkLeft) {
                    if (hasLeft) {
                        if (leftDirty) {
                            std::cout << indent << "Give left" << std::endl;
                            MPI_Send(&forkRight, 1, MPI_CHAR, rankLeft, 0, MPI_COMM_WORLD);
                            hasLeft = false;
                            std::cout << indent << "Needs left" << std::endl;
                            MPI_Send(&forkRight, 1, MPI_CHAR, rankLeft, 0, MPI_COMM_WORLD);
                        } else {
                            std::cout << indent << "Left wants" << std::endl;
                            requestLeft = true;
                        }
                    } else {
                        std::cout << indent << "Left received" << std::endl;
                        hasLeft = true;
                        leftDirty = false;
                    }
                } else {
                    if (hasRight) {
                        if (rightDirty) {
                            std::cout << indent << "Give right" << std::endl;
                            MPI_Send(&forkLeft, 1, MPI_CHAR, rankRight, 0, MPI_COMM_WORLD);
                            hasRight = false;
                            std::cout << indent << "Needs Right" << std::endl;
                            MPI_Send(&forkLeft, 1, MPI_CHAR, rankRight, 0, MPI_COMM_WORLD);
                        } else {
                            std::cout << indent << "Right wants" << std::endl;
                            requestRight = true;
                        }
                    } else {
                        std::cout << indent << "Right received" << std::endl;
                        hasRight = true;
                        rightDirty = false;
                    }
                }
            }
        }

        std::cout << indent << "Eating" << std::endl;
        sleep(dist(rng));
        leftDirty = true;
        rightDirty = true;

        if (requestLeft) {
            std::cout << indent << "Give left" << std::endl;
            MPI_Send(&forkRight, 1, MPI_CHAR, rankLeft, 0, MPI_COMM_WORLD);
            hasLeft = false;
            requestLeft = false;
        }
        if (requestRight) {
            std::cout << indent << "Give right" << std::endl;
            MPI_Send(&forkLeft, 1, MPI_CHAR, rankRight, 0, MPI_COMM_WORLD);
            hasRight = false;
            requestRight = false;
        }
    }

    MPI_Finalize();

    return 0;
}