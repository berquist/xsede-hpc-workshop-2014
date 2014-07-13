#include <stdio.h>
#include "mpi.h"

void main(int argc, char** argv) {

  int mpi_rank;
  MPI_Status mpi_status;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

  /* Exercise 2: Write, using only the routines that we have covered in the
     first three examples, (MPI_Init, MPI_Comm_Rank, MPI_Send, MPI_Recv,
     MPI_Barrier, MPI_Finalize) a program that determines how many PEs it is
     running on. It should perform as the following:
       `aprun –n 4 exercise`
       I am running on 4 PEs.
       `aprun –n 6 exercise`
       I am running on 6 PEs.
     You would normally obtain this information with the simple MPI_Comm_size()
     routine. The solution may not be as simple as it first seems. Remember,
     make no assumptions about when any given message may be received. */

  int counter_mine = 1;
  int counter_neighbor = 0;

  int neighbor_lower = mpi_rank - 1;

  if (mpi_rank > 0) {
    MPI_Send(&counter_mine, 1, MPI_INT, neighbor_lower, 99, MPI_COMM_WORLD);
    MPI_Recv(&counter_neighbor, 1, MPI_INT, MPI_ANY_SOURCE, 99, MPI_COMM_WORLD, &mpi_status);
    counter_mine = counter_neighbor + 1;
    printf("PE: %d total count: %d\n", mpi_rank, counter_mine);
  }

  MPI_Barrier(MPI_COMM_WORLD);

  if (mpi_rank == 0) {
    MPI_Recv(&counter_neighbor, 1, MPI_INT, MPI_ANY_SOURCE, 99, MPI_COMM_WORLD, &mpi_status);
    printf("PE: %d total count: %d\n", mpi_rank, counter_mine);
  }

  if (mpi_rank == 0)
    printf("I am running on %d PEs.\n", counter_mine);

  MPI_Finalize();

}
