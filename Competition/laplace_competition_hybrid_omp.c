/****************************************************************
 * Laplace MPI Template C Version                                         
 *                                                               
 * T is initially 0.0                                            
 * Boundaries are as follows                                     
 *                                                               
 *                T                      4 sub-grids            
 *   0  +-------------------+  0    +-------------------+       
 *      |                   |       |                   |           
 *      |                   |       |-------------------|         
 *      |                   |       |                   |      
 *   T  |                   |  T    |-------------------|             
 *      |                   |       |                   |     
 *      |                   |       |-------------------|            
 *      |                   |       |                   |   
 *   0  +-------------------+ 100   +-------------------+         
 *      0         T       100                                    
 *                                                                 
 * Each PE only has a local subgrid.
 * Each PE works on a sub grid and then sends         
 * its boundaries to neighbors.
 *                                                                 
 *  John Urbanic, PSC 2014
 *
 *******************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <mpi.h>

#define COLUMNS      10000
#define ROWS_GLOBAL  10000      // this is a "global" row count
#define NPES            4      // number of processors
#define ROWS (ROWS_GLOBAL/NPES) // number of real local rows

// communication tags
#define DOWN     100
#define UP       101   

#define MAX_TEMP_ERROR 0.01

double Temperature[ROWS+2][COLUMNS+2];
double Temperature_last[ROWS+2][COLUMNS+2];

void initialize(int npes, int my_PE_num);
void track_progress(int iter);
void output(int mpi_rank, int iteration);


int main(int argc, char *argv[]) {

  int i, j;
  int max_iterations;
  int iteration = 1;
  double dt;
  struct timeval start_time, stop_time, elapsed_time;

  int        npes;                // number of PEs
  int        my_PE_num;           // my PE number
  double     dt_global = 100;     // delta t across all PEs
  MPI_Status status;              // status returned by MPI calls
  MPI_Request request;

  // the usual MPI startup routines
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_PE_num);
  MPI_Comm_size(MPI_COMM_WORLD, &npes);

  // verify only NPES PEs are being used
  if (npes != NPES) {
    if (my_PE_num == 0)
      printf("This code must be run with %d PEs\n", NPES);
    MPI_Finalize();
    exit(1);
  }

  // PE 0 asks for input
  // `max_iterations` undefined on all other PEs!
  if (my_PE_num == 0) {
    printf("Maximum iterations [100-4000]?\n");
    fflush(stdout);
    scanf("%d", &max_iterations);
  }

  // bcast max iterations to other PEs
  MPI_Bcast(&max_iterations, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (my_PE_num==0) gettimeofday(&start_time, NULL);

  initialize(npes, my_PE_num);

  while ( dt_global > MAX_TEMP_ERROR && iteration <= max_iterations ) {

    // main calculation: average my four neighbors
#pragma omp parallel for private (i, j)
    for (i = 1; i <= ROWS; i++) {
      for (j = 1; j <= COLUMNS; j++) {
	Temperature[i][j] = 0.25 * (Temperature_last[i+1][j] + Temperature_last[i-1][j] +
				    Temperature_last[i][j+1] + Temperature_last[i][j-1]);
      }
    }

    // COMMUNICATION PHASE: send and receive ghost rows for next iteration
    /*
      There is a top and a bottom ghost row for all PEs, except for:
      0: only a bottom
      npes-1: only a top
      send Temperature, receive Temperature_last
      COLUMNS, not COLUMNS+2
      all the indexing issues seem to stem from (mis)understanding that the boundary
      conditions don't change
    */

    if (my_PE_num != 0) {
      // send second row to my_PE_num-1
      // receive first row from my_PE_num-1
      MPI_Isend(&Temperature[1][1], COLUMNS, MPI_DOUBLE, my_PE_num-1,
		99, MPI_COMM_WORLD, &request);
      MPI_Recv(&Temperature_last[0][1], COLUMNS, MPI_DOUBLE, my_PE_num-1,
	       MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    }

    if (my_PE_num != npes-1) {
      // send second to last row to my_PE_num+1
      // receive last row from my_PE_num+1
      MPI_Isend(&Temperature[ROWS][1], COLUMNS, MPI_DOUBLE, my_PE_num+1,
		99, MPI_COMM_WORLD, &request);
      MPI_Recv(&Temperature_last[ROWS+1][1], COLUMNS, MPI_DOUBLE, my_PE_num+1,
	       MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    }

    dt = 0.0;

#pragma omp parallel for private (i, j) reduction (max: dt)
    for (i = 1; i <= ROWS; i++) {
      for (j = 1; j <= COLUMNS; j++) {
	dt = fmax( fabs(Temperature[i][j]-Temperature_last[i][j]), dt);
	Temperature_last[i][j] = Temperature[i][j];
      }
    }

    // find global dt
    MPI_Reduce(&dt, &dt_global, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Bcast(&dt_global, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // periodically print test values - only for PE in lower corner
    if ((iteration % 100) == 0) {
      if (my_PE_num == npes-1) {
	track_progress(iteration);
	fflush(stdout);
      }
/*       if (my_PE_num == 2) { */
/* 	printf(" Global coord [750, 900] is: %f\n", Temperature[250][900]); */
/* 	fflush(stdout); */
/*       } */
      //output(my_PE_num, iteration);
    }

    iteration++;
  }

  // Slightly more accurate timing and cleaner output 
  MPI_Barrier(MPI_COMM_WORLD);

  // PE 0 finish timing and output values
  if (my_PE_num == 0) {
    gettimeofday(&stop_time, NULL);
    timersub(&stop_time, &start_time, &elapsed_time);

    printf("\nMax error at iteration %d was %f\n", iteration-1, dt_global);
    printf("Total time was %f seconds.\n", elapsed_time.tv_sec+elapsed_time.tv_usec/1000000.0);
  }

  MPI_Finalize();
}


void initialize(int npes, int my_PE_num) {

  double tMin, tMax;  // Local boundary limits
  int i, j;

#pragma omp parallel private(i, j)
  {
    // blank the entire Temperature_last matrix
#pragma omp for
    for (i = 0; i <= ROWS+1; i++) {
      for (j = 0; j <= COLUMNS+1; j++) {
	Temperature_last[i][j] = 0.0;
      }
    }

    /* Set left side to 0 and right to a linear increase
       Set top to 0 and bottom to a linear increase */

    // Local boundry condition endpoints
    tMin = (my_PE_num)*100.0/npes;
    tMax = (my_PE_num+1)*100.0/npes;

    // Left and right boundaries (all PEs)
#pragma omp for
    for (i = 0; i <= ROWS+1; i++) {
      Temperature_last[i][0] = 0.0;
      Temperature_last[i][COLUMNS+1] = tMin + ((tMax-tMin)/ROWS)*i;
    }

    // Top boundary (PE 0 only)
    if (my_PE_num == 0)
      for (j = 0; j <= COLUMNS+1; j++)
	Temperature_last[0][j] = 0.0;

    // Bottom boundary (Last PE only)
    if (my_PE_num == npes-1)
      for (j = 0; j <= COLUMNS+1; j++)
	Temperature_last[ROWS+1][j] = (100.0/COLUMNS)*j;
  }

}


// only called by last PE
void track_progress(int iteration) {

  int i;

  printf("---------- Iteration number: %d ------------\n", iteration);

  // output global coordinates so user doesn't have to understand decomposition
  for (i = 5; i >= 0; i--) {
    printf("[%d,%d]: %5.2f  ", ROWS_GLOBAL-i, COLUMNS-i, Temperature[ROWS-i][COLUMNS-i]);
  }
  printf("\n");
}


void output(int mpi_rank, int iteration) {

  FILE* fp;
  char filename[50];

  sprintf(filename, "output%d.txt", iteration);

  for (int pe = 0; pe < 4; pe++) {
    if (pe == mpi_rank) {
      
      fp = fopen(filename, "a");

      for (int y = 1; y <= ROWS; y++) {
	for (int x = 1; x <= COLUMNS; x++) {
	    fprintf(fp, "%5.2f ", Temperature[y][x]);
	}
	fprintf(fp, "\n");
      }
      fflush(fp);
      fclose(fp);
    }
    
    MPI_Barrier(MPI_COMM_WORLD);
  }
}
