#include "egl_init.h"

EGLDisplay initEGLDisplay(struct gbm_device *gbm_dev)
{
    PFNEGLGETPLATFORMDISPLAYEXTPROC getPlatformDisplay =
        (PFNEGLGETPLATFORMDISPLAYEXTPROC)
        eglGetProcAddress("eglGetPlatformDisplayEXT");

    if (!getPlatformDisplay) {
        return EGL_NO_DISPLAY;
    }

    EGLDisplay egl_display = getPlatformDisplay(EGL_PLATFORM_GBM_KHR,gbm_dev,NULL);

    if (egl_display == EGL_NO_DISPLAY) {
        return EGL_NO_DISPLAY;
    }

    if (!eglInitialize(egl_display, NULL, NULL)) {
        return EGL_NO_DISPLAY;
    }

    return egl_display;
}


EGLSurface initEGLSurface(struct gbm_device *gbm_dev,EGLDisplay display, EGLConfig config, unsigned int width, unsigned int height, struct gbm_surface **gbm_surf ){
    *gbm_surf = gbm_surface_create(gbm_dev, width, height, GBM_FORMAT_XRGB8888, GBM_BO_USE_RENDERING|GBM_BO_USE_SCANOUT);  
    return eglCreateWindowSurface(display, config, (EGLNativeWindowType)*gbm_surf, NULL);
}

EGLContext initEGLContext(EGLDisplay display, EGLConfig config){
    return eglCreateContext(display, config, EGL_NO_CONTEXT, (const EGLint[]){EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE});  
}

