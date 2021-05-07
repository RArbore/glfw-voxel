#include <math.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#include "../include/texture.h"
#include "../include/constants.h"
#include "../include/world.h"
#include "../include/open-simplex-noise.h"

#define FNL_IMPL

chunk_t* hash_array[HASH_TABLE_SIZE];
chunk_t* deleted;

struct osn_context *ctx;

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

void initialize_world() {
    free(deleted);
    deleted = (chunk_t*) malloc(sizeof(chunk_t));
    deleted->chunk_pos = (chunk_pos_t){INT_MIN, INT_MIN, INT_MIN};
    open_simplex_noise(77374, &ctx); 
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
    if (!(r_x < 0 || r_x >= CHUNK_SIZE || r_y < 0 || r_y >= CHUNK_SIZE || r_z < 0 || r_z >= CHUNK_SIZE)) return &chunk->blocks[r_x][r_y][r_z];
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
        vertices[v + 2] = (float) z;

        vertices[v + 3] = 0.0f;
        vertices[v + 4] = 0.0f;
        vertices[v + 5] = 0.0f;

        vertices[v + 6] = (v - start_vertex == 0 || v - start_vertex == 8) ? (ATLAS_TEX_W - 1 - col) * TEX_SIZE : (ATLAS_TEX_W - col) * TEX_SIZE;
        vertices[v + 7] = (v - start_vertex == 8 || v - start_vertex == 16) ? (row + 1) * TEX_SIZE : row * TEX_SIZE;
    }
}

const int block_mesh_faces[][6] = {
    {1, 1, 0, 2, 1, 1},
    {2, 2, 2, 2, 2, 2},
    {3, 3, 3, 3, 3, 3},
    {4, 4, 4, 4, 4, 4},
};

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
    {1.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0},
    {0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 1.0},
    {0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0, 1.0, 1.0},
    {1.0, 1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0},
};

const float face_normals[][3] = {
    {1, 0, 0},
    {-1, 0, 0},
    {0, 1, 0},
    {0, -1, 0},
    {0, 0, 1},
    {0, 0, -1},
};

float* world_full_mesh_assemble(int *size) {
    int hash_index, mesh_sizes[HASH_TABLE_SIZE], total_size = 0, set_size = 0;
    float *meshes[HASH_TABLE_SIZE];

    for (hash_index = 0; hash_index < HASH_TABLE_SIZE; hash_index++) {
        meshes[hash_index] = world_chunk_mesh_assemble(&mesh_sizes[hash_index], hash_array[hash_index]);
        total_size += mesh_sizes[hash_index];
    }

    float *vertices = (float *) malloc(total_size * sizeof(float));
    for (hash_index = 0; hash_index < HASH_TABLE_SIZE; hash_index++) {
        if (mesh_sizes[hash_index] == 0) continue;
        memcpy(vertices + set_size, meshes[hash_index], mesh_sizes[hash_index] * sizeof(float));
        set_size += mesh_sizes[hash_index];
        free(meshes[hash_index]);
    }

    *size = total_size;
    return vertices;
}

float* world_chunk_mesh_assemble(int *size, chunk_t *chunk) {
    *size = 0;
    if (chunk == NULL || chunk_pos_equal(chunk->chunk_pos, deleted->chunk_pos)) return NULL;
    int current_size = 0, vertices_max_size = 8 * STARTING_VERTICES_NUM;
    float *vertices = (float *) malloc(vertices_max_size * sizeof(float));

    chunk_pos_t *chunk_pos = &chunk->chunk_pos;
    int r_x, r_y, r_z, s_x, s_y, s_z;
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
                        set_vert_base_coords(current_size, block_mesh_faces[to_render->id - 1][neighbor_num], vertices, s_x + r_x, s_y + r_y, s_z + r_z);
                        int coord, vertices_i = 0;
                        for (coord = 0; coord < 12; coord++) {
                            vertices[current_size + vertices_i] += face_relative_offsets[neighbor_num][coord];
                            vertices_i += (vertices_i % 8 < 2) ? 1 : 6;
                        }
                        int old_size = current_size;
                        for (; current_size < old_size + 32; current_size += 8) {
                            vertices[current_size + 3] = face_normals[neighbor_num][0];
                            vertices[current_size + 4] = face_normals[neighbor_num][1];
                            vertices[current_size + 5] = face_normals[neighbor_num][2];
                        }
                    }
                }
            }
        }
    }

    *size = current_size;

    if (current_size == 0) {
        free(vertices);
        return NULL;
    }
    if (current_size != vertices_max_size) vertices = (float *) realloc(vertices, current_size * sizeof(float));

    return vertices;
}

chunk_t* generate_chunk(chunk_pos_t chunk_pos) {
    chunk_t *chunk = (chunk_t *) malloc(sizeof(chunk_t));
    chunk->chunk_pos = chunk_pos;
    chunk->to_update = 1;
    int r_x, r_y, r_z;
    for (r_x = 0; r_x < CHUNK_SIZE; r_x++) {
        for (r_z = 0; r_z < CHUNK_SIZE; r_z++) {
            double height = open_simplex_noise2(ctx, (float) (r_x + chunk_pos.s_x) / 16.0, (float) (r_z + chunk_pos.s_z) / 16.0)*8+8
                ;//+ open_simplex_noise2(ctx, (float) (r_x + chunk_pos.s_x + 64*247) / 32.0, (float) (r_z + chunk_pos.s_z - 64*1237) / 32.0)*16; 
            for (r_y = 0; r_y < CHUNK_SIZE; r_y++) {
                int to_create = 0;
                if (r_y + chunk_pos.s_y < height - 1) to_create = 2;
                else if (r_y + chunk_pos.s_y < height) to_create = 1;
                chunk->blocks[r_x][r_y][r_z] = (block_t) {to_create};
            }
        }
    }
    return chunk;
}

void mesh_thread_worker(void *argsv) {
    mesh_thread_args_t *targs = argsv;
    management_args_t *args = targs->management_args;
    int hash_index, start_index = targs->thread_id*HASH_TABLE_SIZE/NUM_WORKER_THREADS, chunk_size, neighbor_num;
    float *prev, *swap;
    for (hash_index = start_index; hash_index < start_index + HASH_TABLE_SIZE / NUM_WORKER_THREADS; hash_index++) {
        chunk_t *chunk = hash_array[hash_index];
        if (chunk == NULL || chunk_pos_equal(chunk->chunk_pos, deleted->chunk_pos)) continue;
        if (abs(chunk->chunk_pos.s_x / CHUNK_SIZE - targs->n_x) + abs(chunk->chunk_pos.s_y / CHUNK_SIZE - targs->n_y) + abs(chunk->chunk_pos.s_z / CHUNK_SIZE - targs->n_z) > UNLOAD_MANHATTAN_DIST) {
            world_chunk_remove_c(chunk);
            for (neighbor_num = 0; neighbor_num < 6; neighbor_num++) {
                chunk_t *neighbor = world_chunk_search((chunk_pos_t) {chunk->chunk_pos.s_x + neighbor_offsets[neighbor_num][0] * CHUNK_SIZE, chunk->chunk_pos.s_y + neighbor_offsets[neighbor_num][1] * CHUNK_SIZE, chunk->chunk_pos.s_z + neighbor_offsets[neighbor_num][2] * CHUNK_SIZE});
                if (neighbor == NULL) continue;
                neighbor->to_update = 1;
            }
            free(chunk);
            args->sizes[hash_index] = 0;
            free(args->meshes[hash_index]);
            args->meshes[hash_index] = NULL;
            continue;
        }
        if (chunk->to_update == 0) continue;
        prev = args->meshes[hash_index];
        swap = world_chunk_mesh_assemble(&chunk_size, chunk);
        args->sizes[hash_index] = (chunk_size < args->sizes[hash_index]) ? chunk_size : args->sizes[hash_index];
        args->meshes[hash_index] = swap;
        args->sizes[hash_index] = chunk_size;
        args->positions[hash_index] = chunk->chunk_pos;
        free(prev);
        chunk->to_update = 0;
    }
}

void chunk_management(void *argsv) {
    management_args_t *args = argsv;
    int r_x, r_y, r_z, n_x, n_y, n_z, p_x = INT_MIN, p_y = INT_MIN, p_z = INT_MIN, neighbor_num;
        
    for (;;) {
        n_x = round(*(args->x) / CHUNK_SIZE);
        n_y = round(*(args->y) / CHUNK_SIZE);
        n_z = round(*(args->z) / CHUNK_SIZE);
        
        if (n_x != p_x || n_y != p_y || n_z != p_z) {
            for (r_x = -RENDER_MANHATTAN_DIST; r_x <= RENDER_MANHATTAN_DIST; r_x++) {
                for (r_y = -RENDER_MANHATTAN_DIST + abs(r_x); r_y <= RENDER_MANHATTAN_DIST - abs(r_x); r_y++) {
                    for (r_z = -RENDER_MANHATTAN_DIST + abs(r_x) + abs(r_y); r_z <= RENDER_MANHATTAN_DIST - abs(r_x) - abs(r_y); r_z++) {
                        chunk_pos_t chunk_pos = (chunk_pos_t) {(r_x + n_x) * CHUNK_SIZE, (r_y + n_y) * CHUNK_SIZE, (r_z + n_z) * CHUNK_SIZE};
                        chunk_t *chunk = world_chunk_search(chunk_pos);
                        if (chunk != NULL) continue;
                        chunk = generate_chunk(chunk_pos);
                        world_chunk_insert(chunk);
                        for (neighbor_num = 0; neighbor_num < 6; neighbor_num++) {
                            chunk_t *neighbor = world_chunk_search((chunk_pos_t) {chunk_pos.s_x + neighbor_offsets[neighbor_num][0] * CHUNK_SIZE, chunk_pos.s_y + neighbor_offsets[neighbor_num][1] * CHUNK_SIZE, chunk_pos.s_z + neighbor_offsets[neighbor_num][2] * CHUNK_SIZE});
                            if (neighbor == NULL) continue;
                            neighbor->to_update = 1;
                        }
                    }
                }
            }

            int thread_id;
            pthread_t threads[NUM_WORKER_THREADS];
            mesh_thread_args_t *mesh_thread_args[NUM_WORKER_THREADS];
            for (thread_id = 0; thread_id < NUM_WORKER_THREADS; thread_id++) {
                mesh_thread_args[thread_id] = (mesh_thread_args_t *) malloc(sizeof(mesh_thread_args_t));
                mesh_thread_args[thread_id]->management_args = args;
                mesh_thread_args[thread_id]->thread_id = thread_id;
                mesh_thread_args[thread_id]->n_x = n_x;
                mesh_thread_args[thread_id]->n_y = n_y;
                mesh_thread_args[thread_id]->n_z = n_z;
                pthread_create(&threads[thread_id], NULL, mesh_thread_worker, mesh_thread_args[thread_id]);
            }
            for (thread_id = 0; thread_id < NUM_WORKER_THREADS; thread_id++) {
                pthread_join(threads[thread_id], NULL);
                free(mesh_thread_args[thread_id]);
            }

            p_x = n_x;
            p_y = n_y;
            p_z = n_z;
        }
    }
}
