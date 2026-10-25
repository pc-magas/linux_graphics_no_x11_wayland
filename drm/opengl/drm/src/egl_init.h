#ifndef EGL_INIT_H
#define EGL_INIT_H

#include <EGL/egl.h>
#include <EGL/eglext.h>  
#include <gbm.h>

// DRM Headers
#include <xf86drm.h>


// Define EGL extensions (if not in headers)  
#define EGL_PLATFORM_GBM_KHR 0x31D7

EGLDisplay initEGLDisplay(struct gbm_device *gbm_dev);

EGLSurface initEGLSurface(struct gbm_device *gbm_dev,EGLDisplay display, EGLConfig config, unsigned int width, unsigned int height, struct gbm_surface *gbm_surf ){

EGLContext initEGLContext(EGLDisplay display,EGLConfig config);

#endif