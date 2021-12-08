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
void ppm_write(struct GameOfLife *life, FILE *f, char *vid_buff) {
    fprintf(f, "P6\n%d %d 255\n", life->width, life->height);
    fwrite(vid_buff, sizeof(char), life->width * life->height * 3, f);
    fflush(f);
}

char * alloc_video_buff(struct GameOfLife *life) {
    return malloc(life->width * life->height * sizeof(char) * 3);
}

// 

// LIFE FUNCTIONS

// Fills a life struct with useful data
void init_life(struct GameOfLife *life, char *init_type, int width, int height, int op1, int op2) {
    life->width = width;
    life->height = height;
    life->buff_size = (life->width + 2) * (life->height + 2);

    life->next_buff = malloc(life->buff_size * sizeof(bool));
    life->buff = strcmp(init_type, "random") || strcmp(init_type, "rand")
        ? alloc_random_buff(life, op1, op2)
        : alloc_buff_from_file(life, init_type, op1, op2);
}

// Allocates random data to a life struct buffer
bool *alloc_random_buff(struct GameOfLife *life, int seed, int fill) {
    use_game_pos(life);
    bool *buff;

    if (seed == -1) {
        buff = calloc(life->buff_size, sizeof(bool));
        for (int i = 0; i < 3; i++) 
            buff[game_pos(i, 1)] = 1;
    } else {
        srand(seed);
        buff = malloc(life->buff_size * sizeof(bool));
        for (int j = 0; j < life->height; j++) 
            for (int i = 0; i < life->width; i++)
                buff[game_pos(i, j)] = (rand() / (double)RAND_MAX) 
                    < (double)fill / 100;
    }

    return buff;
}

// TODO: Make this function
bool *alloc_buff_from_file(struct GameOfLife *life, char *file, int seed, int fill) {
    bool *buff = calloc(life->buff_size, sizeof(bool));
    return buff;
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
}
