
CC ?= gcc

PKGS := libdrm gbm egl glesv2

CFLAGS += -std=c11 -Wall -Wextra -O2 \
          $(shell pkg-config --cflags $(PKGS))

LDLIBS := $(shell pkg-config --libs $(PKGS))

SRC_DIR := src
BUILD_DIR := build

TARGET := main

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

CFLAGS += -MMD -MP

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

-include $(DEPS)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: all clean
