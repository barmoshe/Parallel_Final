#include <mpi.h>
#include <math.h>
#include <stdio.h>

#define ARRAY_SIZE 8

int main(int argc, char** argv) {
    int rank, size, i;
    int A[ARRAY_SIZE] = {1, 2, 3, 4, 5, 6, 7, 8};
    double B[ARRAY_SIZE];
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        // Master process
        int next_i = 0;

        // Send initial tasks to slaves
        for (i = 1; i < size; i++) {
            MPI_Send(&next_i, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            next_i++;
        }

        // Receive results from slaves and send new tasks until all tasks are done
        while (next_i < ARRAY_SIZE) {
            double result;
            MPI_Recv(&result, 1, MPI_DOUBLE, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            B[status.MPI_SOURCE-1] = result;
            MPI_Send(&next_i, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
            next_i++;
        }

        // Receive final results from remaining slaves
        for (i = 1; i < size; i++) {
            double result;
            MPI_Recv(&result, 1, MPI_DOUBLE, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            B[status.MPI_SOURCE-1] = result;
        }

        // Print the results
        for (i = 0; i < ARRAY_SIZE; i++) {
            printf("B[%d] = %f\n", i, B[i]);
        }
    } else {
        // Slave processes
        int i;

        // Receive tasks from master and compute results
        while (1) {
            int next_i;
            MPI_Recv(&next_i, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            if (next_i >= ARRAY_SIZE) {
                break;
            }
            double result = sin(A[next_i]);
            MPI_Send(&result, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
    return 0;
}