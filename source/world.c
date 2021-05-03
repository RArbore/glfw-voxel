#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../include/texture.h"
#include "../include/constants.h"
#include "../include/world.h"

const int block_mesh_faces[3][6] = {
    {1, 1, 0, 2, 1, 1},
    {3, 3, 3, 3, 3, 3},
    {4, 4, 4, 4, 4, 4},
};

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
    free(deleted);
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

void set_vert_base_coords(int start_vertex, int index, float *vertices, int x, int y, int z) {
    int row = index / TEX_WIDTH;
    int col = index % TEX_HEIGHT;

    int v;
    for (v = start_vertex; v < start_vertex + 32; v += 8) {
        vertices[v] = (float) x;
        vertices[v + 1] = (float) y;
        vertices[z + 2] = (float) z;

        vertices[v + 3] = 1.0f;
        vertices[v + 4] = 1.0f;
        vertices[z + 5] = 1.0f;

        vertices[v + 6] = (v == 0 || v == 1) ? (ATLAS_TEX_W - 1 - col) * TEX_SIZE : (ATLAS_TEX_W - col) * TEX_SIZE;
        vertices[v + 7] = (v == 1 || v == 2) ? (row + 1) * TEX_SIZE : row * TEX_SIZE;
    }
}

const int neighbor_offsets[][3] = {
    {1, 0, 0},
    {-1, 0, 0},
    {0, 1, 0},
    {0, -1, 0},
    {0, 0, 1},
    {0, 0, -1},
};

const int face_relative_offsets[][12] = {
    {1.0, 1.0, 1.0, 1.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0},
    {0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0, 1.0},
    {1.0, 1.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0},
    {0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 1.0},
    {0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0, 1.0, 1.0},
    {1.0, 1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0},
};

const float face_shadows[] = {
    0.7, 0.7, 1.0, 0.4, 0.7, 0.7
};

float* world_full_mesh_assemble(int *size) {
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
                    block_t *to_render = &chunk->blocks[r_x][r_y][r_z];
                    int neighbor_num;
                    for (neighbor_num = 0; neighbor_num < 6; neighbor_num++) {
                        block_t *neighbor = world_get_block_c(r_x + neighbor_offsets[neighbor_num][0], r_y + neighbor_offsets[neighbor_num][1], r_z + neighbor_offsets[neighbor_num][2], chunk);
                        if (neighbor == NULL || neighbor->id == 0) {
                            if (current_size + 32 >= vertices_max_size) {
                                vertices_max_size *= 2;
                                vertices = (float *) realloc(vertices, vertices_max_size * sizeof(float));
                            }
                            set_vert_base_coords(current_size, block_mesh_faces[to_render->id - 1][1], vertices, s_x + r_x, s_y + r_y, s_z + r_z);
                            int coord, vertices_i = 0;
                            for (coord = 0; coord < 12; coord++) {
                                vertices[current_size + vertices_i] += face_relative_offsets[neighbor_num][coord];
                                vertices_i += (vertices_i % 8 < 2) ? 1 : 6;
                            }
                            int old_size = current_size;
                            for (; current_size < old_size + 32; current_size += 8) {
                                vertices[current_size + 3] = face_shadows[neighbor_num];
                                vertices[current_size + 4] = face_shadows[neighbor_num];
                                vertices[current_size + 5] = face_shadows[neighbor_num];
                            }
                        }
                    }
                }
            }
        }
    }

    *size = current_size;
    return vertices;
}

chunk_t* generate_chunk(chunk_pos_t chunk_pos) {
    chunk_t *chunk = (chunk_t *) malloc(sizeof(chunk_t));
    chunk->chunk_pos = chunk_pos;
    int x, y, z;
    for (x = 0; x < CHUNK_SIZE; x++) {
        for (y = 0; y < CHUNK_SIZE; y++) {
            for (z = 0; z < CHUNK_SIZE; z++) {
                chunk->blocks[x][y][z] = (block_t) {1};
            }
        }
    }
    chunk->blocks[0][0][0] = (block_t) {1};
    return chunk;
}
