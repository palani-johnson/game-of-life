
/* File:     omp_game.c
 *
 * Compile:  gcc -g -Wall -fopenmp -o build/omp_game game.c omp_game.c
 * Run:      ./game [threads] [random|rand] [board_width] [board_height] [seed] [fill] [iterations]
 * 
 * Examples: 
 *      Gen 512x512 game board with seed 50 and fill 50 and save as game.ppm:
 *           ./build/omp_game 16 rand 512 512 50 50 600 > output/game.ppm
 *     
 *      View ppm:
 *           mpv --no-correct-pts --fps=60 output/game.ppm
 *      
 *      Stream game with pipe into mpv:
 *          ./build/omp_game 16 rand 512 512 50 50 600 | mpv --no-correct-pts --fps=60 -
 */
#include <omp.h>
#include "game_of_life.h"

// Copies data into the extra space of a life buffer so that
// it has the topology of a torus
void omp_make_torus(struct GameOfLife *life) {
    int wm1 = life->width-1;
    int hm1 = life->height-1;
    int bm1 = life->buff_size-1;

    #pragma omp for
    for (int j = 0; j < life->height; j++) {
        life->buff[game_pos(life, life->width, j)] = life->buff[game_pos(life, 0, j)];
        life->buff[game_pos(life, -1, j)] = life->buff[game_pos(life, wm1, j)];
    }

    #pragma omp for
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
void omp_gen_next_buff(struct GameOfLife *life) {
    int i, j, w, h, sum, p;

    #pragma omp for
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

void omp_write_video_buffer(struct GameOfLife *life) {
    int i, j, b, c, jh, i3;

    #pragma omp for
    for (j = 0; j < life->height; j++) {
        jh = j*life->height*3;
        for (i = 0; i < life->width; i++) {
            i3 = i*3;
            b = life->buff[game_pos(life, i, j)] ? 0 : 255;
            for(c = 0; c < 3; c++) life->vid_buff[i3 + jh + c] = b;
        }
    }
}

int main(int argc, char** argv) {
    if (argc != 8) {
        fprintf(
            stderr, 
            "usage:  %s [threads] [random|rand] [board_width] [board_height] [seed] [fill] [iterations]\n", 
            argv[0]
        );
        exit(EXIT_FAILURE);
    };

    int threads = strtol(argv[1], NULL, 10);

    struct GameOfLife life_struct, *life;
    life = &life_struct;

    init_life(
        life,
        argv[2], 
        strtol(argv[3], NULL, 10), 
        strtol(argv[4], NULL, 10),
        strtol(argv[5], NULL, 10),
        strtol(argv[6], NULL, 10)
    );

    int iterations = strtol(argv[6], NULL, 10);

    #pragma omp parallel num_threads(threads)
    {
        omp_write_video_buffer(life);

        #pragma omp single
        if (DO_IO) ppm_write(life, stdout);

        for (int i = 0; i < iterations; i++) {
            if (DO_TORIS) omp_make_torus(life);
            omp_gen_next_buff(life);

            #pragma omp single
            iterate_buff(life);

            omp_write_video_buffer(life);

            #pragma omp single
            if (DO_IO) ppm_write(life, stdout);
        }
    }
    

    free_buffs(life);

    return EXIT_SUCCESS;
}