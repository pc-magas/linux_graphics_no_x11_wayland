
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm.h>
#include <drm_mode.h>
#include <cairo.h>

struct drm_state {
    int fd;
    uint32_t conn_id;
    uint32_t crtc_id;
    uint32_t fb_id;
    uint32_t handle;
    uint32_t pitch;
    uint64_t size;
    uint32_t width, height;
    void *map;
    drmModeModeInfo mode;
    drmModeCrtc *saved_crtc;
};

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

static int find_display(struct drm_state *s)
{
    drmModeRes *res = drmModeGetResources(s->fd);
    if (!res)
        return -1;

    int found = 0;

    for (int i = 0; i < res->count_connectors && !found; i++) {
        drmModeConnector *conn =
            drmModeGetConnector(s->fd, res->connectors[i]);

        if (!conn)
            continue;

        if (conn->connection != DRM_MODE_CONNECTED ||
            conn->count_modes == 0) {
            drmModeFreeConnector(conn);
            continue;
        }

        s->conn_id = conn->connector_id;
        s->mode = conn->modes[0];

        for (int j = 0; j < conn->count_modes; j++) {
            if (conn->modes[j].type & DRM_MODE_TYPE_PREFERRED) {
                s->mode = conn->modes[j];
                break;
            }
        }

        drmModeEncoder *enc = NULL;

        if (conn->encoder_id)
            enc = drmModeGetEncoder(s->fd, conn->encoder_id);

        if (!enc && conn->count_encoders > 0)
            enc = drmModeGetEncoder(s->fd, conn->encoders[0]);

        if (enc) {
            int crtc_index = -1;

            if (enc->crtc_id) {
                for (int k = 0; k < res->count_crtcs; k++) {
                    if (res->crtcs[k] == enc->crtc_id) {
                        crtc_index = k;
                        break;
                    }
                }
            }

            if (crtc_index < 0) {
                for (int k = 0; k < res->count_crtcs; k++) {
                    if (enc->possible_crtcs & (1u << k)) {
                        crtc_index = k;
                        break;
                    }
                }
            }

            if (crtc_index >= 0) {
                s->crtc_id = res->crtcs[crtc_index];
                found = 1;
            }

            drmModeFreeEncoder(enc);
        }

        drmModeFreeConnector(conn);
    }

    drmModeFreeResources(res);

    if (!found)
        return -1;

    s->width = s->mode.hdisplay;
    s->height = s->mode.vdisplay;

    return 0;
}

static void draw_with_cairo(struct drm_state *s)
{
    cairo_surface_t *surface =
        cairo_image_surface_create_for_data(
            s->map,
            CAIRO_FORMAT_ARGB32,
            s->width,
            s->height,
            s->pitch);

    cairo_t *cr = cairo_create(surface);

    // Background
    cairo_set_source_rgb(cr, 0.05, 0.08, 0.16);
    cairo_paint(cr);

    // Rectangle
    cairo_set_source_rgb(cr, 0.1, 0.6, 0.9);
    cairo_rectangle(cr, 100, 100, 400, 200);
    cairo_fill(cr);

    // Circle
    cairo_set_source_rgb(cr, 1.0, 0.3, 0.2);
    cairo_arc(cr, 600, 300, 100, 0, 2 * 3.14159265);
    cairo_fill(cr);

    // Text
    cairo_select_font_face(cr, "Sans",
        CAIRO_FONT_SLANT_NORMAL,
        CAIRO_FONT_WEIGHT_BOLD);

    cairo_set_font_size(cr, 48);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_move_to(cr, 100, 450);
    cairo_show_text(cr, "Cairo + DRM/KMS");

    cairo_destroy(cr);
    cairo_surface_flush(surface);
    cairo_surface_destroy(surface);
}

int main(void)
{
    struct drm_state s = {0};
    s.map = NULL;

    s.fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
    if (s.fd < 0) {
        perror("open DRM");
        return 1;
    }

    if (find_display(&s) < 0) {
        fprintf(stderr, "No suitable connected display/CRTC\n");
        close(s.fd);
        return 1;
    }

    printf("Display: %ux%u\n", s.width, s.height);

    s.saved_crtc = drmModeGetCrtc(s.fd, s.crtc_id);

    if (create_buffer(&s) < 0) {
        perror("Create DRM buffer");
        goto cleanup;
    }

    draw_with_cairo(&s);

    if (drmModeSetCrtc(s.fd,
                       s.crtc_id,
                       s.fb_id,
                       0, 0,
                       &s.conn_id, 1,
                       &s.mode) < 0) {
        perror("drmModeSetCrtc");
        goto cleanup;
    }

    printf("Cairo image displayed. Press Enter to exit.\n");
    getchar();

cleanup:

    if (s.saved_crtc) {
        drmModeSetCrtc(s.fd,
                       s.saved_crtc->crtc_id,
                       s.saved_crtc->buffer_id,
                       s.saved_crtc->x,
                       s.saved_crtc->y,
                       &s.conn_id, 1,
                       &s.saved_crtc->mode);
        drmModeFreeCrtc(s.saved_crtc);
    }

    if (s.map)
        munmap(s.map, s.size);

    if (s.fb_id)
        drmModeRmFB(s.fd, s.fb_id);

    if (s.handle) {
        struct drm_mode_destroy_dumb dreq = {0};
        dreq.handle = s.handle;
        drmIoctl(s.fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
    }

    close(s.fd);
    return 0;
}
