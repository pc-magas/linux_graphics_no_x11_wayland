CC ?= gcc

PKGS := cairo

CFLAGS += -std=gnu11 -Wall -Wextra -O2 \
          $(shell pkg-config --cflags $(PKGS))

LDLIBS := $(shell pkg-config --libs $(PKGS))

SRC_DIR := src
BUILD_DIR := build

DESTDIR := /opt/cairo/framebuffer
TARGET := main

SERVICEFILE:= cairo_framebuffer.service
SERVICE_USERFILE:= framebufferusr.conf
SERVICE_EXEC_START := $(DESTDIR)/$(TARGET)
SERVICE_USERFILE := framebufferusr.conf
SYSUSERS_DIR := /etc/sysusers.d

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

install: $(TARGET)
	install -D -m 0755 $(TARGET) $(DESTDIR)/$(TARGET)

setup-service-file:
	sed "s|^ExecStart=.*|ExecStart=$(SERVICE_EXEC_START)|" \
		"$(SERVICEFILE)" > /etc/systemd/system/$(SERVICEFILE)

setup-service-user:
	install -D -m 0644 $(SERVICE_USERFILE) \
		$(SYSUSERS_DIR)/$(SERVICE_USERFILE)
	systemd-sysusers $(SYSUSERS_DIR)/$(SERVICE_USERFILE)

install-service: setup-service-file setup-service-user
	systemctl unmask $(SERVICEFILE)
	systemctl daemon-reload
	systemctl enable $(SERVICEFILE)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: all clean install install-service setup-service-file