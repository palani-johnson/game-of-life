/* File:     serial-game.c
 *
 * Compile:  gcc -g -Wall -o game serial-game.c
 * Run:      ./game [input_file] [board_width] [board_height] [start_x] [start_y] [iterations]
 *
 * Compile Run and Process:
 *      gcc -g -o game serial-game.c && ./game rand 3840 2160 100 30 60 | mpv --no-correct-pts --fps=1 -
 * 
 */

#include "game.h"


int main(int argc, char** argv) {
    if (argc != 7) {
        // fprintf(
        //     stderr, 
        //     "usage:  %s [input_file] [board_width] [board_height] [start_x] [start_y] [iterations]\n", 
        //     argv[0]
        // );
        fprintf(
            stderr, 
            "usage:  %s [random|rand|r] [board_width] [board_height] [seed] [fill] [iterations]\n", 
            argv[0]
        );
        exit(EXIT_FAILURE);
    };

    struct GameOfLife life, *life_pointer;

    init_life(
        &life,
        argv[1], 
        strtol(argv[2], NULL, 10), 
        strtol(argv[3], NULL, 10),
        strtol(argv[4], NULL, 10),
        strtol(argv[5], NULL, 10)
    );
    
    ppm_write(&life, stdout);
    for (int i = 0; i < strtol(argv[6], NULL, 10); i++) {
        gen_next_buff(&life);
        iterate_buff(&life);
        ppm_write(&life, stdout);
    }

    free_buffs(&life);

    return EXIT_SUCCESS;
}

// UTILITY
char *get_filename_ext(char *filename) {
    char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) {
        fprintf(stderr, "Invalid file type. File must be .rle or .cells\n");
        exit(EXIT_FAILURE);
    };
    return dot + 1;
}

// LIFE FUNCTIONS
void init_life(
    struct GameOfLife *life,
    char *init_type, 
    int width, 
    int height, 
    int op1, 
    int op2
) {
    life->width = width;
    life->height = height;
    life->buff_size = (life->width + 2) * (life->height + 2);

    life->next_buff = malloc(life->buff_size * sizeof(bool));
    life->buff = strcmp(init_type, "random") 
            || strcmp(init_type, "random") 
            || strcmp(init_type, "r")
        ? alloc_random_buff(life, op1, op2)
        : alloc_buff_from_file(life, init_type, op1, op2);
    make_torus(life);
}

void make_torus(struct GameOfLife *life) {
    use_game_pos(life);

    int wm1 = life->width-1;
    int hm1 = life->height-1;
    int bm1 = life->buff_size-1;

    for (int j = 0; j < life->height; j++) {
        life->buff[game_pos(life->width, j)] = life->buff[game_pos(0, j)];
        life->buff[game_pos(-1, j)] = life->buff[game_pos(wm1, j)];
    }

    for (int i = 0; i < life->width; i++) {
        life->buff[game_pos(i, life->height)] = life->buff[game_pos(i, 0)];
        life->buff[game_pos(i, -1)] = life->buff[game_pos(i, hm1)];
    }

    life->buff[bm1] = life->buff[game_pos(0, 0)];
    life->buff[0] = life->buff[game_pos(wm1, hm1)];
    life->buff[life->width + 1] = life->buff[game_pos(0, hm1)];
    life->buff[bm1 - (life->width + 1)] = life->buff[game_pos(wm1, 0)];
}

bool *alloc_random_buff(struct GameOfLife *life, int seed, int fill) {
    use_game_pos(life);
    srand(seed);

    bool *buff = malloc(life->buff_size * sizeof(bool));
    for (int i = 0; i < life->width; i++) 
        for (int j = 0; j < life->height; j++)
            buff[game_pos(i, j)] = (rand() / (double)RAND_MAX) 
                < (double)fill / 100;
    
    return buff;
}

bool *alloc_buff_from_file(struct GameOfLife *life, char *file, int seed, int fill) {
    bool *buff = calloc(life->buff_size, sizeof(bool));
    return buff;
}


void ppm_write(struct GameOfLife *life, FILE *f) {
    use_game_pos(life);

    fprintf(f, "P6\n%d %d 255\n", life->width, life->height);
    for (int w = 0; w < life->width; w++) {
        for (int h = 0; h < life->height; h++) {
            int b = life->buff[game_pos(w, h)] ? 0 : 200;
            fprintf(f, "%c%c%c", b, b, b);
        }
    }
    
    fflush(f);
}


void gen_next_buff(struct GameOfLife *life) {
    use_game_pos(life);
    int i, j, w, h, sum, p;

    for (w = 0; w < life->width; w++) {
        for (h = 0; h < life->height; h++) {
            sum = 0;
            for (i = -1; i <= 1; i++)
                for (j = -1; j <= 1; j++) 
                    if (life->buff[game_pos(w+i, h+j)]) sum++;

            p = game_pos(w, h);
            life->next_buff[p] = sum == 3 || (life->buff[p] && sum == 4);
        }
    }
}

void iterate_buff(struct GameOfLife *life) {
    bool *temp_buff = life->buff;
    life->buff = life->next_buff;
    life->next_buff = temp_buff;

    make_torus(life);
}   

void free_buffs(struct GameOfLife *life) {
    free(life->buff);
    free(life->next_buff);
}
