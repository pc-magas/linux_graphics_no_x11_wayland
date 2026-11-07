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
    int status_code = 0;

    int drm_fd = open("/dev/dri/card0", O_RDWR);

    if(drm_fd < 0) {
        goto cleanup;
    }

    drmModeConnector *connector = getDRMConnector(drm_fd);
    if (!connector) {
        status_code=1;
        goto cleanup;
    }

    drmModeModeInfo mode = connector->modes[0];

    unsigned int mode_w = mode.hdisplay;
    unsigned int mode_h = mode.vdisplay;

    puts("Get CRTC\n");

    drmModeCrtc *crtc = getDRMCrtc(drm_fd, connector);
    if (!crtc) {
        fprintf(stderr, "No compatible CRTC found\n");
        status_code=1;
        goto cleanup;
    }

    uint32_t pitch;
    uint32_t handle;
    uint64_t size;

    void *map = init_mmap(drm_fd,mode_w,mode_h,&pitch,&handle,&size);

    if (!map) {
        fprintf(stderr, "Failed to initialize mmap\n");
        status_code=1;
        goto cleanup;
    }

    cairo_surface_t *surface=NULL;
    cairo_t *cairo = init_cairo(map, mode_w, mode_h, pitch, &surface);

    while(true){
        draw_with_cairo(cairo);
        cairo_surface_flush(surface);
    }

    cleanup:
        /* Destroy Cairo resources before unmapping their backing memory. */
        if (cairo)
            cairo_destroy(cairo);

        if (surface)
            cairo_surface_destroy(surface);

        if (map)
            munmap(map, size);

        /* Destroy the dumb buffer allocated by init_mmap(). */
        if (drm_fd >= 0 && handle != 0) {
            struct drm_mode_destroy_dumb destroy = {
                .handle = handle
            };

            if (drmIoctl(drm_fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy) < 0)
                perror("Failed to destroy dumb buffer");
        }

        if (crtc)
            drmModeFreeCrtc(crtc);

        if (connector)
            drmModeFreeConnector(connector);

        if (drm_fd >= 0)
            close(drm_fd);

        puts("Cleanup complete");
        return status_code;
}