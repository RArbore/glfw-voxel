#include <stdio.h>
#include <unistd.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

void error_callback(int error, const char* description) {
    fprintf(stderr, "Error #%d: %s\n", error, description);
}

void resize_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

GLFWwindow* initialize_window() {
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    GLFWwindow *window = glfwCreateWindow(800, 600, "Voxel Engine", NULL, NULL);
    if (window) {
        glfwShowWindow(window);
        glfwMakeContextCurrent(window);
        gladLoadGL();
        glfwSwapInterval(1); //vsync
        glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    }
    return window;
}

void render(GLFWwindow *window) {
    glClear(GL_COLOR_BUFFER_BIT);



    glfwSwapBuffers(window);
    glfwPollEvents();
}

void tick(GLFWwindow *window) {
    render(window);
}

int main(int argc, char** argv) {
    if (!glfwInit()) {
        return 1;
    }
    glfwSetErrorCallback(error_callback);

    GLFWwindow *window = initialize_window();


    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glfwSetFramebufferSizeCallback(window, resize_callback);

    while (!glfwWindowShouldClose(window) && !glfwGetKey(window, GLFW_KEY_ESCAPE)) {
        tick(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}
