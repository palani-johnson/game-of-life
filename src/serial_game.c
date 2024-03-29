/* File:     serial_game.c
 *
 * Compile:  gcc -g -Wall -o build/serial_game game.c serial_game.c
 * Run:      ./build/serial_game [random|rand] [board_width] [board_height] [seed] [fill] [iterations]
 * 
 * Examples: 
 *      Gen 256x256 game board with seed 50 and fill 50 and save as game.ppm:
 *           ./build/serial_game rand 256 256 50 50 600 > output/game.ppm
 *     
 *      View ppm (I use mpv):
 *           mpv --no-correct-pts --fps=60 output/game.ppm
 *      
 *      Stream game with pipe into mpv:
 *          ./build/serial_game rand 256 256 10 50 600 | mpv --no-correct-pts --fps=60 -
 */
#include "game_of_life.h"

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
        jh = j * (life->width) * 3;
        for (i = 0; i < life->width; i++) {
            i3 = i*3;
            b = life->buff[game_pos(life, i, j)] ? 0 : 255;
            for(c = 0; c < 3; c++) life->vid_buff[i3 + jh + c] = b;
        }
    }
}

int main(int argc, char** argv) {
    setup_timer();

    if (argc != 7) {
        fprintf(
            stderr, 
            "usage:  %s [random|rand] [board_width] [board_height] [seed] [fill] [iterations]\n", 
            argv[0]
        );
        exit(EXIT_FAILURE);
    };

    struct GameOfLife life_struct, *life;
    life = &life_struct;

    init_life(
        life,
        argv[1], 
        strtol(argv[2], NULL, 10), 
        strtol(argv[3], NULL, 10),
        strtol(argv[4], NULL, 10),
        strtol(argv[5], NULL, 10)
    );

    int iterations = strtol(argv[6], NULL, 10);

    write_video_buffer(life);    
    if (DO_IO) ppm_write(life, stdout);

    for (int i = 0; i < iterations; i++) {
        if (DO_TORIS) make_torus(life);
        gen_next_buff(life);
        iterate_buff(life);

        write_video_buffer(life);  
        if (DO_IO) ppm_write(life, stdout);
    }

    free_buffs(life);

    close_timer();
    report_timer();
    return EXIT_SUCCESS;
}