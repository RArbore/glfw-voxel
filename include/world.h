#ifndef __WORLD_H_
#define __WORLD_H_

#include "../include/constants.h"

typedef struct block_s {
    int id;
} block_t;

typedef struct chunk_pos_s {
    int s_x, s_y, s_z;
} chunk_pos_t;

typedef struct chunk_s {
    chunk_pos_t chunk_pos;
    block_t blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
} chunk_t;

int hash_code(chunk_pos_t);

int chunk_pos_equal(chunk_pos_t, chunk_pos_t);

chunk_t* world_chunk_search(chunk_pos_t);

int world_chunk_insert(chunk_t *);

chunk_t* world_chunk_remove(chunk_pos_t);

chunk_t* world_chunk_remove_c(chunk_t *);

void initialize_world();

block_t* world_get_block(int, int, int);

block_t* world_get_block_c(int, int, int, chunk_t *);

void set_vert_base_coords(int, int, float *, int, int, int);

float* world_full_mesh_assemble(int *);

float* world_chunk_mesh_assemble(int *, chunk_t *);

chunk_t* generate_chunk(chunk_pos_t);

typedef struct management_args_s {
    float **mesh;
    int *size;
} management_args_t;

void chunk_management(void *);

#endif // __WORLD_H_
