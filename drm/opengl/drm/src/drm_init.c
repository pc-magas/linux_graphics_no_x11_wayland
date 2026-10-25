#include "drm_init.h"

drmModeConnector *getDRMConnector(int drm_fd)
{
    drmModeRes *resources = drmModeGetResources(drm_fd);

    if (!resources){
        return NULL;
    }

    drmModeConnector *connector = NULL;

    for (int i = 0; i < resources->count_connectors; i++) {
        drmModeConnector *candidate = drmModeGetConnector(
            drm_fd,
            resources->connectors[i]
        );

        if (!candidate){
            continue;
        }

        if (candidate->connection == DRM_MODE_CONNECTED &&
            candidate->count_modes > 0) {
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



int displayRefresh(int drm_fd, struct gbm_surface *gbm_surf, uint32_t crtc_id, uint32_t conn_id,drmModeModeInfo *mode, DisplayFrame *current) {

    // 1. Lock the new front buffer
    struct gbm_bo *bo = gbm_surface_lock_front_buffer(gbm_surf);

    if (!bo) {
        return -1;
    }

    // 2. Get buffer information
    uint32_t handle = gbm_bo_get_handle(bo).u32;
    uint32_t pitch = gbm_bo_get_stride(bo);
    uint32_t width = gbm_bo_get_width(bo);
    uint32_t height = gbm_bo_get_height(bo);

    uint32_t fb_id = 0;

    // 3. Create DRM framebuffer
    int ret = drmModeAddFB(drm_fd,width,height,24,32,pitch,handle,&fb_id);

    if (ret != 0) {
        perror("drmModeAddFB");
        gbm_surface_release_buffer(gbm_surf, bo);
        return -1;
    }

    // 4. Display the framebuffer
    ret = drmModeSetCrtc(drm_fd,crtc_id,fb_id,0,0,&conn_id,1,mode);

    if (ret != 0) {
        perror("drmModeSetCrtc");
        drmModeRmFB(drm_fd, fb_id);
        gbm_surface_release_buffer(gbm_surf, bo);
        return -1;
    }

    // 5. Release the previous framebuffer
    if (current->bo) {
        drmModeRmFB(drm_fd, current->fb_id);
        gbm_surface_release_buffer(gbm_surf,current->bo);
    }

    // 6. Remember the currently displayed buffer
    current->bo = bo;
    current->fb_id = fb_id;

    return 0;
}
