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

#endif