/* File:     game.c
 *
 * This file contains some of my utility functions 
 * that I use in some of my other files
 * 
 */
#include "game_of_life.h"


// UTILITY

// Gets the file extension
char *get_filename_ext(char *filename) {
    char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) {
        fprintf(stderr, "Invalid file type. File must be .rle or .cells\n");
        exit(EXIT_FAILURE);
    };
    return dot + 1;
}

// Writes the current buffer of a life struct to ppm format
void ppm_write(struct GameOfLife *life, FILE *f) {
    fprintf(f, "P6\n%d %d 255\n", life->width, life->height);
    fwrite(life->vid_buff, sizeof(char), life->vid_buff_size, f);
    fflush(f);
}


// LIFE FUNCTIONS

// Allocates random data to a life struct buffer
void fill_random_buff(struct GameOfLife *life, int seed, int fill) {
    if (seed == -1) {
        for (int i = 0; i < 3; i++) 
            life->buff[game_pos(life, i, 1)] = 1;
    } else {
        srand(seed);
        for (int j = 0; j < life->height; j++) 
            for (int i = 0; i < life->width; i++)
                life->buff[game_pos(life, i, j)] = (rand() / (double)RAND_MAX) 
                    < (double)fill / 100;
    }
}

// TODO: Make this function
void fill_buff_from_file(struct GameOfLife *life, char *file, int seed, int fill) {

}

// Fills a life struct with useful data
void init_life(
    struct GameOfLife *life, 
    char *init_type, 
    int width, 
    int height, 
    int op1, 
    int op2
) {
    life_calc_derived(life, width, height);
    life_alloc_buffs(life);
    
    if (strcmp(init_type, "random") || strcmp(init_type, "rand")) 
        fill_random_buff(life, op1, op2);
    else fill_buff_from_file(life, init_type, op1, op2);
}

void life_alloc_buffs(struct GameOfLife *life) {
    life->next_buff = malloc(life->buff_size * sizeof(bool));
    life->buff = calloc(life->buff_size, sizeof(bool));
    life->vid_buff = malloc(life->vid_buff_size * sizeof(char));
}

void life_calc_derived(struct GameOfLife *life, int width, int height) {
    life->width = width;
    life->height = height;
    life->buff_size = (life->width + 2) * (life->height + 2);
    life->_w2 = life->width + 2;
    life->_w3 = life->width + 3;
    life->vid_buff_size = life->width * life->height * 3;
}

// Make the next buffer the current buffer in a life struct
void iterate_buff(struct GameOfLife *life) {
    bool *temp_buff = life->buff;
    life->buff = life->next_buff;
    life->next_buff = temp_buff;
}   

// Free the buffers in a life struct
void free_buffs(struct GameOfLife *life) {
    free(life->buff);
    free(life->next_buff);
    free(life->vid_buff);
}
