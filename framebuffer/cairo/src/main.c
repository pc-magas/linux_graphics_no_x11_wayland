#define _GNU_SOURCE

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>

#include <cairo.h>

#include "framebuffer_init.h"
#include "cairo_render.h"

int main(void)
{
    int fb = open("/dev/fb0", O_RDWR);

    if (fb < 0) {
        perror("open framebuffer");
        return 1;
    }

    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t pitch = 0;
    uint32_t bpp = 0;
    uint64_t size = 0;
    void *map = NULL;

    if (!framebuffer_init(fb,&width,&height,&pitch,&bpp,&size,&map)) {

        fprintf(stderr, "Framebuffer initialization failed\n");
        framebuffer_cleanup(&fb, &map, size);

        return 1;
    }

    // Create Cairo surface over framebuffer memory
    cairo_surface_t *surface = cairo_image_surface_create_for_data(
        map,
        bpp == 32 ? CAIRO_FORMAT_ARGB32 : CAIRO_FORMAT_RGB16_565,
        width,
        height,
        pitch
    );

    if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
        fprintf(stderr, "Cairo surface creation failed\n");
        cairo_surface_destroy(surface);
        framebuffer_cleanup(&fb, &map, size);
        return 1;
    }

    cairo_t *cairo = cairo_create(surface);

    while (true) {
        draw_with_cairo(cairo);
        cairo_surface_flush(surface);
    }

    cairo_destroy(cairo);
    cairo_surface_destroy(surface);

    framebuffer_cleanup(&fb, &map, size);

    return 0;
}