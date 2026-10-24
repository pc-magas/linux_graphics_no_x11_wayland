
#include "drm_init.h"


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


EGLSurface initEGLSurface(struct gbm_device *gbm_dev,EGLDisplay display, EGLConfig config, unsigned int width, unsigned int height){

    struct gbm_surface *gbm_surf = gbm_surface_create(gbm_dev, width, height, GBM_BO_FORMAT_ARGB8888, GBM_BO_USE_RENDERING);  
 
    return eglCreateWindowSurface(display, config, (EGLNativeWindowType)gbm_surf, NULL);
}

EGLContext initEGLContext(EGLDisplay display, EGLConfig config){
    return eglCreateContext(display, config, EGL_NO_CONTEXT, (const EGLint[]){EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE});  
}


drmModeConnector *getDRMConnector(int drm_fd)
{
    drmModeRes *resources = drmModeGetResources(drm_fd);

    if (!resources){
        return NULL;
    }

    drmModeConnector *connector = NULL;

    for (int i = 0; i < resources->count_connectors; i++) {
        drmModeConnector *candidate = drmModeGetConnector(
            drm_fd,
            resources->connectors[i]
        );

        if (!candidate){
            continue;
        }

        if (candidate->connection == DRM_MODE_CONNECTED &&
            candidate->count_modes > 0) {
            connector = candidate;
            break;
        }

        drmModeFreeConnector(candidate);
    }

    drmModeFreeResources(resources);

    return connector;
}


drmModeCrtc *getDRMCrtc( int drm_fd, drmModeConnector *connector )
{
    if (!connector)
        return NULL;

    drmModeRes *resources = drmModeGetResources(drm_fd);

    if (!resources)
        return NULL;

    drmModeCrtc *crtc = NULL;

    for (int i = 0; i < connector->count_encoders && !crtc; i++) {
        drmModeEncoder *encoder = drmModeGetEncoder(
            drm_fd,
            connector->encoders[i]
        );

        if (!encoder)
            continue;

        for (int j = 0; j < resources->count_crtcs; j++) {
            if (encoder->possible_crtcs & (1u << j)) {
                crtc = drmModeGetCrtc(
                    drm_fd,
                    resources->crtcs[j]
                );

                if (crtc)
                    break;
            }
        }

        drmModeFreeEncoder(encoder);
    }

    drmModeFreeResources(resources);

    return crtc;
}

