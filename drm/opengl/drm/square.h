#ifndef SQUARE_H
#define SQUARE_H

static GLuint createShader(GLenum type, const char *source);
static GLuint createProgram(void);
static void drawSquare(GLuint program, int width, int height);

#endif