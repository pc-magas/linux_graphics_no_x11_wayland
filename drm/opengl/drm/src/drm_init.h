#ifndef DRM_INIT_H
#define DRM_INIT_H

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <stddef.h>


drmModeConnector *getDRMConnector(int drm_fd);
drmModeCrtc *getDRMCrtc(int drm_fd,drmModeConnector *connector);

#endif