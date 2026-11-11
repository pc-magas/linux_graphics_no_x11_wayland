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
    int fb = open("/dev/fb0", O_RDWR|O_CLOEXEC);

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

    cairo_format_t fmt = (bpp == 32) ? CAIRO_FORMAT_RGB24 : CAIRO_FORMAT_RGB16_565;

    // Off-screen back buffer (normal heap memory, not the framebuffer)
    cairo_surface_t *back = cairo_image_surface_create(fmt, width, height);
    if (cairo_surface_status(back) != CAIRO_STATUS_SUCCESS) {
        fprintf(stderr, "Back buffer creation failed\n");
        framebuffer_cleanup(&fb, &map, size);
        return 1;
    }

    cairo_t *cr = cairo_create(back);

    while (true) {
        draw_with_cairo(cairo);
        cairo_surface_flush(surface);

        uint32_t dummy = 0;
        ioctl(fb, FBIO_WAITFORVSYNC, &dummy);

        // Copy row by row, since the strides can differ
        for (uint32_t y = 0; y < height; y++) {
            memcpy((uint8_t *)map + (size_t)y * pitch,src + (size_t)y * src_stride,row_bytes);
        }
    }

    cairo_destroy(cairo);
    cairo_surface_destroy(surface);

    framebuffer_cleanup(&fb, &map, size);

    return 0;
}