#ifndef DRM_INIT_H
#define DRM_INIT_H

#include <stddef.h>
#include <gbm.h>
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
int displayRefresh(int drm_fd, struct gbm_surface *gbm_surf, uint32_t crtc_id, uint32_t conn_id,drmModeModeInfo *mode, DisplayFrame *current);

#endif