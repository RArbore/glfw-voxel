#ifndef __WORLD_H_
#define __WORLD_H_

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define CHUNK_SIZE 16
#define HASH_TABLE_SIZE 256
#define STARTING_VERTICES_NUM 16

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

chunk_t* hash_array[HASH_TABLE_SIZE];
chunk_t* deleted;

int hash_code(chunk_pos_t chunk_pos) {
    int hash = (chunk_pos.s_x + 293 * chunk_pos.s_y + 617 * chunk_pos.s_z) % HASH_TABLE_SIZE;
    return (hash < 0) ? hash + HASH_TABLE_SIZE : hash;
}

int chunk_pos_equal(chunk_pos_t a, chunk_pos_t b) {
    return a.s_x == b.s_x && a.s_y == b.s_y && a.s_z == b.s_z;
}

chunk_t* world_chunk_search(chunk_pos_t chunk_pos) {
    int hash_index = hash_code(chunk_pos);
    int original_index = hash_index;

    while (hash_array[hash_index] != NULL) {
        if (chunk_pos_equal(hash_array[hash_index]->chunk_pos, chunk_pos)) return hash_array[hash_index];
        hash_index++;
        hash_index %= HASH_TABLE_SIZE;
        if (hash_index == original_index) return NULL;
    }

    return NULL;
}

int world_chunk_insert(chunk_t *chunk) {
    int hash_index = hash_code(chunk->chunk_pos);
    int original_index = hash_index;

    while (hash_array[hash_index] != NULL && !chunk_pos_equal(hash_array[hash_index]->chunk_pos, deleted->chunk_pos)) {
        hash_index++;
        hash_index %= HASH_TABLE_SIZE;
        if (hash_index == original_index) return 0;
    }

    hash_array[hash_index] = chunk;
    return 1;
}

chunk_t* world_chunk_remove(chunk_pos_t chunk_pos) {
    int hash_index = hash_code(chunk_pos);
    int original_index = hash_index;

    while (hash_array[hash_index] != NULL) {
        if (chunk_pos_equal(hash_array[hash_index]->chunk_pos, chunk_pos)) {
            chunk_t* ret = hash_array[hash_index];
            hash_array[hash_index] = deleted;
            return ret;
        }
        hash_index++;
        hash_index %= HASH_TABLE_SIZE;
        if (hash_index == original_index) return NULL;
    }

    return NULL;
}

chunk_t* world_chunk_remove_c(chunk_t *chunk) {
    return world_chunk_remove(chunk->chunk_pos);
}

void initialize_world_hash() {
    deleted = (chunk_t*) malloc(sizeof(chunk_t));
    deleted->chunk_pos = (chunk_pos_t){INT_MIN, INT_MIN, INT_MIN};
}

block_t* world_get_block(int x, int y, int z) {
    int r_x = x % CHUNK_SIZE;
    int r_y = y % CHUNK_SIZE;
    int r_z = z % CHUNK_SIZE;
    while (r_x < 0) r_x += CHUNK_SIZE;
    while (r_y < 0) r_y += CHUNK_SIZE;
    while (r_z < 0) r_z += CHUNK_SIZE;
    chunk_pos_t chunk_pos = (chunk_pos_t){x - r_x, y - r_y, z - r_z};
    chunk_t* chunk = world_chunk_search(chunk_pos);
    if (chunk == NULL) return NULL;
    return &chunk->blocks[r_x][r_y][r_z];
}

block_t* world_get_block_c(int r_x, int r_y, int r_z, chunk_t *chunk) {
    if (r_x < 0 || r_x >= CHUNK_SIZE || r_y < 0 || r_y >= CHUNK_SIZE || r_z < 0 || r_z >= CHUNK_SIZE) return &chunk->blocks[r_x][r_y][r_z];
    chunk_pos_t chunk_pos = chunk->chunk_pos;
    while (r_x < 0) {
        r_x += CHUNK_SIZE;
        chunk_pos.s_x -= CHUNK_SIZE;
    }
    while (r_y < 0) {
        r_y += CHUNK_SIZE;
        chunk_pos.s_y -= CHUNK_SIZE;
    }
    while (r_z < 0) {
        r_z += CHUNK_SIZE;
        chunk_pos.s_z -= CHUNK_SIZE;
    }
    while (r_x >= CHUNK_SIZE) {
        r_x -= CHUNK_SIZE;
        chunk_pos.s_x += CHUNK_SIZE;
    }
    while (r_y >= CHUNK_SIZE) {
        r_y -= CHUNK_SIZE;
        chunk_pos.s_y += CHUNK_SIZE;
    }
    while (r_z >= CHUNK_SIZE) {
        r_z -= CHUNK_SIZE;
        chunk_pos.s_z += CHUNK_SIZE;
    }
    chunk = world_chunk_search(chunk_pos);
    return (chunk == NULL) ? NULL : &chunk->blocks[r_x][r_y][r_z];
}

float* world_mesh_assemble(int *size) {
    int hash_index, current_size = 0;
    int vertices_max_size = 8 * STARTING_VERTICES_NUM;
    float *vertices = (float *) malloc(vertices_max_size * sizeof(float));

    for (hash_index = 0; hash_index < HASH_TABLE_SIZE; hash_index++) {
        if (hash_array[hash_index] == NULL || chunk_pos_equal(hash_array[hash_index]->chunk_pos, deleted->chunk_pos)) continue;
        chunk_t *chunk = hash_array[hash_index];
        int r_x, r_y, r_z, s_x, s_y, s_z;
        chunk_pos_t *chunk_pos = &chunk->chunk_pos;
        s_x = chunk_pos->s_x;
        s_y = chunk_pos->s_y;
        s_z = chunk_pos->s_z;
        for (r_x = 0; r_x < CHUNK_SIZE; r_x++) {
            for (r_y = 0; r_y < CHUNK_SIZE; r_y++) {
                for (r_z = 0; r_z < CHUNK_SIZE; r_z++) {
                    if (chunk->blocks[r_x][r_y][r_z].id == 0) continue;

                }
            }
        }
    }

    *size = current_size;
    return vertices;
}

#endif // __WORLD_H_
