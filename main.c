#include "stdio.h"
#include <unistd.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

void error_callback(int error, const char* description) {
    fprintf(stderr, "Error #%d: %s\n", error, description);
}

GLFWwindow* initialize_window() {
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Voxel Engine", NULL, NULL);
    if (window) {
        glfwShowWindow(window);
        glfwMakeContextCurrent(window);
    }
    return window;
}

int main(int argc, char** argv) {
    if (glfwInit()) {
        glfwSetErrorCallback(error_callback);

        GLFWwindow* window = initialize_window();
        sleep(1);
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}
