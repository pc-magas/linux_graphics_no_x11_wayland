#ifndef NEXTFRAME_H
#define NEXTFRAME_H

#include <stdint.h>
#include <gbm.h>
#include <xf86drmMode.h>

typedef struct {
    struct gbm_bo *bo;
    uint32_t fb_id;
} DisplayFrame;

int displayFirstFrame(int drm_fd, struct gbm_surface *surface, uint32_t crtc_id, uint32_t conn_id, drmModeModeInfo *mode, DisplayFrame *current );


int displayNextFrame(
    int drm_fd,
    struct gbm_surface *surface,
    uint32_t crtc_id,
    DisplayFrame *current
);

#endif