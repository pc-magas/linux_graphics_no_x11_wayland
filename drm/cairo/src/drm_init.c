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

int displayInit( int drm_fd, uint32_t crtc_id, uint32_t connector_id, drmModeModeInfo *mode, uint32_t handle, uint32_t pitch, uint32_t *fb_id)
{
    int ret = drmModeAddFB(drm_fd,mode->hdisplay,mode->vdisplay,24,32,pitch,handle,fb_id);

    if (ret != 0) {
        perror("drmModeAddFB");
        return -1;
    }

    ret = drmModeSetCrtc(
        drm_fd,
        crtc_id,
        *fb_id,
        0,
        0,
        &connector_id,
        1,
        mode
    );

    if (ret != 0) {
        perror("drmModeSetCrtc");
        drmModeRmFB(drm_fd, *fb_id);
        *fb_id = 0;
        return -1;
    }

    return 0;
}