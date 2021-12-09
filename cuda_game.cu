
/* File:     cuda_game.cu
 *
 * Compile:  nvcc cuda_game.cu game.c -o cuda_game
 * Run:      ./cuda_game [random|rand] [board_width] [board_height] [seed] [fill] [iterations]
 * 
 * Examples: 
 *      Gen 1000x1000 game board with seed 50 and fill 50 and save as game.ppm:
 *           ./cuda_game rand 1000 1000 10 50 600 > game.ppm
 *     
 *      View ppm:
 *           mpv --no-correct-pts --fps=60 game.ppm
 *      
 *      Stream game with pipe into mpv:
 *          ./cuda_game rand 1000 1000 10 50 600 | mpv --no-correct-pts --fps=10 -
 */
extern "C" {
    #include "game_of_life.h"
}

#define cuda_pos(i, j) int i = blockIdx.x * blockDim.x + threadIdx.x;\
    int j = blockIdx.y * blockDim.y + threadIdx.y

#define error_if(check, msg, val) if (check) { fprintf(stderr, msg, val); exit(EXIT_FAILURE); } 

// this number is implementation specific. change accordingly
#define CUDA_NUM 16

// Uses cuda to compute a life buffer.
__global__ void cuda_gen_next_buff(struct GameOfLife *life) {
    cuda_pos(i, j);

    int sum = 0;
    for (int h = j-1; h <= j+1; h++)
        for (int w = i-1; w <= i+1; w++) 
            if (life->buff[game_pos(life, w, h)]) sum++;
    
    int p = game_pos(life, i, j);
    life->next_buff[p] = sum == 3 || (life->buff[p] && sum == 4);
}

// Uses cuda to fill a video buffer.
__global__ void cuda_write_video_buffer(struct GameOfLife *life) {
    cuda_pos(i, j);

    int jh = j * (life->height) * 3;
    int i3 = i*3;

    char b = life->buff[game_pos(life, i, j)] ? 0 : 255;
    for(int c = 0; c < 3; c++) life->vid_buff[i3 + jh + c] = b;
}

struct GameOfLife *cuda_init_life(struct GameOfLife *life, struct GameOfLife *cuda_life_h) {
    memcpy(cuda_life_h, life, sizeof(GameOfLife));

    // allocate the buffers
    cudaMalloc(&(cuda_life_h->buff), life->buff_size * sizeof(bool));
    cudaMalloc(&(cuda_life_h->next_buff), life->buff_size * sizeof(bool));
    cudaMalloc(&(cuda_life_h->vid_buff), life->vid_buff_size * sizeof(char));

    // copy buffer
    cudaMemcpy(cuda_life_h->buff, life->buff, life->buff_size, cudaMemcpyHostToDevice);

    // make the data struct
    struct GameOfLife *cuda_life_d;
    cudaMalloc(&cuda_life_d, sizeof(GameOfLife)); 
    cudaMemcpy(cuda_life_d, cuda_life_h, sizeof(GameOfLife), cudaMemcpyHostToDevice);

    return cuda_life_d;
}

void ppm_write_from_cuda(struct GameOfLife *life, struct GameOfLife *cuda_life, FILE *f) {
    cudaMemcpy(
        life->vid_buff, 
        cuda_life->vid_buff, 
        life->vid_buff_size * sizeof(char), 
        cudaMemcpyDeviceToHost
    );

    fprintf(f, "P6\n%d %d 255\n", life->width, life->height);
    fwrite(life->vid_buff, sizeof(char), life->vid_buff_size, f);
    fflush(f);
}

int main(int argc, char** argv) {
    get_env;

    if (argc != 7) {
        fprintf(
            stderr, 
            "usage:  %s [random|rand] [board_width] [board_height] [seed] [fill] [iterations]\n", 
            argv[0]
        );
        exit(EXIT_FAILURE);
    };

    struct GameOfLife life_struct, *life, *cuda_life_d, cuda_life_h_struct, *cuda_life_h;
    life = &life_struct;
    cuda_life_h = &cuda_life_h_struct;

    int iterations = strtol(argv[6], NULL, 10);
    int width = strtol(argv[2], NULL, 10);
    int height = strtol(argv[3], NULL, 10);

    error_if(width < CUDA_NUM, "Width must be >= %d\n", CUDA_NUM)
    error_if(height < CUDA_NUM, "Height must be >= %d\n", CUDA_NUM)
    error_if(width % CUDA_NUM != 0, "Width mod %d must be 0\n", CUDA_NUM)
    error_if(height % CUDA_NUM != 0, "Height mod %d must be 0\n", CUDA_NUM)
    
    dim3 threads(CUDA_NUM, CUDA_NUM);
    dim3 blocks(width/CUDA_NUM, height/CUDA_NUM);

    init_life(
        life,
        argv[1], 
        width, 
        height,
        strtol(argv[4], NULL, 10),
        strtol(argv[5], NULL, 10)
    );

    cuda_life_d = cuda_init_life(life, cuda_life_h);

    cuda_write_video_buffer<<<blocks, threads>>>(cuda_life_d);
    if (DO_IO) ppm_write_from_cuda(life, cuda_life_h, stdout);

    for (int i = 0; i < iterations; i++) {
        cuda_gen_next_buff<<<blocks, threads>>>(cuda_life_d);
        iterate_buff(cuda_life_h);
        cudaMemcpy(cuda_life_d, cuda_life_h, sizeof(GameOfLife), cudaMemcpyHostToDevice);

        cuda_write_video_buffer<<<blocks, threads>>>(cuda_life_d); 
        if (DO_IO) ppm_write_from_cuda(life, cuda_life_h, stdout);
    }

    return EXIT_SUCCESS;
}