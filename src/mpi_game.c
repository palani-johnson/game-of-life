/* File:     mpi_game.c
 *
 * Compile:  mpicc -g -Wall -o build/mpi_game game.c mpi_game.c
 * Run:      mpiexec -n [processes] ./build/mpi_game [random|rand] [board_width] [board_height] [seed] [fill] [iterations]
 * 
 * Examples: 
 *      Gen 512x512 game board with seed 50 and fill 50 and save as game.ppm:
 *           mpiexec -n 8 ./build/mpi_game rand 512 512 50 50 600 > output/game.ppm
 *     
 *      View ppm:
 *           mpv --no-correct-pts --fps=60 output/game.ppm
 *      
 *      Stream game with pipe into mpv:
 *          mpiexec -n 8 ./build/mpi_game rand 512 512 50 50 600 | mpv --no-correct-pts --fps=60 -
 */
#include <mpi.h>
#include "game_of_life.h"

#define error_if(check, msg, v1) if (check) { \
    fprintf(stderr, msg, v1); \
    MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE); \
    MPI_Finalize(); \
    exit(EXIT_FAILURE); } 

// Creates the next buffer in a life struct
void mpi_gen_next_buff(struct GameOfLife *life, int process, int step) {
    int i, j, w, h, sum, p;

    for (j = process; j < life->height; j += step) {
        for (i = 0; i < life->width; i++) {
            sum = 0;
            for (h = j-1; h <= j+1; h++)
                for (w = i-1; w <= i+1; w++) 
                    if (life->buff[game_pos(life, w, h)]) sum++;
            
            p = game_pos(life, i, j);
            life->next_buff[p] = sum == 3 || (life->buff[p] && sum == 4);
        }
    }
}

void mpi_iterate_buff(struct GameOfLife *life) {
    MPI_Allreduce(
        life->next_buff,
        life->buff,
        life->buff_size,
        MPI_CHAR,
        MPI_LOR,
        MPI_COMM_WORLD
    );
}

void write_video_buffer(struct GameOfLife *life) {
    int i, j, b, c, jh, i3;

    for (j = 0; j < life->height; j++) {
        jh = j * (life->width) * 3;
        for (i = 0; i < life->width; i++) {
            i3 = i*3;
            b = life->buff[game_pos(life, i, j)] ? 0 : 255;
            for(c = 0; c < 3; c++) life->vid_buff[i3 + jh + c] = b;
        }
    }
}

int main(int argc, char** argv) {
    int processes, this_process;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &processes);
    MPI_Comm_rank(MPI_COMM_WORLD, &this_process);

    struct GameOfLife life_struct, *life;
    life = &life_struct;
    int life_items[3];

    if(this_process == 0) {
        error_if(
            argc != 7, 
            "usage:  %s [random|rand] [board_width] [board_height] [seed] [fill] [iterations]\n",
            argv[0]
        )

        life_items[0] = strtol(argv[2], NULL, 10);
        life_items[1] = strtol(argv[2], NULL, 10);
        life_items[2] = strtol(argv[6], NULL, 10);

        init_life(
            life,
            argv[1], 
            life_items[0], 
            life_items[1],
            strtol(argv[4], NULL, 10),
            strtol(argv[5], NULL, 10)
        );
    }

    MPI_Bcast(life_items, 3, MPI_INT, 0, MPI_COMM_WORLD);
    if(this_process != 0) {
        life_calc_derived(life, life_items[0], life_items[1]);
        life_alloc_buffs(life);
    }
    MPI_Bcast(life->buff, life->buff_size, MPI_BYTE, 0, MPI_COMM_WORLD);
    if(this_process == 0 && DO_IO) {
        write_video_buffer(life);
        ppm_write(life, stdout);
    }

    for (int i = 0; i < life_items[2]; i++) {
        mpi_gen_next_buff(life, this_process, processes);
        mpi_iterate_buff(life);

        if(this_process == 0 && DO_IO) {
            write_video_buffer(life);
            ppm_write(life, stdout);
        }
    }

    free_buffs(life);
    MPI_Finalize();
    return EXIT_SUCCESS;
}