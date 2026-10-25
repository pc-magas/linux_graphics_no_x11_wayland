# Minimal DRM for OPENGLES

A minimal example that renders a square upon screen directly without using OpenGL but only OpenGL ES diurectly to DRM.

## Build steps

### Setup Dependencies:

```
sudo apt install build-essential pkg-config libdrm-dev libgbm-dev libegl-dev libgles-dev mesa-utils libgl1-mesa-dri libdrm-dev
```

### Build

```
make clean
make
```

## Run:

```
./main
```