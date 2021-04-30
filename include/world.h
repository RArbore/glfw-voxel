#ifndef __WORLD_H_
#define __WORLD_H_

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define CHUNK_SIZE 16
#define HASH_TABLE_SIZE 256

typedef struct block_s {
    int block_id;
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
    return (chunk_pos.s_x + 293 * chunk_pos.s_y + 617 * chunk_pos.s_z) % HASH_TABLE_SIZE;
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

int world_chunk_insert(chunk_t chunk) {
    int hash_index = hash_code(chunk.chunk_pos);
    int original_index = hash_index;

    while (hash_array[hash_index] != NULL && !chunk_pos_equal(hash_array[hash_index]->chunk_pos, (chunk_pos_t){INT_MIN, INT_MIN, INT_MIN})) {
        hash_index++;
        hash_index %= HASH_TABLE_SIZE;
        if (hash_index == original_index) return 0;
    }

    hash_array[hash_index] = &chunk;
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

chunk_t* world_chunk_remove_c(chunk_t chunk) {
    return world_chunk_remove(chunk.chunk_pos);
}

void initialize_world_hash() {
    deleted = (chunk_t*) malloc(sizeof(chunk_t));
    deleted->chunk_pos = (chunk_pos_t){INT_MIN, INT_MIN, INT_MIN};
}

#endif // __WORLD_H_
