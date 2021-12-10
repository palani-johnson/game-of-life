#! /usr/bin/bash

echo "./build/serial_game"
for size in {6..11}
do
    n=$((2**$size))
    echo "$n :"
    ./build/serial_game rand $n $n 50 50 600 > output/game.ppm
done


echo "./build/omp_game"
for threads in {0..4}
do
    t=$((2**$threads))
    for size in {6..11}
    do
        n=$((2**$size))
        echo "($t, $n) :"
        ./build/omp_game $t rand $n $n 50 50 600 > output/game.ppm
    done
done


echo "./build/cuda_game"
for size in {6..11}
do
    n=$((2**$size))
    echo "$n :"
    prime-run ./build/cuda_game rand $n $n 50 50 600 > output/game.ppm
done


echo "./build/mpi_game"
for threads in {0..3}
do
    t=$((2**$threads))
    for size in {6..11}
    do
        n=$((2**$size))
        echo "($t, $n) :"
        mpiexec -n $t ./build/mpi_game rand $n $n 50 50 600 > output/game.ppm
    done
done