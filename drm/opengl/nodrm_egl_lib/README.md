# OpenGL ES demo using a seperate lib

This is a intermediate version step that we are trying to render graphics directly into screen. 

From `nodrm_egl` example we are moving the square rendering logic into its own dedicated library. The reqason why is because we want to remove the dependency on OpenGL and use only Opengl ES to render the graphics directly into the screen.