#! /usr/bin/bash
build="./build"
src="./src"

echo "*******************************************"
echo "********** Building serial game ***********"
gcc -g -Wall -o $build/serial_game $src/game.c $src/serial_game.c

echo "********** Building openMP game ***********"
gcc -g -Wall -fopenmp -o $build/omp_game $src/game.c $src/omp_game.c

echo "********** Building cuda game *************"
nvcc $src/cuda_game.cu $src/game.c -o $build/cuda_game

echo "********** Building MPI game **************"
mpicc -g -Wall -o $build/mpi_game $src/game.c $src/mpi_game.c

echo "*******************************************"