#include <stdlib.h>
#include <fcntl.h>       
#include <unistd.h>
#include <stdio.h>

#define GLFW_INCLUDE_ES2

#include <drm.h>  
#include <drm_mode.h>  
#include <gbm.h>


#include "square.h"
#include "egl_init.h"
#include "drm_init.h"

#define WIDTH 1920
#define HEIGHT 1080

int main(void)
{
    int drm_fd = open("/dev/dri/card0", O_RDWR);  
    struct gbm_device *gbm_dev = gbm_create_device(drm_fd);  
    
    EGLDisplay gbm_display;
    EGLSurface gbm_surface;
    EGLContext gbm_context;

    uint32_t conn_id,crtc_id;

    gbm_display = initEGLDisplay(gbm_dev);

    if(gbm_display == EGL_NO_DISPLAY){
        fprintf(stderr,"Unable to Initialize display");
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

    eglChooseConfig(gbm_display, config_attribs, &config, 1, &num_configs);  

    gbm_surface = initEGLSurface(gbm_dev,gbm_display,config,WIDTH,HEIGHT); 
    gbm_context = initEGLContext(gbm_display,config);

    eglMakeCurrent(gbm_display, gbm_surface, gbm_surface, gbm_context);  

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
        eglSwapBuffers(gbm_display, gbm_surface);

        struct gbm_bo *bo = gbm_surface_lock_front_buffer(gbm_surface);  
        drmModeSetCrtc(drm_fd, crtc_id, gbm_bo_get_handle(bo).u32, 0, 0, &conn_id, 1, &mode);  
    }

    eglDestroySurface(gbm_display, gbm_surface);  
    eglDestroyContext(gbm_display, gbm_context);  
    eglTerminate(gbm_display);  
    gbm_device_destroy(gbm_dev);  
    close(drm_fd);

    return 0;
}