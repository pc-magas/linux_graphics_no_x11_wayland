
#include "drm_init.h"
#include <stddef.h>

EGLDisplay initEGLDisplay(struct gbm_device *gbm_dev) {
    
    PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT =  eglGetPlatformDisplayEXT(EGL_PLATFORM_GBM_KHR, gbm_dev, NULL);
    
    EGLDisplay egl_display = eglGetPlatformDisplayEXT(EGL_PLATFORM_GBM_KHR, gbm_dev, NULL);  
    
    eglInitialize(egl_display, NULL, NULL);

    return egl_display;
}

EGLSurface initEGLSurface(struct gbm_device *gbm_dev, EGLDisplay display, EGLConfig config, unsigned int width, int height) {

    struct gbm_surface *gbm_surf = gbm_surface_create(gbm_dev, width, height, GBM_BO_FORMAT_ARGB8888, GBM_BO_USE_RENDERING);  
 
    return eglCreateWindowSurface(display, config, (EGLNativeWindowType)gbm_surf, NULL);
}

EGLContext initEGLContext(EGLDisplay display, EGLConfig config){
    return eglCreateContext(display, config, EGL_NO_CONTEXT, (const EGLint[]){EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE});  
}
