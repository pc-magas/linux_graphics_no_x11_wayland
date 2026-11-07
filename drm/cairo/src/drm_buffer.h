#ifndef DRM_BUFFER_H
#define DRM_BUFFER_H

#include <stdint.h>

void *init_mmap(
    int drm_fd,
    uint32_t width,
    uint32_t height,
    uint32_t *pitch,
    uint32_t *handle,
    uint64_t *size
);

#endif