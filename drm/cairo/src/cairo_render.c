

#include "cairo_render.h";

cairo_t* init_cairo(void *map, uint32_t width,uint32_t height,uint32_t pitch, cairo_surface_t **surface ){
   *surface =cairo_image_surface_create_for_data(map,CAIRO_FORMAT_ARGB32,width,height,pitch);
   return cairo_create(surface);
}

void draw_with_cairo  (cairo_t *cr) {

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
}