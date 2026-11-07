#ifndef CAIRO_RENDER_H
#define CAIRO_RENDER_H

#include <stdint.h>
#include <cairo.h>

cairo_t* init_cairo(void *map, uint32_t width,uint32_t height,uint32_t pitch, cairo_surface_t **surface );
void draw_with_cairo(cairo_t *cr);

#endif