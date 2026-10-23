#include <GLFW/glfw3.h>
 
int main(void) {
    glfwInit();
    GLFWwindow *w = glfwCreateWindow(640, 480, "", 0, 0);
    glfwMakeContextCurrent(w);
    while (!glfwWindowShouldClose(w)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glColor3f(1, 0, 0);
        glRectf(-0.5f, -0.5f, 0.5f, 0.5f);
        glfwSwapBuffers(w);
        glfwPollEvents();
    }
}
