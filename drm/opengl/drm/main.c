#include <stdlib.h>
#include <fcntl.h>       
#include <unistd.h>

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

    gbm_display = initEGLDisplay(gbm_dev);

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


    GLuint program = createProgram();
    if (!program) {
        return 1;
    }

    while (true) {
        drawSquare(program, width, height);
        eglSwapBuffers(gbm_display, gbm_surface);

        // struct gbm_bo *bo = gbm_surface_lock_front_buffer(gbm_surface);  
        // drmModeSetCrtc(drm_fd, crtc_id, gbm_bo_get_handle(bo).u32, 0, 0, &conn_id, 1, &mode);  
    }

    eglDestroySurface(gbm_display, gbm_surface);  
    eglDestroyContext(gbm_display, gbm_context);  
    eglTerminate(gbm_display);  
    gbm_device_destroy(gbm_dev);  
    close(drm_fd);

    return 0;
}