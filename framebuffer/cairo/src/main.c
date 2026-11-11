#define _GNU_SOURCE

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/fb.h>

#include <cairo.h>

#include "framebuffer_init.h"
#include "cairo_render.h"

int main(void)
{
    int fb = open("/dev/fb0", O_RDWR | O_CLOEXEC);
    if (fb < 0) {
        perror("open framebuffer");
        return 1;
    }

    uint32_t width = 0, height = 0, pitch = 0, bpp = 0;
    uint64_t size = 0;
    void *map = NULL;

    if (!framebuffer_init(fb, &width, &height, &pitch, &bpp, &size, &map)) {
        fprintf(stderr, "Framebuffer initialization failed\n");
        framebuffer_cleanup(&fb, &map, size);
        return 1;
    }

    cairo_format_t fmt = (bpp == 32) ? CAIRO_FORMAT_RGB24
                                     : CAIRO_FORMAT_RGB16_565;

    // Back buffer in normal RAM, with exactly the same layout as the framebuffer
    size_t frame_bytes = (size_t)pitch * height;
    uint8_t *backbuf = calloc(1, frame_bytes);
    if (!backbuf) {
        fprintf(stderr, "Back buffer allocation failed\n");
        framebuffer_cleanup(&fb, &map, size);
        return 1;
    }

    cairo_surface_t *back = cairo_image_surface_create_for_data(
        backbuf, fmt, width, height, pitch);

    if (cairo_surface_status(back) != CAIRO_STATUS_SUCCESS) {
        fprintf(stderr, "Back buffer surface creation failed: %s\n",
                cairo_status_to_string(cairo_surface_status(back)));
        cairo_surface_destroy(back);
        free(backbuf);
        framebuffer_cleanup(&fb, &map, size);
        return 1;
    }

    cairo_t *cr = cairo_create(back);

    const struct timespec frame_delay = { 0, 16 * 1000 * 1000 }; // ~60 FPS

    while (true) {
        draw_with_cairo(cr);
        cairo_surface_flush(back);

        // Wait for vblank; if unsupported, fall back to a fixed delay
        uint32_t dummy = 0;
        if (ioctl(fb, FBIO_WAITFORVSYNC, &dummy) < 0) {
            nanosleep(&frame_delay, NULL);
        }

        // Single copy of the whole frame
        memcpy(map, backbuf, frame_bytes);
    }

    // Cleanup (reached if you add an exit condition to the loop)
    cairo_destroy(cr);
    cairo_surface_destroy(back);
    free(backbuf);
    framebuffer_cleanup(&fb, &map, size);

    return 0;
}