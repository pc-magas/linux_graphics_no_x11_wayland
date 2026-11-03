# Basic Vulkan


## Setup

```shell
sudo apt-get install build-essential make libvulkan-dev libglfw3-dev libgl1-mesa-dev vulkan-validationlayers-dev spirv-tools glslang-tools
```

## Build & run

```shell
g++ -std=c++20 -Wall -Wextra -O2 main.cpp -o HelloTriangle \
    $(pkg-config --cflags --libs glfw3) \
    -lvulkan

./HelloTriangle
```