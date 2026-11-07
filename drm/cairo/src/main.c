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

    /* Everything cleanup touches, initialized to "not created" */
    int drm_fd = -1;
    drmModeConnector *connector = NULL;
    drmModeCrtc *crtc = NULL;

    void *map = NULL;
    uint32_t handle = 0;
    uint32_t pitch  = 0;
    uint64_t size   = 0;
    uint32_t fb_id  = 0;

    cairo_surface_t *surface = NULL;
    cairo_t *cairo = NULL;

    /* Now the real work; plain assignments, not declarations */
    drm_fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
    if (drm_fd < 0) {
        perror("open /dev/dri/card0");
        status_code = 1;
        goto cleanup;
    }

    connector = getDRMConnector(drm_fd);
    if (!connector) {
        fprintf(stderr, "No connected DRM connector found\n");
        status_code = 1;
        goto cleanup;
    }

    drmModeModeInfo mode = connector->modes[0];   /* see note below */
    uint32_t mode_w = mode.hdisplay;
    uint32_t mode_h = mode.vdisplay;

    crtc = getDRMCrtc(drm_fd, connector);
    if (!crtc) {
        fprintf(stderr, "No compatible CRTC found\n");
        status_code = 1;
        goto cleanup;
    }

    map = init_mmap(drm_fd, mode_w, mode_h, &pitch, &handle, &size);
    if (!map) {
        fprintf(stderr, "Failed to initialize mmap\n");
        status_code = 1;
        goto cleanup;
    }

    if (displayInit(drm_fd, crtc->crtc_id, connector->connector_id,
                    &mode, handle, pitch, &fb_id) != 0) {
        fprintf(stderr, "Failed to initialize display\n");
        status_code = 1;
        goto cleanup;
    }

    cairo = init_cairo(map, mode_w, mode_h, pitch, &surface);
    if (!cairo) {
        status_code = 1;
        goto cleanup;
    }

    draw_with_cairo(cairo);
    cairo_surface_flush(surface);
    getchar();

cleanup:
    if (cairo)   cairo_destroy(cairo);
    if (surface) cairo_surface_destroy(surface);
    if (map)     drmUnmap(map, size);

    if (drm_fd >= 0 && fb_id)
        drmModeRmFB(drm_fd, fb_id);

    if (drm_fd >= 0 && handle) {
        struct drm_mode_destroy_dumb destroy = { .handle = handle };
        drmIoctl(drm_fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy);
    }

    if (crtc)      drmModeFreeCrtc(crtc);
    if (connector) drmModeFreeConnector(connector);
    if (drm_fd >= 0) close(drm_fd);

    return status_code;
}