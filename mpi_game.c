/* File:     mpi_game.c
 *
 * Compile:  mpicc -g -Wall -o mip_game game.c mpi_game.c
 * Run:      ./mpi_game [random|rand] [board_width] [board_height] [seed] [fill] [iterations]
 * 
 * Examples: 
 *      Gen 500x500 game board with seed 50 and fill 50 and save as game.ppm:
 *           ./mpi_game rand 500 500 10 50 600 > game.ppm
 *     
 *      View ppm:
 *           mpv --no-correct-pts --fps=10 game.ppm
 *      
 *      Stream game with pipe into mpv:
 *          ./mpi_game rand 500 500 10 50 600 | mpv --no-correct-pts --fps=10 -
 */
#include <mpi.h>
#include "game_of_life.h"

#define error_if(check, msg, v1) if (check) { \
    fprintf(stderr, msg, v1); \
    MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE); \
    MPI_Finalize(); \
    exit(EXIT_FAILURE); } 

int PROCESSES, THIS_PROCESS;

// Copies data into the extra space of a life buffer so that
// it has the topology of a torus
void make_torus(struct GameOfLife *life) {
    int wm1 = life->width-1;
    int hm1 = life->height-1;
    int bm1 = life->buff_size-1;

    for (int j = 0; j < life->height; j++) {
        life->buff[game_pos(life, life->width, j)] = life->buff[game_pos(life, 0, j)];
        life->buff[game_pos(life, -1, j)] = life->buff[game_pos(life, wm1, j)];
    }

    for (int i = 0; i < life->width; i++) {
        life->buff[game_pos(life, i, life->height)] = life->buff[game_pos(life, i, 0)];
        life->buff[game_pos(life, i, -1)] = life->buff[game_pos(life, i, hm1)];
    }

    life->buff[bm1] = life->buff[game_pos(life, 0, 0)];
    life->buff[0] = life->buff[game_pos(life, wm1, hm1)];
    life->buff[life->width + 1] = life->buff[game_pos(life, 0, hm1)];
    life->buff[bm1 - (life->width + 1)] = life->buff[game_pos(life, wm1, 0)];
}

// Creates the next buffer in a life struct
void gen_next_buff(struct GameOfLife *life) {
    int i, j, w, h, sum, p;

    for (j = 0; j < life->height; j++) {
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

void write_video_buffer(struct GameOfLife *life) {
    int i, j, b, c, jh, i3;

    for (j = 0; j < life->height; j++) {
        jh = j * (life->height) * 3;
        for (i = 0; i < life->width; i++) {
            i3 = i*3;
            b = life->buff[game_pos(life, i, j)] ? 0 : 255;
            for(c = 0; c < 3; c++) life->vid_buff[i3 + jh + c] = b;
        }
    }
}

void mpi_init_life(
    struct GameOfLife *life, 
    int init_type, 
    int width, 
    int height, 
    int op1, 
    int op2
) {
    life->width = width;
    life->height = height;
    life->buff_size = (life->width + 2) * (life->height + 2);
    life->_w2 = life->width + 2;
    life->_w3 = life->height + 3;

    life->next_buff = malloc(life->buff_size * sizeof(bool));
    if(THIS_PROCESS == 0)
        life->buff = init_type
            ? alloc_random_buff(life, op1, op2)
            : alloc_buff_from_file(life, init_type, op1, op2);
    
    life->vid_buff_size = life->width * life->height * 3;
    life->vid_buff = malloc(life->vid_buff_size * sizeof(char));
}

int main(int argc, char** argv) {
    get_env;
    
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &PROCESSES);
    MPI_Comm_rank(MPI_COMM_WORLD, &THIS_PROCESS);

    int argi[6];

    if(THIS_PROCESS == 0) {
        error_if(
            argc != 7, 
            "usage:  %s [random|rand] [board_width] [board_height] [seed] [fill] [iterations]\n",
            argv[0]
        )
        argi[0] = strcmp(argv[1], "random") || strcmp(argv[0], "rand");
        argi[1] = strtol(argv[2], NULL, 10);
        argi[2] = strtol(argv[3], NULL, 10);
        argi[3] = strtol(argv[4], NULL, 10);
        argi[4] = strtol(argv[5], NULL, 10);
        argi[5] = strtol(argv[6], NULL, 10);

        error_if(argi[1] % PROCESSES == 0, "Width mod %d (processes) must be 0", PROCESSES)
        error_if(argi[2] % PROCESSES == 0, "Height mod %d (processes) must be 0", PROCESSES)
    }

    MPI_Bcast(&argi, 6, MPI_INT, 0, MPI_COMM_WORLD);
    
    struct GameOfLife life_struct, *life;
    life = &life_struct;

    mpi_init_life(life, argi[0], argi[1], argi[2], argi[3], argi[4]);
    MPI_Barrier(MPI_COMM_WORLD);

    write_video_buffer(life);
    MPI_Barrier(MPI_COMM_WORLD);
    if(THIS_PROCESS == 0 && DO_IO) ppm_write(life, stdout);

    for (int i = 0; i < argi[5]; i++) {
        if (DO_TORIS) make_torus(life);

        gen_next_buff(life);
        MPI_Barrier(MPI_COMM_WORLD);

        if (THIS_PROCESS == 0) iterate_buff(life);
        MPI_Barrier(MPI_COMM_WORLD);

        write_video_buffer(life);
        MPI_Barrier(MPI_COMM_WORLD);
        if(THIS_PROCESS == 0 && DO_IO) ppm_write(life, stdout);
    }

    free_buffs(life);
    MPI_Finalize();

    return EXIT_SUCCESS;
}