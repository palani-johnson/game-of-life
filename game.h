#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

struct GameOfLife {
    int width;
    int height;
    int buff_size;
    bool *buff;
    bool *next_buff;
};

#define use_game_pos(l) int _w2 = l->width+2; int _w3 = l->width+3
#define game_pos(i, j) (j*_w2 + _w3 + i)

void init_life(
    struct GameOfLife *life,
    char *init_type, 
    int width, 
    int height, 
    int op1, 
    int op2
);

bool *alloc_random_buff(struct GameOfLife *life, int seed, int fill);
bool *alloc_buff_from_file(struct GameOfLife *life, char *file, int seed, int fill);
void ppm_write(struct GameOfLife *life, FILE *f);
void gen_next_buff(struct GameOfLife *life);
void iterate_buff(struct GameOfLife *life);
void free_buffs(struct GameOfLife *life);
void make_torus(struct GameOfLife *life);