CC      ?= gcc
CFLAGS  += -Wall -Wextra -O2 $(shell pkg-config --cflags glfw3 glesv2)
LDLIBS  = $(shell pkg-config --libs glfw3 glesv2)

TARGET  = main
OBJS    = main.o square.o drm_init.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDLIBS)

main.o: main.c square.h
square.o: square.c square.h
drm_init.o: drm_init.c drm_init.h

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all clean