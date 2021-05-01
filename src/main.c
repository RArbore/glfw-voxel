#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>

#include <GL/glut.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <cglm/call.h>
#include <cglm/struct.h>

#include "../include/shaders/shaders.h"
#include "../include/texture.h"
#include "../include/world.h"

#define TEX_SIZE 16.0f/256.0f
#define PI 3.14159265358979323846

#define ANGLE_SPEED 0.02
#define MOVE_SPEED 0.02
#define MIN_PHI 0.0001

float vertices[] = {
     0.0f,  0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 15.0f*TEX_SIZE, 0.0f,
     0.0f, -0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 15.0f*TEX_SIZE, TEX_SIZE,
     0.0f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 16.0f*TEX_SIZE, TEX_SIZE,
     0.0f, 0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 16.0f*TEX_SIZE, 0.0f,
};

GLuint elements[] = {
    0, 1, 2, 3,
};

int width, height;
long counter = 0;

GLuint vao, vbo, ebo, tex_atlas, trans_attrib, vertex_shader, fragment_shader, shader_program;
GLint pos_attrib, col_attrib, tex_attrib;

mat4 view_mat, proj_mat;

float x, y, z, theta, phi = PI/2;

void set_tex_coords(int start_vertex, int index) {
    int row = index / texture_width;
    int col = index % texture_width;

    int v;
    for (v = start_vertex; v < start_vertex + 4; v++) {
        vertices[8 * v + 6] = (v == 0 || v == 1) ? (15 - col) * TEX_SIZE : (16 - col) * TEX_SIZE;
        vertices[8 * v + 7] = (v == 1 || v == 2) ? (row + 1) * TEX_SIZE : row * TEX_SIZE;
    }
}

void error_callback(int error, const char* description) {
    fprintf(stderr, "Error #%d: %s\n", error, description);
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
        glfwSwapInterval(1); //vsync: 1, no vsync: 0
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

    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);

    glLinkProgram(shader_program);
    glUseProgram(shader_program);

    pos_attrib = glGetAttribLocation(shader_program, "position");
    glEnableVertexAttribArray(pos_attrib);
    glVertexAttribPointer(pos_attrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);

    col_attrib = glGetAttribLocation(shader_program, "color");
    glEnableVertexAttribArray(col_attrib);
    glVertexAttribPointer(col_attrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (3 * sizeof(float)));

    tex_attrib = glGetAttribLocation(shader_program, "texcoord");
    glEnableVertexAttribArray(tex_attrib);
    glVertexAttribPointer(tex_attrib, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (6 * sizeof(float)));
}

void init_camera() {
    glm_perspective(1.2, ((float) width) / ((float) height), 0.1, 100.0, proj_mat);
}

void render(GLFWwindow *window) {
    if (counter % 10 == 0) {
        set_tex_coords(0, (counter / 10) % 5);
    }

    mat4 world_mat = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
    };

    //glm_rotate(world_mat, ((float) counter) / 100.0f, (vec3){0.0, 1.0, 0.0});
    glm_translate(world_mat, (vec3){3.0, 0.0, 0.0});

    glm_look((vec3){x, y, z}, (vec3){cos(theta)*sin(phi), cos(phi), sin(theta)*sin(phi)}, (vec3){0.0, 1.0, 0.0}, view_mat);

    trans_attrib = glGetUniformLocation(shader_program, "world_mat");
    glUniformMatrix4fv(trans_attrib, 1, GL_FALSE, (float * ) world_mat);
    trans_attrib = glGetUniformLocation(shader_program, "view_mat");
    glUniformMatrix4fv(trans_attrib, 1, GL_FALSE, (float * ) view_mat);
    trans_attrib = glGetUniformLocation(shader_program, "proj_mat");
    glUniformMatrix4fv(trans_attrib, 1, GL_FALSE, (float * ) proj_mat);

    glClear(GL_COLOR_BUFFER_BIT);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STREAM_DRAW);

    glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, 0);

    glfwSwapBuffers(window);
}

void handle_input(GLFWwindow *window) {
    glfwPollEvents();
    if (glfwGetKey(window, GLFW_KEY_W)) {
        x += MOVE_SPEED * cos(theta);
        z += MOVE_SPEED * sin(theta);
    }
    if (glfwGetKey(window, GLFW_KEY_A)) {
        x += MOVE_SPEED * sin(theta);
        z -= MOVE_SPEED * cos(theta);
    }
    if (glfwGetKey(window, GLFW_KEY_S)) {
        x -= MOVE_SPEED * cos(theta);
        z -= MOVE_SPEED * sin(theta);
    }
    if (glfwGetKey(window, GLFW_KEY_D)) {
        x -= MOVE_SPEED * sin(theta);
        z += MOVE_SPEED * cos(theta);
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE)) {
        y += MOVE_SPEED;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
        y -= MOVE_SPEED;
    }
    if (glfwGetKey(window, GLFW_KEY_K)) phi -= ANGLE_SPEED;
    if (glfwGetKey(window, GLFW_KEY_H)) theta -= ANGLE_SPEED;
    if (glfwGetKey(window, GLFW_KEY_J)) phi += ANGLE_SPEED;
    if (glfwGetKey(window, GLFW_KEY_L)) theta += ANGLE_SPEED;
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
    printf("FPS: %f   X: %f   Y: %f   Z: %f   THETA: %f   PHI: %f\n",1.0/secs, x, y, z, theta, phi);
}

int main(int argc, char** argv) {
    if (!glfwInit()) {
        return 1;
    }
    glfwSetErrorCallback(error_callback);

    GLFWwindow *window = initialize_window();

    glfwGetFramebufferSize(window, &width, &height);

    glViewport(0, 0, width, height);

    glfwSetFramebufferSizeCallback(window, resize_callback);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    glGenTextures(1, &tex_atlas);
    glBindTexture(GL_TEXTURE_2D, tex_atlas);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_width, texture_height, 0, GL_RGB, GL_FLOAT, texture);

    load_shaders();
    init_camera();
    initialize_world_hash();

    while (!glfwWindowShouldClose(window) && !glfwGetKey(window, GLFW_KEY_ESCAPE)) {
        tick(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}
