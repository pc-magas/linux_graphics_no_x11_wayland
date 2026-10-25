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
