# OpenGL Demo

Render a single square having a single color.

## Install dependencies

```
sudo apt install build-essential libglfw3-dev libegl-dev libgl-dev libegl1-mesa-dev libgles2-mesa-dev libglfw3-dev
```

## Compile

In order to compile this demo run:

```
gcc main.c -o main $(pkg-config --cflags --libs glfw3 glesv2)
```

## Run

In case you have X11 Installed run

```
./main
```

This example currenly uses X11 and no wayland.

