CC = cc
CFLAGS = -acc -ta=nvidia -Minfo=accel

all: prepare_env laplace_competition run

all_no_run: prepare_env laplace_competition

prepare_env:
	module swap PrgEnv-cray PrgEnv-pgi
	module unload pgi
	module load pgi/14.3.0
	module load cudatoolkit

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

laplace_competition: laplace_competition.o
	$(CC) $(CFLAGS) -o laplace_competition.exe laplace_competition.o

laplace_mpi_nonblock: laplace_mpi_nonblock.o
	$(CC) $(CFLAGS) -o laplace_mpi_nonblock.exe laplace_mpi_nonblock.o

laplace_mpi_only: laplace_mpi_only.o
	$(CC) $(CFLAGS) -o laplace_mpi_only.exe laplace_mpi_only.o

laplace_mpi_solution: laplace_mpi_solution.o
	$(CC) $(CFLAGS) -o laplace_mpi_solution.exe laplace_mpi_solution.o

laplace_serial: laplace_serial.o
	$(CC) $(CFLAGS) -o laplace_serial.exe laplace_serial.o

laplace_your_mpi: laplace_your_mpi.o
	$(CC) $(CFLAGS) -o laplace_your_mpi.exe laplace_your_mpi.o

run:
	laplace_competition.exe

.PHONY: clean

clean:
	rm -f *.o *.out *.exe
