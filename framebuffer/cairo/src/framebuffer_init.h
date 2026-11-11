#ifndef FRAMEBUFFER_INIT_H
#define FRAMEBUFFER_INIT_H

#include <stdint.h>

int framebuffer_init(
    int fb,
    uint32_t *width,
    uint32_t *height,
    uint32_t *pitch,
    uint32_t *bpp,
    uint64_t *size,
    void **map
);

void framebuffer_cleanup(
    int *fb_fd,
    void **map,
    uint64_t size
);

#endif