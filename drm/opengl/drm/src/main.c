#include <stdlib.h>
#include <fcntl.h>       
#include <unistd.h>
#include <stdio.h>

#define GLFW_INCLUDE_ES2

#include <drm.h>  
#include <drm_mode.h>  
#include <gbm.h>
#include <xf86drm.h>

#include "square.h"
#include "egl_init.h"
#include "drm_init.h"

#define WIDTH 1920
#define HEIGHT 1080

int main(void)
{
    int drm_fd = open("/dev/dri/card0", O_RDWR);  
    struct gbm_device *gbm_dev = gbm_create_device(drm_fd);  
    
    struct gbm_surface *gbm_surf=NULL;

    EGLDisplay egl_display;
    EGLSurface egl_surface;
    EGLContext egl_context;

    uint32_t conn_id,crtc_id;

    egl_display = initEGLDisplay(gbm_dev);

    if(egl_display == EGL_NO_DISPLAY){
        fprintf(stderr,"Unable to Initialize EGL display");
        return 1;
    }

    EGLConfig config;  
    EGLint num_configs;

    static const EGLint config_attribs[] = {  
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,  
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8, 
        EGL_BLUE_SIZE, 8, 
        EGL_ALPHA_SIZE, 8, 
        EGL_NONE
    };

    eglChooseConfig(egl_display, config_attribs, &config, 1, &num_configs);  

    gbm_surf = gbm_surface_create(gbm_dev, width, height, GBM_BO_FORMAT_ARGB8888, GBM_BO_USE_RENDERING);  
    egl_surface = eglCreateWindowSurface(egl_display, config, (EGLNativeWindowType)gbm_surf, NULL);

    if (egl_surface == EGL_NO_SURFACE) {
        fprintf(stderr, "Failed to create EGL surface\n");
        return 1;
    }

    if (!gbm_surf) {
        fprintf(stderr, "Failed to create GBM surface\n");
        return 1;
    }

    egl_context = initEGLContext(egl_display,config);


    eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);  

    drmModeConnector *connector = getDRMConnector(drm_fd);
    if (!connector) {
        fprintf(stderr, "No connected DRM connector found\n");
        close(drm_fd);
        return 1;
    }

    drmModeCrtc *crtc = getDRMCrtc(drm_fd, connector);
    if (!crtc) {
        fprintf(stderr, "No compatible CRTC found\n");
        drmModeFreeConnector(connector);
        close(drm_fd);
        return 1;
    }

    conn_id = connector->connector_id;
    crtc_id = crtc->crtc_id;
    drmModeModeInfo mode = connector->modes[0];

    GLuint program = createProgram();
    if (!program) {
        return 1;
    }

    while (true) {
        drawSquare(program, WIDTH, HEIGHT);
        eglSwapBuffers(egl_display, egl_surface);

        struct gbm_bo *bo = gbm_surface_lock_front_buffer(egl_surface);  
        drmModeSetCrtc(drm_fd, crtc_id, gbm_bo_get_handle(bo).u32, 0, 0, &conn_id, 1, &mode);  
    }

    eglDestroySurface(egl_display, egl_surface);  
    eglDestroyContext(egl_display, egl_context);  
    eglTerminate(egl_display);  
    gbm_device_destroy(gbm_dev);  
    close(drm_fd);

    return 0;
}