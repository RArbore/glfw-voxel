#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>

#include <GL/glut.h>
#include <GLFW/glfw3.h>

#include "../include/shaders/shaders.h"
#include "../include/texture.h"

#define TEX_SIZE 16.0f/256.0f

float vertices[] = {
     0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 15.0f*TEX_SIZE, 0.0f,
     0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 15.0f*TEX_SIZE, TEX_SIZE,
     -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 16.0f*TEX_SIZE, TEX_SIZE,
     -0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 16.0f*TEX_SIZE, 0.0f,
};

GLuint elements[] = {
    0, 1, 2, 3,
};

long counter = 0;

GLuint vao, vbo, ebo, tex_atlas, vertex_shader, fragment_shader, shader_program;
GLint pos_attrib, col_attrib, tex_attrib;

void set_tex_coords(int start_vertex, int index) {
    int row = index / texture_width;
    int col = index % texture_width;

    int v;
    for (v = start_vertex; v < start_vertex + 4; v++) {
        vertices[7 * v + 5] = (v == 0 || v == 1) ? (15 - col) * TEX_SIZE : (16 - col) * TEX_SIZE;
        vertices[7 * v + 6] = (v == 1 || v == 2) ? (row + 1) * TEX_SIZE : row * TEX_SIZE;
    }
}

void error_callback(int error, const char* description) {
    fprintf(stderr, "Error #%d: %s\n", error, description);
}

void resize_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
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

void render(GLFWwindow *window) {
    if (counter % 10 == 0) {
        set_tex_coords(0, (counter / 10) % 5);
    }

    glClear(GL_COLOR_BUFFER_BIT);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STREAM_DRAW);

    glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, 0);

    glfwSwapBuffers(window);
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
    glVertexAttribPointer(pos_attrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), 0);

    col_attrib = glGetAttribLocation(shader_program, "color");
    glEnableVertexAttribArray(col_attrib);
    glVertexAttribPointer(col_attrib, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void *) (2 * sizeof(float)));

    tex_attrib = glGetAttribLocation(shader_program, "texcoord");
    glEnableVertexAttribArray(tex_attrib);
    glVertexAttribPointer(tex_attrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void *) (5 * sizeof(float)));
}

void handle_input() {
    glfwPollEvents();
}

void tick(GLFWwindow *window) {
    struct timeval start, stop;
    double secs = 0;
    gettimeofday(&start, NULL);

    render(window);
    handle_input();
    counter++;

    gettimeofday(&stop, NULL);
    secs = (double)(stop.tv_usec - start.tv_usec) / 1000000 + (double)(stop.tv_sec - start.tv_sec);
    printf("FPS: %f\n",1.0/secs);
}

int main(int argc, char** argv) {
    if (!glfwInit()) {
        return 1;
    }
    glfwSetErrorCallback(error_callback);

    GLFWwindow *window = initialize_window();

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    glShadeModel(GL_SMOOTH);
    glClearColor(0, 0, 0, 0);
    glClearDepth(1);
    glDepthFunc(GL_LEQUAL);
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45,1,0.1,100);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

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

    while (!glfwWindowShouldClose(window) && !glfwGetKey(window, GLFW_KEY_ESCAPE)) {
        tick(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}
