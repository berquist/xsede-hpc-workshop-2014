#include <stdio.h>
#include "mpi.h"

void main(int argc, char** argv) {

  int mpi_rank, mpi_size;
  MPI_Status mpi_status;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

  /* Exercise 1: Write a code that runs on 8 PEs and does a “circular shift.”
     This means that every PE sends some data to its nearest neighbor either
     “up” (one PE higher) or “down.” To make it circular, PE 7 and PE 0 are
     treated as neighbors. Make sure that whatever data you send is received. */

  int data_mine = mpi_rank * 10;
  int data_neighbor;

  /* Determine our neighbors.
     If we are the max rank, our upper neighbor is 0 (loop around).
     If we are rank 0, our lower neighbor is mpi_size - 1. */

  int neighbor_upper = mpi_rank + 1;
  if (neighbor_upper == mpi_size)
    neighbor_upper = 0;

  /* Send the data to our upwards neighbor. */

  for (int i = 0; i < mpi_size; i++) {
    if (mpi_rank == i) {
      MPI_Send(&data_mine, 1, MPI_INT, neighbor_upper, 99, MPI_COMM_WORLD);
      MPI_Recv(&data_neighbor, 1, MPI_INT, MPI_ANY_SOURCE, 99, MPI_COMM_WORLD, &mpi_status);
    }
  }

  for (int i = 0; i < mpi_size; i++) {
    MPI_Barrier(MPI_COMM_WORLD);
    if (mpi_rank == i)
      printf("I am PE %d and my new data is %d.\n", mpi_rank, data_neighbor);
  }

  MPI_Finalize();

}
