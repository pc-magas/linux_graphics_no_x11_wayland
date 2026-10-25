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

    puts("Initiaslize display \n");
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

    puts("Initiaslize surface \n");
    egl_surface = initEGLSurface(gbm_dev,egl_display,config,WIDTH,HEIGHT, &gbm_surf);

    if (egl_surface == EGL_NO_SURFACE) {
        fprintf(stderr, "Failed to create EGL surface\n");
        return 1;
    }

    if (!gbm_surf) {
        fprintf(stderr, "Failed to create GBM surface\n");
        return 1;
    }

    puts("Initiaslize context \n");
    egl_context = initEGLContext(egl_display,config);
    eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);  

    puts("Get Connector \n");
    drmModeConnector *connector = getDRMConnector(drm_fd);
    if (!connector) {
        fprintf(stderr, "No connected DRM connector found\n");
        close(drm_fd);
        return 1;
    }

    puts("Get CRTC\n");
    drmModeCrtc *crtc = getDRMCrtc(drm_fd, connector);
    if (!crtc) {
        fprintf(stderr, "No compatible CRTC found\n");
        drmModeFreeConnector(connector);
        close(drm_fd);
        return 1;
    }

    drmModeModeInfo mode = connector->modes[0];

    puts("Init program\n");
    GLuint program = createProgram();
    if (!program) {
        return 1;
    }
     
    puts("Draw Square\n");

    drawSquare(program, WIDTH, HEIGHT);

    puts("Swap Buffers\n");
    eglSwapBuffers(egl_display, egl_surface);
        
    puts("Lock buffers");
    struct gbm_bo *bo = gbm_surface_lock_front_buffer(gbm_surf);
    drmModeSetCrtc(drm_fd, res->crtcs[0], gbm_bo_get_handle(bo).u32, 0, 0,  &connector->connector_id, 1, mode);  

    puts("Framebuffer displayed");

    sleep(10);

    // struct gbm_bo *previous_bo = NULL;
    // uint32_t previous_fb_id = 0;

   
    // while (true) {

    //     puts("Draw Square\n");

    //     drawSquare(program, WIDTH, HEIGHT);

    //     puts("Swap Buffers\n");
    //     eglSwapBuffers(egl_display, egl_surface);
        
    //     puts("Lock buffers");
    //     struct gbm_bo *bo = gbm_surface_lock_front_buffer(gbm_surf);

    //     if (!bo) {
    //         fprintf(stderr, "gbm_surface_lock_front_buffer failed\n");
    //         break;
    //     }

    //     uint32_t fb_id = 0;

    //     // Register GBM buffer as a DRM framebuffer
    //     int ret = drmModeAddFB(
    //         drm_fd,
    //         gbm_bo_get_width(bo),
    //         gbm_bo_get_height(bo),
    //         24,
    //         32,
    //         gbm_bo_get_stride(bo),
    //         gbm_bo_get_handle(bo).u32,
    //         &fb_id
    //     );

    //     if (ret != 0) {
    //         perror("drmModeAddFB");
    //         gbm_surface_release_buffer(gbm_surf, bo);
    //         break;
    //     }

    //     printf("GBM handle: %u\n", gbm_bo_get_handle(bo).u32);
    //     printf("DRM framebuffer ID: %u\n", fb_id);

    //     puts("Set CRTC");

    //     ret = drmModeSetCrtc(
    //         drm_fd,
    //         crtc_id,
    //         fb_id,
    //         0,
    //         0,
    //         &conn_id,
    //         1,
    //         &mode
    //     );

    //     if (ret != 0) {
    //         perror("drmModeSetCrtc");
    //         drmModeRmFB(drm_fd, fb_id);
    //         gbm_surface_release_buffer(gbm_surf, bo);
    //         break;
    //     }

    //     // Previous framebuffer is no longer scanned out
    //     // by this CRTC after successful modeset.
    //     if (previous_bo) {
    //         drmModeRmFB(drm_fd, previous_fb_id);
    //         gbm_surface_release_buffer(gbm_surf, previous_bo);
    //     }

    //     previous_bo = bo;
    //     previous_fb_id = fb_id;
    // }

    puts("Cleanup\n");

    eglDestroySurface(egl_display, egl_surface);  
    eglDestroyContext(egl_display, egl_context);  
    eglTerminate(egl_display);  
    gbm_device_destroy(gbm_dev);  
    close(drm_fd);

    return 0;
}