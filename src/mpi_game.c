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
 *          mpiexec -n 8 ./mpi_game rand  rand 512 512 50 50 600 | mpv --no-correct-pts --fps=60 -
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

void copy_to_mini_life(struct GameOfLife *life, struct GameOfLife *mini_life) {
    memcpy(
        mini_life->buff, 
        &(life->buff[game_pos(life, (THIS_PROCESS*mini_life->height) - 1, -1)]),
        mini_life->buff_size * sizeof(bool)
    );
}

int main(int argc, char** argv) {
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &PROCESSES);
    MPI_Comm_rank(MPI_COMM_WORLD, &THIS_PROCESS);

    struct GameOfLife life_struct, *life, mini_life_struct, *mini_life;
    life = &life_struct;
    mini_life = &mini_life_struct;
    int life_items[3];

    if(THIS_PROCESS == 0) {
        error_if(
            argc != 7, 
            "usage:  %s [random|rand] [board_width] [board_height] [seed] [fill] [iterations]\n",
            argv[0]
        )

        life_items[0] = strtol(argv[2], NULL, 10);
        life_items[1] = strtol(argv[2], NULL, 10);
        life_items[2] = strtol(argv[6], NULL, 10);

        error_if(
            life_items[1] % PROCESSES != 0, 
            "Height mod %d (processes) must be 0", 
            PROCESSES
        )

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

    life_calc_derived(mini_life, life_items[0], life_items[1] / PROCESSES);
    life_alloc_buffs(mini_life);
    if(THIS_PROCESS != 0) {
        life_calc_derived(life, life_items[0], life_items[1]);
        life_alloc_buffs(life);
        free(life->vid_buff);
    }
    MPI_Bcast(life->buff, life->buff_size, MPI_BYTE, 0, MPI_COMM_WORLD);
    copy_to_mini_life(life, mini_life);    

    write_video_buffer(mini_life);
    printf("rank %d %d %d\n", THIS_PROCESS, mini_life->vid_buff_size, life->vid_buff_size);
    MPI_Gather(
        mini_life->vid_buff,
        mini_life->vid_buff_size, 
        MPI_CHAR,
        life->vid_buff,
        life->vid_buff_size,
        MPI_CHAR,
        0,
        MPI_COMM_WORLD
    );
    //if(THIS_PROCESS == 0 && DO_IO) ppm_write(life, stdout);

    // for (int i = 0; i < life_items[3]; i++) {
    //     gen_next_buff(mini_life);
    //     MPI_Allgather(
    //         &(mini_life->buff[game_pos(mini_life, 0, -1)]),
    //         mini_life->buff_size - (mini_life->width - 2) * 2, 
    //         MPI_CHAR,
    //         &(life->buff[game_pos(life, 0, -1)]),
    //         life->buff_size - (life->width - 2) * 2,
    //         MPI_CHAR,
    //         MPI_COMM_WORLD
    //     );
    //     copy_to_mini_life(life, mini_life);

    //     write_video_buffer(mini_life);
    //     MPI_Gather(
    //         mini_life->vid_buff,
    //         mini_life->vid_buff_size, 
    //         MPI_CHAR,
    //         life->vid_buff,
    //         life->vid_buff_size,
    //         MPI_CHAR,
    //         0,
    //         MPI_COMM_WORLD
    //     );
    //     //if(THIS_PROCESS == 0 && DO_IO) ppm_write(life, stdout);
    // }

    // free_buffs(life);
    //free_buffs(mini_life);
    MPI_Finalize();

    return EXIT_SUCCESS;
}