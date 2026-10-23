# Using OpenGL to render graphics directly into the screen using DRM

This folder contains intermediate steps that start from a windowed OpenGL demo and gradually moving towards OpenGL ES.

The steps are into dedicated folders:

* `nodrm` uses OpenGL (`GLFW`) to render the graphics
* `nodrm_egl` We are using OpenGL (`GLFW`) for window only whereas we are using OpenGL ES (`EGL`) for graphics
* `nodrm_egl_lib` We are creating a seperate library that uses OpenGL ES (`EGL`) for rendering our gra1phics whereas the OpenGL (`GLFW`) is still used for window
* `drm` Uses the previous library alongside the `card0` DRM device to render the graphics directly into the display

Each odler has a README.md that shows how to build and run the examples.