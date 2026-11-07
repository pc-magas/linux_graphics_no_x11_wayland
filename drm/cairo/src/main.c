#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

#include <xf86drm.h>
#include <xf86drmMode.h>

#include "drm_init.h"
#include "cairo_render.h"
#include "drm_buffer.h"

int main(void)
{
    int drm_fd = open("/dev/dri/card0", O_RDWR);

    puts("Get Connector \n");

    drmModeConnector *connector = getDRMConnector(drm_fd);
    if (!connector) {
        fprintf(stderr, "No connected DRM connector found\n");
        close(drm_fd);
        return 1;
    }

    drmModeModeInfo mode = connector->modes[0];

    unsigned int mode_w = mode.hdisplay;
    unsigned int mode_h = mode.vdisplay;

    puts("Get CRTC\n");

    drmModeCrtc *crtc = getDRMCrtc(drm_fd, connector);
    if (!crtc) {
        fprintf(stderr, "No compatible CRTC found\n");
        drmModeFreeConnector(connector);
        close(drm_fd);
        return 1;
    }

    uint32_t pitch;
    uint32_t handle;
    uint64_t size;

    void *map = init_mmap(drm_fd,mode_w,mode_h,&pitch,&handle,&size);

    if (!map) {
        fprintf(stderr, "Failed to initialize mmap\n");
        return 1;
    }

    cairo_surface_t *surface=NULL;
    cairo_t *cairo = init_cairo(map, mode_w, mode_h, pitch, &surface);



    return 0;
}