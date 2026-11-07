#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <linux/kd.h>

#include "cairo_render.h"

static volatile sig_atomic_t running = 1;
static void on_signal(int sig) { (void)sig; running = 0; }

int main(void)
{
    int status_code = 0;

    int fb_fd = -1;
    int tty_fd = -1;
    uint8_t *map = NULL;
    size_t map_size = 0;
    cairo_surface_t *surface = NULL;
    cairo_t *cairo = NULL;

    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;

    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);

    fb_fd = open("/dev/fb0", O_RDWR | O_CLOEXEC);
    if (fb_fd < 0) {
        perror("open /dev/fb0");
        status_code = 1;
        goto cleanup;
    }

    if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo) < 0 ||
        ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo) < 0) {
        perror("FBIOGET_*SCREENINFO");
        status_code = 1;
        goto cleanup;
    }

    /* Cairo RGB24 = 32-bit XRGB (R at bit 16). Refuse anything else. */
    if (vinfo.bits_per_pixel != 32 || vinfo.red.offset != 16) {
        fprintf(stderr, "Unsupported format: %u bpp, red offset %u\n",
                vinfo.bits_per_pixel, vinfo.red.offset);
        status_code = 1;
        goto cleanup;
    }

    uint32_t width  = vinfo.xres;
    uint32_t height = vinfo.yres;
    uint32_t stride = finfo.line_length;       /* use this, not width * 4 */

    map_size = finfo.smem_len;
    map = mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
    if (map == MAP_FAILED) {
        perror("mmap");
        map = NULL;
        status_code = 1;
        goto cleanup;
    }

    /* Stop the text console from drawing over us (needs a real VT) */
    tty_fd = open("/dev/tty", O_RDWR | O_CLOEXEC);
    if (tty_fd >= 0)
        ioctl(tty_fd, KDSETMODE, KD_GRAPHICS);

    /* Skip the panned offset if the driver has one set */
    uint8_t *pixels = map + (size_t)vinfo.yoffset * stride
                          + (size_t)vinfo.xoffset * 4;

    cairo = init_cairo(pixels, width, height, stride, &surface);
    if (!cairo) {
        status_code = 1;
        goto cleanup;
    }

    draw_with_cairo(cairo);
    cairo_surface_flush(surface);

    while (running)
        pause();                  /* Ctrl+C to exit */

cleanup:
    if (tty_fd >= 0) {
        ioctl(tty_fd, KDSETMODE, KD_TEXT);
        close(tty_fd);
    }
    if (cairo)   cairo_destroy(cairo);
    if (surface) cairo_surface_destroy(surface);
    if (map)     munmap(map, map_size);
    if (fb_fd >= 0) close(fb_fd);

    return status_code;
}