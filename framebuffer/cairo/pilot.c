
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <cairo/cairo.h>

int main(void)
{
    int fb = open("/dev/fb0", O_RDWR);
    if (fb < 0) {
        perror("open framebuffer");
        return 1;
    }

    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;

    if (ioctl(fb, FBIOGET_VSCREENINFO, &vinfo) < 0 ||
        ioctl(fb, FBIOGET_FSCREENINFO, &finfo) < 0) {
        perror("framebuffer ioctl");
        close(fb);
        return 1;
    }

    int width  = vinfo.xres;
    int height = vinfo.yres;
    int bpp    = vinfo.bits_per_pixel;

    printf("Framebuffer: %dx%d, %d bpp\n",
           width, height, bpp);

    if (bpp != 32) {
        fprintf(stderr, "This example requires 32 bpp\n");
        close(fb);
        return 1;
    }

    size_t fb_size = finfo.smem_len;

    uint8_t *fbmem = mmap(NULL, fb_size,
                          PROT_READ | PROT_WRITE,
                          MAP_SHARED, fb, 0);

    if (fbmem == MAP_FAILED) {
        perror("mmap");
        close(fb);
        return 1;
    }

    // Cairo ARGB32 image buffer
    cairo_surface_t *surface =
        cairo_image_surface_create(
            CAIRO_FORMAT_ARGB32, width, height);

    cairo_t *cr = cairo_create(surface);

    // Background
    cairo_set_source_rgb(cr, 0.08, 0.10, 0.15);
    cairo_paint(cr);

    // Draw a rectangle
    cairo_set_source_rgb(cr, 0.1, 0.5, 0.9);
    cairo_rectangle(cr, 50, 50, 300, 150);
    cairo_fill(cr);

    // Draw a circle
    cairo_set_source_rgb(cr, 1.0, 0.3, 0.2);
    cairo_arc(cr, 250, 300, 80, 0, 2 * 3.14159265);
    cairo_fill(cr);

    // Draw text
    cairo_select_font_face(
        cr, "Sans",
        CAIRO_FONT_SLANT_NORMAL,
        CAIRO_FONT_WEIGHT_BOLD);

    cairo_set_font_size(cr, 40);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_move_to(cr, 50, 450);
    cairo_show_text(cr, "Hello Framebuffer!");

    cairo_surface_flush(surface);

    // Copy Cairo pixels to framebuffer.
    unsigned char *src =
        cairo_image_surface_get_data(surface);

    int src_stride =
        cairo_image_surface_get_stride(surface);

    int dst_stride = finfo.line_length;

    int bytes_per_pixel = bpp / 8;

    for (int y = 0; y < height; y++) {
        uint32_t *src_row =
            (uint32_t *)(src + y * src_stride);

        uint8_t *dst_row =
            fbmem + (y + vinfo.yoffset) * dst_stride
            + vinfo.xoffset * bytes_per_pixel;

        for (int x = 0; x < width; x++) {
            uint32_t pixel = src_row[x];

            // Cairo ARGB32 is native-endian.
            // This assumes framebuffer BGRA byte order.
            dst_row[x * 4 + 0] = (pixel >> 0) & 0xff;
            dst_row[x * 4 + 1] = (pixel >> 8) & 0xff;
            dst_row[x * 4 + 2] = (pixel >> 16) & 0xff;
            dst_row[x * 4 + 3] = (pixel >> 24) & 0xff;
        }
    }

    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    munmap(fbmem, fb_size);
    close(fb);

    return 0;
}
