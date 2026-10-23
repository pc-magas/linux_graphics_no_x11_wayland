#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>
#include <errno.h>

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <gbm.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>

#include "square.h"

static volatile sig_atomic_t running = 1;
static void on_signal(int sig) { (void)sig; running = 0; }

/* ---------- framebuffer cache attached to each gbm_bo ---------- */
struct fb {
    int      drm_fd;
    uint32_t id;
};

static void fb_destroy(struct gbm_bo *bo, void *data)
{
    struct fb *fb = data;
    (void)bo;
    if (fb->id)
        drmModeRmFB(fb->drm_fd, fb->id);
    free(fb);
}

static struct fb *fb_get(int drm_fd, struct gbm_bo *bo)
{
    struct fb *fb = gbm_bo_get_user_data(bo);
    if (fb)
        return fb;

    fb = calloc(1, sizeof *fb);
    if (!fb)
        return NULL;
    fb->drm_fd = drm_fd;

    uint32_t w      = gbm_bo_get_width(bo);
    uint32_t h      = gbm_bo_get_height(bo);
    uint32_t stride = gbm_bo_get_stride(bo);
    uint32_t handle = gbm_bo_get_handle(bo).u32;

    if (drmModeAddFB(drm_fd, w, h, 24, 32, stride, handle, &fb->id)) {
        fprintf(stderr, "drmModeAddFB failed: %s\n", strerror(errno));
        free(fb);
        return NULL;
    }
    gbm_bo_set_user_data(bo, fb, fb_destroy);
    return fb;
}

/* ---------- page flip handling ---------- */
static void page_flip_handler(int fd, unsigned int frame, unsigned int sec,
                              unsigned int usec, void *data)
{
    (void)fd; (void)frame; (void)sec; (void)usec;
    *(int *)data = 0;   /* flip finished */
}

/* ---------- DRM helpers ---------- */
static int open_drm(const char *path)
{
    int fd = open(path, O_RDWR | O_CLOEXEC);
    if (fd < 0)
        fprintf(stderr, "Cannot open %s: %s\n", path, strerror(errno));
    return fd;
}

static drmModeConnector *find_connector(int fd, drmModeRes *res)
{
    for (int i = 0; i < res->count_connectors; i++) {
        drmModeConnector *c = drmModeGetConnector(fd, res->connectors[i]);
        if (c && c->connection == DRM_MODE_CONNECTED && c->count_modes > 0)
            return c;
        if (c)
            drmModeFreeConnector(c);
    }
    return NULL;
}

static drmModeModeInfo *pick_mode(drmModeConnector *c)
{
    for (int i = 0; i < c->count_modes; i++)
        if (c->modes[i].type & DRM_MODE_TYPE_PREFERRED)
            return &c->modes[i];
    return &c->modes[0];
}

static uint32_t find_crtc(int fd, drmModeRes *res, drmModeConnector *c)
{
    /* Prefer the CRTC the connector is already using */
    if (c->encoder_id) {
        drmModeEncoder *e = drmModeGetEncoder(fd, c->encoder_id);
        if (e) {
            uint32_t id = e->crtc_id;
            drmModeFreeEncoder(e);
            if (id)
                return id;
        }
    }
    /* Otherwise search all encoders / CRTCs */
    for (int i = 0; i < c->count_encoders; i++) {
        drmModeEncoder *e = drmModeGetEncoder(fd, c->encoders[i]);
        if (!e)
            continue;
        for (int j = 0; j < res->count_crtcs; j++) {
            if (e->possible_crtcs & (1u << j)) {
                uint32_t id = res->crtcs[j];
                drmModeFreeEncoder(e);
                return id;
            }
        }
        drmModeFreeEncoder(e);
    }
    return 0;
}

/* ---------- EGL helpers ---------- */
static EGLDisplay get_egl_display(struct gbm_device *gbm)
{
    PFNEGLGETPLATFORMDISPLAYEXTPROC get_platform_display =
        (PFNEGLGETPLATFORMDISPLAYEXTPROC)eglGetProcAddress("eglGetPlatformDisplayEXT");

    if (get_platform_display)
        return get_platform_display(EGL_PLATFORM_GBM_KHR, gbm, NULL);
    return eglGetDisplay((EGLNativeDisplayType)gbm);
}

/* The EGLConfig's native visual must match the GBM surface format */
static bool choose_config(EGLDisplay dpy, EGLConfig *out, EGLint gbm_format)
{
    static const EGLint attribs[] = {
        EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE,   8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE,  8,
        EGL_ALPHA_SIZE, 0,
        EGL_NONE
    };
    EGLConfig configs[64];
    EGLint n = 0;

    if (!eglChooseConfig(dpy, attribs, configs, 64, &n) || n == 0)
        return false;

    for (int i = 0; i < n; i++) {
        EGLint id;
        if (eglGetConfigAttrib(dpy, configs[i], EGL_NATIVE_VISUAL_ID, &id) &&
            id == gbm_format) {
            *out = configs[i];
            return true;
        }
    }
    return false;
}

int main(int argc, char **argv)
{
    const char *card = (argc > 1) ? argv[1] : "/dev/dri/card0";
    int ret = 1;

    signal(SIGINT,  on_signal);
    signal(SIGTERM, on_signal);

    /* ---------- Init DRM ---------- */
    int fd = open_drm(card);
    if (fd < 0)
        return 1;

    drmModeRes *res = drmModeGetResources(fd);
    if (!res) {
        fprintf(stderr, "drmModeGetResources failed (is %s a KMS device?)\n", card);
        close(fd);
        return 1;
    }

    drmModeConnector *conn = find_connector(fd, res);
    if (!conn) {
        fprintf(stderr, "No connected display found\n");
        drmModeFreeResources(res);
        close(fd);
        return 1;
    }

    drmModeModeInfo mode = *pick_mode(conn);
    uint32_t crtc_id = find_crtc(fd, res, conn);
    if (!crtc_id) {
        fprintf(stderr, "No usable CRTC found\n");
        drmModeFreeConnector(conn);
        drmModeFreeResources(res);
        close(fd);
        return 1;
    }

    drmModeCrtc *saved_crtc = drmModeGetCrtc(fd, crtc_id);
    int width  = mode.hdisplay;
    int height = mode.vdisplay;
    printf("Using %s: %dx%d@%d\n", conn->connector_id ? mode.name : "?",
           width, height, mode.vrefresh);

    /* ---------- Init GBM ---------- */
    struct gbm_device *gbm = gbm_create_device(fd);
    if (!gbm) {
        fprintf(stderr, "gbm_create_device failed\n");
        goto out_drm;
    }

    struct gbm_surface *gbm_surf = gbm_surface_create(
        gbm, width, height, GBM_FORMAT_XRGB8888,
        GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
    if (!gbm_surf) {
        fprintf(stderr, "gbm_surface_create failed\n");
        goto out_gbm;
    }

    /* ---------- Init EGL ---------- */
    EGLDisplay dpy = get_egl_display(gbm);
    if (dpy == EGL_NO_DISPLAY || !eglInitialize(dpy, NULL, NULL)) {
        fprintf(stderr, "EGL init failed\n");
        goto out_gbm_surf;
    }
    eglBindAPI(EGL_OPENGL_ES_API);

    EGLConfig config;
    if (!choose_config(dpy, &config, GBM_FORMAT_XRGB8888)) {
        fprintf(stderr, "No matching EGLConfig for XRGB8888\n");
        goto out_egl;
    }

    static const EGLint ctx_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    EGLContext ctx = eglCreateContext(dpy, config, EGL_NO_CONTEXT, ctx_attribs);
    if (ctx == EGL_NO_CONTEXT) {
        fprintf(stderr, "eglCreateContext failed\n");
        goto out_egl;
    }

    EGLSurface surf = eglCreateWindowSurface(dpy, config,
                                             (EGLNativeWindowType)gbm_surf, NULL);
    if (surf == EGL_NO_SURFACE) {
        fprintf(stderr, "eglCreateWindowSurface failed\n");
        goto out_ctx;
    }

    if (!eglMakeCurrent(dpy, surf, surf, ctx)) {
        fprintf(stderr, "eglMakeCurrent failed\n");
        goto out_surf;
    }
    eglSwapInterval(dpy, 0);   /* we pace ourselves with page flips */

    /* ---------- Your library ---------- */
    GLuint program = createProgram();
    if (!program)
        goto out_surf;

    /* ---------- Render loop ---------- */
    drmEventContext evctx = {
        .version = 2,
        .page_flip_handler = page_flip_handler,
    };

    struct gbm_bo *bo = NULL, *prev_bo = NULL;
    bool first = true;

    while (running) {
        drawSquare(program, width, height);
        eglSwapBuffers(dpy, surf);

        bo = gbm_surface_lock_front_buffer(gbm_surf);
        if (!bo) {
            fprintf(stderr, "gbm_surface_lock_front_buffer failed\n");
            break;
        }
        struct fb *fb = fb_get(fd, bo);
        if (!fb)
            break;

        if (first) {
            /* First frame: set the mode */
            if (drmModeSetCrtc(fd, crtc_id, fb->id, 0, 0,
                               &conn->connector_id, 1, &mode)) {
                fprintf(stderr, "drmModeSetCrtc failed: %s\n", strerror(errno));
                break;
            }
            first = false;
        } else {
            int waiting = 1;
            if (drmModePageFlip(fd, crtc_id, fb->id,
                                DRM_MODE_PAGE_FLIP_EVENT, &waiting)) {
                fprintf(stderr, "drmModePageFlip failed: %s\n", strerror(errno));
                break;
            }
            /* Wait for vblank / flip completion */
            while (waiting && running) {
                struct pollfd pfd = { .fd = fd, .events = POLLIN };
                int r = poll(&pfd, 1, -1);
                if (r < 0) {
                    if (errno == EINTR)
                        continue;
                    break;
                }
                if (pfd.revents & POLLIN)
                    drmHandleEvent(fd, &evctx);
            }
        }

        /* Previous buffer is no longer on screen: give it back to GBM */
        if (prev_bo)
            gbm_surface_release_buffer(gbm_surf, prev_bo);
        prev_bo = bo;
    }
    ret = 0;

    /* ---------- Cleanup ---------- */
    glDeleteProgram(program);

    /* Restore the original CRTC configuration */
    if (saved_crtc) {
        drmModeSetCrtc(fd, saved_crtc->crtc_id, saved_crtc->buffer_id,
                       saved_crtc->x, saved_crtc->y,
                       &conn->connector_id, 1, &saved_crtc->mode);
    }
    if (prev_bo)
        gbm_surface_release_buffer(gbm_surf, prev_bo);

out_surf:
    eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface(dpy, surf);
out_ctx:
    eglDestroyContext(dpy, ctx);
out_egl:
    eglTerminate(dpy);
out_gbm_surf:
    gbm_surface_destroy(gbm_surf);
out_gbm:
    gbm_device_destroy(gbm);
out_drm:
    if (saved_crtc)
        drmModeFreeCrtc(saved_crtc);
    drmModeFreeConnector(conn);
    drmModeFreeResources(res);
    close(fd);
    return ret;
}