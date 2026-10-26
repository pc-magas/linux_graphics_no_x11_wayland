#include <stdlib.h>
#include <fcntl.h>       
#include <unistd.h>
#include <stdio.h>
#include <time.h>

#include <signal.h>
#include <stdbool.h>


#define GLFW_INCLUDE_ES2

#include <drm.h>  
#include <drm_mode.h>  
#include <gbm.h>
#include <xf86drm.h>

#include "square.h"
#include "egl_init.h"
#include "drm_init.h"
#include "nextframe.h"

static volatile sig_atomic_t running = 1;

static void handleSignal(int sig)
{
    (void)sig;
    running = 0;
}

void cleanup(
    EGLDisplay egl_display,
    EGLSurface egl_surface,
    EGLContext egl_context,
    struct gbm_device *gbm_dev,
    struct gbm_surface *gbm_surf,
    drmModeConnector *connector,
    drmModeCrtc *crtc,
    GLuint program,
    int drm_fd)
{
    if (program){
        glDeleteProgram(program);
    }

    if (egl_display != EGL_NO_DISPLAY) {
        eglMakeCurrent(egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (egl_surface != EGL_NO_SURFACE) eglDestroySurface(egl_display, egl_surface);
        if (egl_context != EGL_NO_CONTEXT) eglDestroyContext(egl_display, egl_context);
        eglTerminate(egl_display);
    }

    if (gbm_surf) gbm_surface_destroy(gbm_surf);

    if (gbm_dev) gbm_device_destroy(gbm_dev);

    if (crtc) drmModeFreeCrtc(crtc);

    if (connector) drmModeFreeConnector(connector);

    if (drm_fd >= 0) close(drm_fd);
}

int main(void)
{
    signal(SIGINT, handleSignal);
    signal(SIGTERM, handleSignal);

    int drm_fd = open("/dev/dri/card0", O_RDWR);  
    struct gbm_device *gbm_dev = gbm_create_device(drm_fd);      
    struct gbm_surface *gbm_surf=NULL;

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

    puts("Initiaslize GBM display");

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

    puts("Init program\n");
    GLuint shader = createColorShader();
    GLuint program = createProgram(shader);
    
    if (!program) {
        return 1;
    }

    uint32_t conn_id = connector->connector_id;
    uint32_t crtc_id = crtc->crtc_id;     
    DisplayFrame current = {0};
    int firstFrame = 0;

    srand((unsigned int)time(NULL));

    puts("Draw Square\n");

    while(running){
        drawSquare(program, mode_w,mode_h);
        eglSwapBuffers(egl_display, egl_surface);
        
        if(firstFrame == 0){
            int firstframeNotShown = displayFirstFrame(drm_fd, gbm_surf, crtc_id, conn_id, &mode, &current);
            if(firstframeNotShown){
                cleanup(egl_display, egl_surface, egl_context,gbm_dev, gbm_surf, connector, crtc,program, drm_fd);
                return 1;
            }
            firstFrame=1;
            continue;
        }
        
        if (displayNextFrame( drm_fd, gbm_surf, crtc_id, &current) != 0) {
            fprintf(stderr, "Display refresh failed\n");
            cleanup(egl_display, egl_surface, egl_context,gbm_dev, gbm_surf, connector, crtc,program, drm_fd);
            return 1;
        }

        sleep(5);
    }

    cleanup(egl_display, egl_surface, egl_context,gbm_dev, gbm_surf, connector, crtc,program, drm_fd);

    return 0;
}