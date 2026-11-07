#include "drm_init.h"

drmModeConnector *getDRMConnector(int drm_fd)
{
    drmModeRes *resources = drmModeGetResources(drm_fd);

    if (!resources){
        return NULL;
    }

    drmModeConnector *connector = NULL;

    for (int i = 0; i < resources->count_connectors; i++) {
        drmModeConnector *candidate = drmModeGetConnector(drm_fd,resources->connectors[i]);

        if (!candidate){
            continue;
        }

        if (candidate->connection == DRM_MODE_CONNECTED && candidate->count_modes > 0) {
            connector = candidate;
            break;
        }

        drmModeFreeConnector(candidate);
    }

    drmModeFreeResources(resources);

    return connector;
}


drmModeCrtc *getDRMCrtc( int drm_fd, drmModeConnector *connector )
{
    if (!connector)
        return NULL;

    drmModeRes *resources = drmModeGetResources(drm_fd);

    if (!resources)
        return NULL;

    drmModeCrtc *crtc = NULL;

    for (int i = 0; i < connector->count_encoders && !crtc; i++) {
        drmModeEncoder *encoder = drmModeGetEncoder(
            drm_fd,
            connector->encoders[i]
        );

        if (!encoder)
            continue;

        for (int j = 0; j < resources->count_crtcs; j++) {
            if (encoder->possible_crtcs & (1u << j)) {
                crtc = drmModeGetCrtc(
                    drm_fd,
                    resources->crtcs[j]
                );

                if (crtc)
                    break;
            }
        }

        drmModeFreeEncoder(encoder);
    }

    drmModeFreeResources(resources);

    return crtc;
}


static int create_buffer(struct drm_state *s)
{
    struct drm_mode_create_dumb creq = {0};

    creq.width = s->width;
    creq.height = s->height;
    creq.bpp = 32;

    if (drmIoctl(s->fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq))
        return -1;

    s->handle = creq.handle;
    s->pitch = creq.pitch;
    s->size = creq.size;

    if (drmModeAddFB(s->fd, s->width, s->height,
                     24, 32, s->pitch,
                     s->handle, &s->fb_id))
        return -1;

    struct drm_mode_map_dumb mreq = {0};
    mreq.handle = s->handle;

    if (drmIoctl(s->fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq))
        return -1;

    s->map = mmap(NULL, s->size,
                  PROT_READ | PROT_WRITE,
                  MAP_SHARED, s->fd, mreq.offset);

    if (s->map == MAP_FAILED) {
        s->map = NULL;
        return -1;
    }

    return 0;
}