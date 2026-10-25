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

int main(void)
{
    int drm_fd = open("/dev/dri/card0", O_RDWR);  
    struct gbm_device *gbm_dev = gbm_create_device(drm_fd);  
    
    struct gbm_surface *gbm_surf=NULL;

    EGLContext egl_context;

    puts("Get Connector \n");
    drmModeConnector *connector = getDRMConnector(drm_fd);
    if (!connector) {
        fprintf(stderr, "No connected DRM connector found\n");
        close(drm_fd);
        return 1;
    }

    drmModeModeInfo mode = connector->modes[0];

    unsigned int mode_w = mode.hdisplay;
    unsigned int mode_h = mode.vdisplay;

    puts("Get CRTC\n");
    drmModeCrtc *crtc = getDRMCrtc(drm_fd, connector);
    if (!crtc) {
        fprintf(stderr, "No compatible CRTC found\n");
        drmModeFreeConnector(connector);
        close(drm_fd);
        return 1;
    }

    puts("Initiaslize display \n");
    EGLDisplay egl_display = initEGLDisplay(gbm_dev);

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

    puts("Initiaslize surface \n");
    EGLSurface egl_surface = initEGLSurface(gbm_dev,egl_display,config,mode_w,mode_h, &gbm_surf);

    if (egl_surface == EGL_NO_SURFACE) {
        fprintf(stderr, "Failed to create EGL surface\n");
        return 1;
    }

    if (!gbm_surf) {
        fprintf(stderr, "Failed to create GBM surface\n");
        return 1;
    }

    puts("Initiaslize context \n");
    EGLContext egl_context = initEGLContext(egl_display,config);
    eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);  


    uint32_t conn_id = connector->connector_id;
    uint32_t crtc_id = crtc->crtc_id;
    
    puts("Init program\n");
    GLuint program = createProgram();
    if (!program) {
        return 1;
    }
     
    puts("Draw Square\n");

    drawSquare(program, mode_w,mode_h);

    puts("Swap Buffers\n");
    eglSwapBuffers(egl_display, egl_surface);
        
    puts("Lock buffers"); 
    struct gbm_bo *bo = gbm_surface_lock_front_buffer(gbm_surf);
    if (!bo) {
        fprintf(stderr, "Failed to lock front buffer\n");
        return 1;
    }

    uint32_t handle = gbm_bo_get_handle(bo).u32;
    uint32_t pitch  = gbm_bo_get_stride(bo);
    uint32_t fb_id  = 0;

    int ret = drmModeAddFB(drm_fd, mode_w, mode_h, 24, 32, pitch, handle, &fb_id);
    if (ret) {
        perror("drmModeAddFB");
        return 1;
    }

    puts("Set CRTC");
    ret = drmModeSetCrtc(drm_fd, crtc_id, fb_id, 0, 0, &conn_id, 1, &mode);
    if (ret) {
        perror("drmModeSetCrtc");
        drmModeRmFB(drm_fd, fb_id);
        gbm_surface_release_buffer(gbm_surf, bo);
        return 1;
    }

    puts("Framebuffer displayed");
    sleep(10);

    
    puts("Cleanup\n");

    eglDestroySurface(egl_display, egl_surface);  
    eglDestroyContext(egl_display, egl_context);  
    eglTerminate(egl_display);  
    gbm_device_destroy(gbm_dev);  
    close(drm_fd);

    return 0;
}