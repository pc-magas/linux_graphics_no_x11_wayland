#include <stdio.h>
#include <stdlib.h>

#define GLFW_INCLUDE_ES2
#include <GLFW/glfw3.h>
#include "square.h"

int main(void)
{
    if (!glfwInit())
        return 1;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow *w = glfwCreateWindow(640, 480, "GLES square", NULL, NULL);
    if (!w) {
        fprintf(stderr, "Failed to create window/context\n");
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(w);
    glfwSwapInterval(1);

    GLuint program = createProgram();
    if (!program) {
        glfwDestroyWindow(w);
        glfwTerminate();
        return 1;
    }

    int width, height;
    glfwGetFramebufferSize(w, &width, &height);

    while (!glfwWindowShouldClose(w)) {
        drawSquare(program, width, height);
        glfwSwapBuffers(w);
        glfwPollEvents();
    }

    glDeleteProgram(program);
    glfwDestroyWindow(w);
    glfwTerminate();
    return 0;
}