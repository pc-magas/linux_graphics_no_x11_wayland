#include <stdlib.h>
#include <fcntl.h>

#define GLFW_INCLUDE_ES2

#include <drm.h>  
#include <drm_mode.h>  
#include <gbm.h>


#include "square.h"
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

    int width, height;

    gbm_display = initEGLDisplay(gbm_device);

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

    eglChooseConfig(egl_dpy, config_attribs, &config, 1, &num_configs);  

    gbm_surface = initEGLSurface(display,config,WIDTH,HEIGHT); 
    gbm_context = initEGLContect(display,config);

    eglMakeCurrent(egl_dpy, egl_surf, egl_surf, egl_ctx);  


    GLuint program = createProgram();
    if (!program) {
        return 1;
    }

    while (true) {
        drawSquare(program, width, height);
        eglSwapBuffers(egl_dpy, egl_surf);

        struct gbm_bo *bo = gbm_surface_lock_front_buffer(gbm_surf);  
        drmModeSetCrtc(drm_fd, crtc_id, gbm_bo_get_handle(bo).u32, 0, 0, &conn_id, 1, &mode);  
    }

    eglDestroySurface(egl_dpy, egl_surf);  
    eglDestroyContext(egl_dpy, egl_ctx);  
    eglTerminate(egl_dpy);  
    gbm_device_destroy(gbm_dev);  
    close(drm_fd);

    return 0;
}