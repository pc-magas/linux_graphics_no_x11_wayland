CXX = g++
TARGET = HelloTriangle
SOURCE = main.cpp

CXXFLAGS = -std=c++20 -Wall -Wextra -O2
CXXFLAGS += $(shell pkg-config --cflags glfw3)

LDFLAGS = $(shell pkg-config --libs glfw3) -lvulkan



all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) $(SOURCE) -o $(TARGET) $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean