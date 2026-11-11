#include "framebuffer_init.h"
#include <sys/ioctl.h>
#include <stdint.h>      // uint32_t, uint64_t
#include <stddef.h>      // size_t
#include <stdio.h>       // perror()
#include <unistd.h>      // close()
#include <sys/ioctl.h>   // ioctl()
#include <sys/mman.h>    // mmap(), munmap(), PROT_*, MAP_*
#include <linux/fb.h>    // fb_var_screeninfo, fb_fix_screeninfo, FBIOGET_*
#include <fcntl.h>       // open(), O_RDWR (if opening framebuffer here)

int framebuffer_init(
    int fb,
    uint32_t *width,
    uint32_t *height,
    uint32_t *pitch,
    uint32_t *bpp,
    uint64_t *size,
    void **map
){

    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;

    if (ioctl(fb, FBIOGET_VSCREENINFO, &vinfo) < 0 ||
        ioctl(fb, FBIOGET_FSCREENINFO, &finfo) < 0) {
        perror("framebuffer ioctl");
        return 0;
    }

    *width  = vinfo.xres;
    *height = vinfo.yres;
    *bpp    = vinfo.bits_per_pixel;
    *pitch  = finfo.line_length;   // bytes per row (was never set)

    // Total mapped size: use the virtual height in case of double buffering
    *size = (uint64_t)finfo.line_length * vinfo.yres_virtual;
    
    *map = mmap(NULL,*size,PROT_READ | PROT_WRITE,MAP_SHARED,fb,0);

    if (*map == MAP_FAILED) {
        perror("framebuffer mmap");
        *map = NULL;
        return 1;
    }

    return 1;
}

void framebuffer_cleanup(
    int *fb_fd,
    void **map,
    uint64_t size
) {
    if (map && *map && *map != MAP_FAILED) {
        munmap(*map, size);
        *map = NULL;
    }

    if (fb_fd && *fb_fd >= 0) {
        close(*fb_fd);
        *fb_fd = -1;
    }
}