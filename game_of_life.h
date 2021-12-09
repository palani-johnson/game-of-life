#ifndef GAME_OF_LIFE
    #define GAME_OF_LIFE

    #include <stdio.h>
    #include <string.h>
    #include <stdlib.h>
    #include <stdbool.h>
    #include <string.h>

    struct GameOfLife {
        // main values for game of life
        int width;
        int height;
        int buff_size;
        bool *buff;
        bool *next_buff;

        // values for the video buffer
        int vid_buff_size;
        char *vid_buff;

        // common values used in other functions
        int _w2;
        int _w3;
    };

    #define game_pos(l, i, j) ((j)*(l->_w2) + (l->_w3) + (i))

    void init_life(struct GameOfLife *life, char *init_type, int width, int height, int op1, int op2);
    void ppm_write(struct GameOfLife *life, FILE *f);
    void iterate_buff(struct GameOfLife *life);
    void free_buffs(struct GameOfLife *life);
    bool *alloc_random_buff(struct GameOfLife *life, int seed, int fill);
    bool *alloc_buff_from_file(struct GameOfLife *life, char *file, int seed, int fill);


    #define get_env bool DO_TORIS = getenv("GAME_DO_TORIS") != NULL; \
    bool DO_IO = getenv("GAME_DONT_DO_IO") == NULL;
#endif