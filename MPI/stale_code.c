/* 	// send last real row down (unless we are bottom PE) */
/* 	if (my_PE_num != npes-1) // not [ROWS-1][0] */
/* 	  MPI_Send(&Temperature[ROWS][1], COLUMNS, MPI_DOUBLE, my_PE_num+1, */
/* 		   99, MPI_COMM_WORLD); */

/* 	// receive into our top ghost row (unless we are top PE) from below */
/* 	if (my_PE_num != 0) // not [0][0] */
/* 	  MPI_Recv(&Temperature_last[0][1], COLUMNS, MPI_DOUBLE, my_PE_num-1, */
/* 		   MPI_ANY_TAG, MPI_COMM_WORLD, &status); */

/* 	// send first real row up (unless we are top PE) */
/* 	if (my_PE_num != 0) // not [1][0] */
/* 	  MPI_Send(&Temperature[1][1], COLUMNS, MPI_DOUBLE, my_PE_num-1, */
/* 		   99, MPI_COMM_WORLD); */

/* 	// receive into our bottom ghost row (unless we are bottom PE) from above */
/* 	if (my_PE_num != npes-1) // not [ROWS][0] */
/* 	  MPI_Recv(&Temperature_last[ROWS+1][1], COLUMNS, MPI_DOUBLE, my_PE_num+1, */
/* 		   MPI_ANY_TAG, MPI_COMM_WORLD, &status); */

	  MPI_Sendrecv(&Temperature[1][1], COLUMNS, MPI_DOUBLE, my_PE_num-1, 99,
		       &Temperature_last[0][1], COLUMNS, MPI_DOUBLE, my_PE_num-1, MPI_ANY_TAG,
		       MPI_COMM_WORLD, &status);
	  MPI_Sendrecv(&Temperature[ROWS][1], COLUMNS, MPI_DOUBLE, my_PE_num-1, 99,
		       &Temperature_last[ROWS+1][1], COLUMNS, MPI_DOUBLE, my_PE_num-1, MPI_ANY_TAG,
		       MPI_COMM_WORLD, &status);
