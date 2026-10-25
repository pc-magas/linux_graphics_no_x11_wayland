#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <poll.h>

#include <gbm.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#include "nextframe.h"

static int createFramebuffer(int drm_fd, struct gbm_bo *bo, uint32_t *fb_id)
{
    uint32_t width = gbm_bo_get_width(bo);
    uint32_t height = gbm_bo_get_height(bo);
    uint32_t pitch = gbm_bo_get_stride(bo);
    uint32_t handle = gbm_bo_get_handle(bo).u32;

    *fb_id = 0;

    if (drmModeAddFB(drm_fd,width,height,24,32,pitch,handle,fb_id) != 0) {
        perror("drmModeAddFB");
        return -1;
    }

    return 0;
}


int displayFirstFrame(int drm_fd, struct gbm_surface *surface,uint32_t crtc_id,uint32_t conn_id,drmModeModeInfo *mode,DisplayFrame *current)
{
    struct gbm_bo *bo = gbm_surface_lock_front_buffer(surface);

    if (!bo){
        return -1;
    }

    uint32_t fb_id;

    if (createFramebuffer(drm_fd, bo, &fb_id) != 0) {
        gbm_surface_release_buffer(surface, bo);
        return -1;
    }

    if (drmModeSetCrtc(drm_fd, crtc_id, fb_id, 0, 0, &conn_id, 1, mode) != 0) {
        perror("drmModeSetCrtc");
        drmModeRmFB(drm_fd, fb_id);
        gbm_surface_release_buffer(surface, bo);
        return -1;
    }

    current->bo = bo;
    current->fb_id = fb_id;

    return 0;
}

static void pageFlipHandler(int fd, unsigned int sequence,unsigned int tv_sec,unsigned int tv_usec,void *user_data) {
    bool *waiting = user_data;
    *waiting = false;
}

int displayNextFrame(int drm_fd, struct gbm_surface *surface, uint32_t crtc_id, DisplayFrame *current)
{
    struct gbm_bo *bo = gbm_surface_lock_front_buffer(surface);

    if (!bo){
        return -1;
    }

    uint32_t fb_id;

    if (createFramebuffer(drm_fd, bo, &fb_id) != 0) {
        gbm_surface_release_buffer(surface, bo);
        return -1;
    }

    bool waiting = true;

    int ret = drmModePageFlip(drm_fd,crtc_id,fb_id,DRM_MODE_PAGE_FLIP_EVENT,&waiting);

    if (ret != 0) {
        perror("drmModePageFlip");
        drmModeRmFB(drm_fd, fb_id);
        gbm_surface_release_buffer(surface, bo);
        return -1;
    }

    drmEventContext ev = {0};
    ev.version = DRM_EVENT_CONTEXT_VERSION;
    ev.page_flip_handler = pageFlipHandler;

    while (waiting) {
        struct pollfd pfd = {
            .fd = drm_fd,
            .events = POLLIN
        };

        ret = poll(&pfd, 1, -1);

        if (ret < 0) {
            if (errno == EINTR) continue;
            perror("poll");
            return -1;
        }

        if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) {
            fprintf(stderr, "DRM polling error\n");
            return -1;
        }

        if (pfd.revents & POLLIN) {
            ret = drmHandleEvent(drm_fd, &ev);

            if (ret != 0) {
                fprintf(stderr,"drmHandleEvent failed: %d\n", ret);
                return -1;
            }
        }
    }

    // Previous buffer is no longer being scanned out.
    if (current->bo) {
        drmModeRmFB(drm_fd, current->fb_id);
        gbm_surface_release_buffer(surface, current->bo);
    }

    current->bo = bo;
    current->fb_id = fb_id;

    return 0;
}