#include <stdio.h>
#include "mpi.h"

void main(int argc, char** argv) {

  int mpi_rank
  MPI_Status mpi_status;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

  int data_mine = mpi_rank * 10;
  int data_neighbor;

  /* Determine our neighbors.
     If we are the max rank, our upper neighbor is 0 (loop around).
     If we are rank 0, our lower neighbor is mpi_size - 1. */


  /* Send the data to our upwards neighbor. */

/*   MPI_Send(&data_mine, 1, MPI_INT, neighbor_upper, 99, MPI_COMM_WORLD); */
/*   MPI_Recv(&data_neighbor, 1, MPI_INT, neighbor_lower, 99, MPI_COMM_WORLD, &mpi_status); */

  for (int i = 0; i < mpi_size; i++) {
    if (mpi_rank == i) {
      MPI_Send(&data_mine, 1, MPI_INT, neighbor_upper, 99, MPI_COMM_WORLD);
      MPI_Recv(&data_neighbor, 1, MPI_INT, neighbor_lower, 99, MPI_COMM_WORLD, &mpi_status);
    }
  }

  for (int i = 0; i < mpi_size; i++) {
    MPI_Barrier(MPI_COMM_WORLD);
    if (mpi_rank == i)
      printf("I am PE %d and my new data is %d.\n", mpi_rank, data_neighbor);
  }

  MPI_Finalize();

}
