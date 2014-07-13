CC = cc
CFLAGS = -fast -Mipa=fast -Msmart -O4
ACCFLAGS = -acc -ta=nvidia -Minfo=accel -lcudart -mcmodel=medium
OMPFLAGS = -mp=nonuma
SRCS = $(wildcard *.c)
PROGS = $(patsubst %.c,%.exe,$(SRCS))

all: $(PROGS)

%.exe: %.c
	$(CC) $(CFLAGS) $(OMPFLAGS) $(ACCFLAGS) -o $@ $<

prepare_env:
	module swap PrgEnv-cray PrgEnv-pgi
	module unload pgi
	module load pgi/14.3.0
	module load cudatoolkit

run_hybrid_gpu:
	aprun -n 4 -N 1 laplace_competition_hybrid_gpu.exe

run_hybrid_omp:
	aprun -n 4 -d 16 -N 1 laplace_competition_hybrid_omp.exe

.PHONY: clean

clean:
	rm -f *.o *.out *.exe
