CXX      = g++
GLSLC    = glslangValidator -V
TARGET   = HelloTriangle
SOURCE   = main.cpp

CXXFLAGS = -std=c++20 -Wall -Wextra -O2
CXXFLAGS += $(shell pkg-config --cflags glfw3)

LDFLAGS  = $(shell pkg-config --libs glfw3) -lvulkan

SHADER_DIR  = shaders
RELEASE_DIR = release
OUT_SHADERS = $(RELEASE_DIR)/shaders
SPV = $(OUT_SHADERS)/vert.spv $(OUT_SHADERS)/frag.spv

all: $(RELEASE_DIR)/$(TARGET) shaders

$(RELEASE_DIR)/$(TARGET): $(SOURCE) | $(RELEASE_DIR)
	$(CXX) $(CXXFLAGS) $(SOURCE) -o $@ $(LDFLAGS)

shaders: $(SPV)

$(OUT_SHADERS)/vert.spv: $(SHADER_DIR)/shader.vert | $(OUT_SHADERS)
	$(GLSLC) $< -o $@

$(OUT_SHADERS)/frag.spv: $(SHADER_DIR)/shader.frag | $(OUT_SHADERS)
	$(GLSLC) $< -o $@

$(RELEASE_DIR) $(OUT_SHADERS):
	mkdir -p $@

run: all
	cd $(RELEASE_DIR) && ./$(TARGET)

clean:
	rm -rf $(RELEASE_DIR)

.PHONY: all shaders run clean