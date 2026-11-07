#ifndef DRM_INIT_H
#define DRM_INIT_H

#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

typedef struct {
    struct gbm_bo *bo;
    uint32_t fb_id;
} DisplayFrame;

drmModeConnector *getDRMConnector(int drm_fd);
drmModeCrtc *getDRMCrtc(int drm_fd,drmModeConnector *connector);
int displayInit( int drm_fd, uint32_t crtc_id, uint32_t connector_id, drmModeModeInfo *mode, uint32_t handle, uint32_t pitch, uint32_t *fb_id);

#endif