#ifndef SQUARE_H
#define SQUARE_H

#include <GLES2/gl2.h>

GLuint createShader(GLenum type, const char *source);
GLuint createProgram(GLuint f);
GLuint crateShader();

void drawSquare(GLuint program, int width, int height);

#endif