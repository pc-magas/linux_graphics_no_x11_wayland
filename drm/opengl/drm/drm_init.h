#ifndef DRM_INIT_H
#define DRM_INIT_H

#include <EGL/egl.h>  
#include <EGL/eglext.h>  
#include <stddef.h>
#include <gbm.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

// Define EGL extensions (if not in headers)  
#define EGL_PLATFORM_GBM_KHR 0x31D7

EGLDisplay initEGLDisplay(struct gbm_device *gbm_dev);

EGLSurface initEGLSurface(struct gbm_device *gbm_dev,EGLDisplay display, EGLConfig config, unsigned int width, unsigned int height);

EGLContext initEGLContext(EGLDisplay display,EGLConfig config);

drmModeConnector *getDRMConnector(int drm_fd);
drmModeCrtc *getDRMCrtc(int drm_fd,drmModeConnector *connector);



#endif