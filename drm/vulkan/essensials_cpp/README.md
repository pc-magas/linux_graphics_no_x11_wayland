# Basic Vulkan


## Setup

We need to setup the following dependencies:

```shell
sudo apt-get install build-essential make libvulkan-dev libglfw3-dev libgl1-mesa-dev vulkan-validationlayers-dev spirv-tools glslang-tools
```

## Build & run

We are using make to build our application

```shell
make
make run
```

Once build you can also run the application like this:

```shell
./release/HelloTriangle
```

## Built Application and shaders

The shaders and the build application is locates inside `./release` folder

## Clean built artifacts

Run:

```shell
make clean
```