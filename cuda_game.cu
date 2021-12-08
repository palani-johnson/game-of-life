
/* File:     cuda_game.cu
 *
 * Compile:  nvcc cuda_game.cu game.c -o cuda_game
 * Run:      ./game [threads] [random|rand] [board_width] [board_height] [seed] [fill] [iterations]
 * 
 * Examples: 
 *      Gen 1000x1000 game board with seed 50 and fill 50 and save as game.ppm:
 *           ./omp_game 16 rand 1000 1000 10 50 600 > game.ppm
 *     
 *      View ppm:
 *           mpv --no-correct-pts --fps=60 game.ppm
 *      
 *      Stream game with pipe into mpv:
 *          ./omp_game 16 rand 1000 1000 10 50 600 | mpv --no-correct-pts --fps=10 -
 */
#include "game_of_life.h"

__device__ int width;

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

// Uses cuda to compute a life buffer.
__global__ void cuda_gen_next_buff(struct GameOfLife *life) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    int j = blockIdx.y * blockDim.y + threadIdx.y;

    int sum = 0;
    for (int h = j-1; h <= j+1; h++)
        for (int w = i-1; w <= i+1; w++) 
            if (life->buff[game_pos(life, w, h)]) sum++;
    
    int p = game_pos(life, i, j);
    life->next_buff[p] = sum == 3 || (life->buff[p] && sum == 4);
}

// Uses cuda to fill a video buffer.
__global__ void cuda_write_video_buffer(struct GameOfLife *life) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    int j = blockIdx.y * blockDim.y + threadIdx.y;

    int i3;
    int jh;

    char b = life->buff[game_pos(life, i, j)] ? 0 : 255;
    for(int c = 0; c < 3; c++) life->vid_buff[i3 + jh + c] = b;
}

void cuda_init_life(
    struct GameOfLife *life,
    struct GameOfLife *cuda_life
) {
    // allocate the struct
    cudaMalloc((void **)&cuda_life, sizeof(GameOfLife));

    // copy data from struct (pointers will be bad but thats ok for now)
    cudaMemcpy(cuda_life, life, sizeof(GameOfLife), cudaMemcpyHostToDevice); 

    // allocate the buffers
    cudaMalloc((void **)&cuda_life->buff, life->buff_size);
    cudaMalloc((void **)&cuda_life->next_buff, life->buff_size);
    cudaMalloc((void **)&cuda_life->vid_buff, life->width * life->height * sizeof(char) * 3);

    // copy buffer
    cudaMemcpy(cuda_life->buff, life->buff, life->buff_size, cudaMemcpyHostToDevice); 
}

int main(int argc, char** argv) {
    if (argc != 7) {
        fprintf(
            stderr, 
            "usage:  %s [random|rand] [board_width] [board_height] [seed] [fill] [iterations]\n", 
            argv[0]
        );
        exit(EXIT_FAILURE);
    };

    struct GameOfLife life_struct, *life, *cuda_life;
    life = &life_struct;

    init_life(
        life,
        argv[1], 
        strtol(argv[2], NULL, 10), 
        strtol(argv[3], NULL, 10),
        strtol(argv[4], NULL, 10),
        strtol(argv[5], NULL, 10)
    );

    cuda_init_life(life, cuda_life);

    // // setup cuda
    // char *cuda_vid_buff;
    // bool *cuda_buff, *cuda_next_buff;

    // cuda_init_life(life, cuda_vid_buff, cuda_buff, cuda_vid_buff)

    // write_video_buffer(life, vid_buff);
    // ppm_write(life, stdout, vid_buff);

    // for (int i = 0; i < strtol(argv[6], NULL, 10); i++) {
    //     make_torus(life);
    //     gen_next_buff(life);
    //     iterate_buff(life);
    //     write_video_buffer(life, vid_buff);  
    //     ppm_write(life, stdout, vid_buff);
    // }

    // free_buffs(life);

    return EXIT_SUCCESS;
}