#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <GL/glut.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <cglm/call.h>
#include <cglm/struct.h>

#include "../include/shaders/shaders.h"
#include "../include/texture.h"
#include "../include/constants.h"
#include "../include/world.h"

float *world_vertices[HASH_TABLE_SIZE];
int world_vertices_size[HASH_TABLE_SIZE];

int width, height;
long counter = 0;

GLuint vao, vbo, tex_atlas, vertex_shader, fragment_shader, shader_program;
GLint pos_attrib, col_attrib, tex_attrib;

mat4 view_mat, proj_mat;

float x, y, z, theta, phi = PI/2, dt = 0;

void error_callback(int error, const char* description) {
    fprintf(stderr, "Error #%d: %s\n", error, description);
}

void init_camera() {
    glm_perspective(1.2, ((float) width) / ((float) height), 0.1, 1000.0, proj_mat);
}

void resize_callback(GLFWwindow* window, int iwidth, int iheight) {
    width = iwidth;
    height = iheight;
    glViewport(0, 0, width, height);
    init_camera();
    printf("%d %d\n", width, height);
}

GLFWwindow* initialize_window() {
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    GLFWwindow *window = glfwCreateWindow(800, 800, "Voxel Engine", NULL, NULL);
    if (window) {
        glfwShowWindow(window);
        glfwMakeContextCurrent(window);
        gladLoadGL();
        glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    }
    return window;
}

void load_shaders() {
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_source, NULL);
    glCompileShader(vertex_shader);

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_source, NULL);
    glCompileShader(fragment_shader);

    GLint isCompiled = 0;
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &maxLength);

        char *errorLog = malloc(maxLength * sizeof(char));
        glGetShaderInfoLog(fragment_shader, maxLength, &maxLength, &errorLog[0]);
        printf("%s\n", errorLog);

    }

    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);

    glLinkProgram(shader_program);
    glUseProgram(shader_program);

    pos_attrib = glGetAttribLocation(shader_program, "position");
    glEnableVertexAttribArray(pos_attrib);
    glVertexAttribPointer(pos_attrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);

    col_attrib = glGetAttribLocation(shader_program, "normal");
    glEnableVertexAttribArray(col_attrib);
    glVertexAttribPointer(col_attrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (3 * sizeof(float)));

    tex_attrib = glGetAttribLocation(shader_program, "texcoord");
    glEnableVertexAttribArray(tex_attrib);
    glVertexAttribPointer(tex_attrib, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (6 * sizeof(float)));
}

void render(GLFWwindow *window) {
    mat4 world_mat;
    glm_mat4_identity(world_mat);

    glm_look((vec3){x, y, z}, (vec3){cos(theta)*sin(phi), cos(phi), sin(theta)*sin(phi)}, (vec3){0.0, 1.0, 0.0}, view_mat);

    float dir_light[] = {0.0, -1.0, 0.0, 0.5, 0.5, 0.5, 0.3, 0.3, 0.3, 0.1, 0.1, 0.1};
    float point_light[] = {x, y, z, 0.0, 0.0, 0.0, 0.6, 0.6, 0.6, 0.2, 0.2, 0.2, 1.0, 0.0009, 0.000032};

    float camera_pos[] = {x, y, z};

    glUniformMatrix4fv(glGetUniformLocation(shader_program, "world_mat"), 1, GL_FALSE, (float *) world_mat);
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "view_mat"), 1, GL_FALSE, (float *) view_mat);
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "proj_mat"), 1, GL_FALSE, (float *) proj_mat);
    glUniform3fv(glGetUniformLocation(shader_program, "dir_light.direction"), 1, (float *) dir_light);
    glUniform3fv(glGetUniformLocation(shader_program, "dir_light.ambient_color"), 1, (float *) dir_light + 3);
    glUniform3fv(glGetUniformLocation(shader_program, "dir_light.diffuse_color"), 1, (float *) dir_light + 6);
    glUniform3fv(glGetUniformLocation(shader_program, "dir_light.specular_color"), 1, (float *) dir_light + 9);
    glUniform3fv(glGetUniformLocation(shader_program, "point_light[0].position"), 1, (float *) point_light);
    glUniform3fv(glGetUniformLocation(shader_program, "point_light[0].ambient_color"), 1, (float *) point_light + 3);
    glUniform3fv(glGetUniformLocation(shader_program, "point_light[0].diffuse_color"), 1, (float *) point_light + 6);
    glUniform3fv(glGetUniformLocation(shader_program, "point_light[0].specular_color"), 1, (float *) point_light + 9);
    glUniform3fv(glGetUniformLocation(shader_program, "point_light[0].attenuation"), 1, (float *) point_light + 12);
    glUniform3fv(glGetUniformLocation(shader_program, "camera_pos"), 1, (float *) camera_pos);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    int hash_index;
    for (hash_index = 0; hash_index < HASH_TABLE_SIZE; hash_index++) {
        if (world_vertices_size[hash_index] == 0) continue;
        glBufferData(GL_ARRAY_BUFFER, world_vertices_size[hash_index] * sizeof(float), world_vertices[hash_index], GL_STREAM_DRAW);
        glDrawArrays(GL_QUADS, 0, world_vertices_size[hash_index] / 8);
    }

    glfwSwapBuffers(window);
}

void handle_input(GLFWwindow *window) {
    glfwPollEvents();
    if (glfwGetKey(window, GLFW_KEY_W)) {
        x += MOVE_SPEED * cos(theta) * dt;
        z += MOVE_SPEED * sin(theta) * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_A)) {
        x += MOVE_SPEED * sin(theta) * dt;
        z -= MOVE_SPEED * cos(theta) * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_S)) {
        x -= MOVE_SPEED * cos(theta) * dt;
        z -= MOVE_SPEED * sin(theta) * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_D)) {
        x -= MOVE_SPEED * sin(theta) * dt;
        z += MOVE_SPEED * cos(theta) * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE)) {
        y += MOVE_SPEED * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
        y -= MOVE_SPEED * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_K)) phi -= ANGLE_SPEED * dt;
    if (glfwGetKey(window, GLFW_KEY_H)) theta -= ANGLE_SPEED * dt;
    if (glfwGetKey(window, GLFW_KEY_J)) phi += ANGLE_SPEED * dt;
    if (glfwGetKey(window, GLFW_KEY_L)) theta += ANGLE_SPEED * dt;
    if (phi < MIN_PHI) phi = MIN_PHI;
    if (phi > PI-MIN_PHI) phi = PI-MIN_PHI;
    while (theta < 0) theta += 2*PI;
    while (theta >= 2*PI) theta -= 2*PI;
}

void tick(GLFWwindow *window) {
    struct timeval start, stop;
    double secs = 0;
    gettimeofday(&start, NULL);

    render(window);
    handle_input(window);
    counter++;

    gettimeofday(&stop, NULL);
    secs = (double)(stop.tv_usec - start.tv_usec) / 1000000 + (double)(stop.tv_sec - start.tv_sec);
    dt = secs;
    printf("FPS: %f   X: %f   Y: %f   Z: %f   THETA: %f   PHI: %f\n", 1.0/secs, x, y, z, theta, phi);
}

int main(int argc, char** argv) {
    time_t t;
    srand((unsigned) time(&t));

    if (!glfwInit()) {
        return 1;
    }
    glfwSetErrorCallback(error_callback);

    GLFWwindow *window = initialize_window();

    glfwGetFramebufferSize(window, &width, &height);

    glViewport(0, 0, width, height);

    glfwSetFramebufferSizeCallback(window, resize_callback);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    glEnable(GL_CULL_FACE);

    // glEnable(GL_FRAMEBUFFER_SRGB);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glGenTextures(1, &tex_atlas);
    glBindTexture(GL_TEXTURE_2D, tex_atlas);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEX_WIDTH, TEX_HEIGHT, 0, GL_RGB, GL_FLOAT, texture);

    load_shaders();
    init_camera();
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    initialize_world();

    pthread_t world_manager;
    pthread_create(&world_manager, NULL, chunk_management, &((management_args_t) {world_vertices, world_vertices_size, &x, &y, &z}));
    //chunk_management(&world_vertices, &world_vertices_size);

    while (!glfwWindowShouldClose(window) && !glfwGetKey(window, GLFW_KEY_ESCAPE)) {
        tick(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}
