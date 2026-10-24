#include <stdio.h>  
#include <stdlib.h>  
#include <fcntl.h>  
#include <unistd.h>  
#include <EGL/egl.h>  
#include <EGL/eglext.h>  
#include <GLES2/gl2.h>  
#include <drm.h>  
#include <drm_mode.h>  
#include <gbm.h>  
 
// EGL extensions  
#define EGL_PLATFORM_GBM_KHR 0x31D7  
PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT = NULL;  
 
int main() {  
  // Step 1: Open DRM device  
  int drm_fd = open("/dev/dri/card0", O_RDWR);  
  if (drm_fd < 0) { perror("open /dev/dri/card0"); return 1; }  
 
  // Step 2: Initialize GBM  
  struct gbm_device *gbm_dev = gbm_create_device(drm_fd);  
  if (!gbm_dev) { perror("gbm_create_device"); return 1; }  
 
  // Step 3: Initialize EGL  
  eglGetPlatformDisplayEXT = (PFNEGLGETPLATFORMDISPLAYEXTPROC)eglGetProcAddress("eglGetPlatformDisplayEXT");  
  EGLDisplay egl_dpy = eglGetPlatformDisplayEXT(EGL_PLATFORM_GBM_KHR, gbm_dev, NULL);  
  if (egl_dpy == EGL_NO_DISPLAY) { perror("eglGetPlatformDisplayEXT"); return 1; }  
  if (!eglInitialize(egl_dpy, NULL, NULL)) { perror("eglInitialize"); return 1; }  
 
  // Step 4: Choose EGL config  
  EGLConfig config;  
  EGLint num_configs;  
  const EGLint config_attribs[] = {  
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,  
    EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8,  
    EGL_NONE  
  };  
  if (!eglChooseConfig(egl_dpy, config_attribs, &config, 1, &num_configs)) { perror("eglChooseConfig"); return 1; }  
 
  // Step 5: Get display mode (simplified)  
  drmModeRes *res = drmModeGetResources(drm_fd);  
  drmModeConnector *conn = drmModeGetConnector(drm_fd, res->connectors[0]);  
  drmModeModeInfo *mode = &conn->modes[conn->count_modes - 1]; // Preferred mode  
 
  // Step 6: Create GBM/EGL surface  
  struct gbm_surface *gbm_surf = gbm_surface_create(gbm_dev, mode->hdisplay, mode->vdisplay,  
    GBM_BO_FORMAT_ARGB8888, GBM_BO_USE_RENDERING | GBM_BO_USE_SCANOUT);  
  EGLSurface egl_surf = eglCreateWindowSurface(egl_dpy, config, (EGLNativeWindowType)gbm_surf, NULL);  
 
  // Step 7: Create OpenGL ES context  
  EGLContext egl_ctx = eglCreateContext(egl_dpy, config, EGL_NO_CONTEXT, (const EGLint[]){EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE});  
  eglMakeCurrent(egl_dpy, egl_surf, egl_surf, egl_ctx);  
 
  // Step 8: Render (clear to red)  
  glClearColor(1.0f, 0.0f, 0.0f, 1.0f);  
  glClear(GL_COLOR_BUFFER_BIT);  
  eglSwapBuffers(egl_dpy, egl_surf);  
 
  // Step 9: Commit buffer to display  
  struct gbm_bo *bo = gbm_surface_lock_front_buffer(gbm_surf);  
  drmModeSetCrtc(drm_fd, res->crtcs[0], gbm_bo_get_handle(bo).u32, 0, 0,  
    &conn->connector_id, 1, mode);  
 
  sleep(5); // Show red screen for 5 seconds  
 
  // Cleanup  
  drmModeFreeConnector(conn);  
  drmModeFreeResources(res);  
  gbm_surface_destroy(gbm_surf);  
  eglDestroySurface(egl_dpy, egl_surf);  
  eglDestroyContext(egl_dpy, egl_ctx);  
  eglTerminate(egl_dpy);  
  gbm_device_destroy(gbm_dev);  
  close(drm_fd);  
  return 0;  
}  