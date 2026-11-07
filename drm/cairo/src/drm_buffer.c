#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <drm.h>
#include <drm_mode.h>

#include "drm_buffer.h"

void *init_mmap(int drm_fd,uint32_t width,uint32_t height,uint32_t *pitch,uint32_t *handle,uint64_t *size)
{
    struct drm_mode_create_dumb creq = {0};

    creq.width = width;
    creq.height = height;
    creq.bpp = 32;

    if (drmIoctl(drm_fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq) < 0) {
        perror("CREATE_DUMB");
        return NULL;
    }

    *pitch = creq.pitch;
    *handle = creq.handle;
    *size = creq.size;

    struct drm_mode_map_dumb mreq = {0};
    mreq.handle = *handle;

    if (drmIoctl(drm_fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq) < 0) {
        perror("MAP_DUMB");
        return NULL;
    }

    void *map = mmap(
        NULL,
        *size,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        drm_fd,
        mreq.offset
    );

    if (map == MAP_FAILED) {
        perror("mmap");
        return NULL;
    }

    return map;
}