CXX      = g++
GLSLC = glslangValidator -V
TARGET   = HelloTriangle
SOURCE   = main.cpp

CXXFLAGS = -std=c++20 -Wall -Wextra -O2
CXXFLAGS += $(shell pkg-config --cflags glfw3)

LDFLAGS  = $(shell pkg-config --libs glfw3) -lvulkan

SHADER_DIR = shaders
SPV = $(SHADER_DIR)/vert.spv $(SHADER_DIR)/frag.spv

all: $(TARGET) shaders

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) $(SOURCE) -o $(TARGET) $(LDFLAGS)

shaders: $(SPV)

$(SHADER_DIR)/vert.spv: $(SHADER_DIR)/shader.vert
	$(GLSLC) $< -o $@

$(SHADER_DIR)/frag.spv: $(SHADER_DIR)/shader.frag
	$(GLSLC) $< -o $@

run: $(TARGET) $(SPV)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(SPV)

.PHONY: all shaders run clean