#! /usr/bin/bash

echo "*******************************************"
echo "********** Building serial game ***********"
gcc -g -Wall -o build/serial_game game.c serial_game.c

echo "********** Building openMP game ***********"
gcc -g -Wall -fopenmp -o build/omp_game game.c omp_game.c

echo "********** Building cuda game *************"
nvcc cuda_game.cu game.c -o build/cuda_game

echo "********** Building MPI game **************"
mpicc -g -Wall -o build/mpi_game game.c mpi_game.c

echo "*******************************************"