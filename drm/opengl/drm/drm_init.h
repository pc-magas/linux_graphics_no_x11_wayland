#ifndef DRM_INIT_H
#define DRM_INIT_H

#include <EGL/egl.h>  
#include <EGL/eglext.h>  
 
// Define EGL extensions (if not in headers)  
#define EGL_PLATFORM_GBM_KHR 0x31D7

EGLDisplay initEGLDisplay(gbm_device *gbm_dev);

EGLSurface initEGLSurface(gbm_device *gbm_dev, EGLConfig config, unsigned int width, unsigned int height);

EGLContext initEGLContext(EGLDisplay display,EGLConfig config);

#endif