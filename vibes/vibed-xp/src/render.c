#include "server.h"
#include <cairo.h>
#include <pango/pangocairo.h>
#include <math.h>

static void draw_xp_titlebar(cairo_t *cr, int width, bool active) {
    cairo_pattern_t *grad = cairo_pattern_create_linear(0, 0, 0, 26);
    if (active) {
        cairo_pattern_add_color_stop_rgb(grad, 0.0, 0.0, 0.33, 0.89); // #0055E3
        cairo_pattern_add_color_stop_rgb(grad, 1.0, 0.0, 0.14, 0.42); // #00246A
    } else {
        cairo_pattern_add_color_stop_rgb(grad, 0.0, 0.5, 0.5, 0.5);
        cairo_pattern_add_color_stop_rgb(grad, 1.0, 0.3, 0.3, 0.3);
    }
    cairo_set_source(cr, grad);
    cairo_rectangle(cr, 1, 1, width - 2, 25);
    cairo_fill(cr);
    cairo_pattern_destroy(grad);

    PangoLayout *layout = pango_cairo_create_layout(cr);
    PangoFontDescription *desc = pango_font_description_from_string("Tahoma Bold 8");
    pango_layout_set_font_description(layout, desc);
    pango_layout_set_text(layout, "Untitled - XP App", -1);
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_move_to(cr, 8, 7);
    pango_cairo_show_layout(cr, layout);
    g_object_unref(layout);
    pango_font_description_free(desc);
}

static void draw_xp_window_border(cairo_t *cr, int width, int height) {
    cairo_set_source_rgb(cr, 0.04, 0.14, 0.41);
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_stroke(cr);
    cairo_set_source_rgb(cr, 0.83, 0.81, 0.78);
    cairo_rectangle(cr, 2, 2, width - 4, height - 4);
    cairo_stroke(cr);
}

void render_xp_decoration(struct wlr_scene_buffer *buffer, int width, int height, bool active) {
    int stride = width * 4;
    size_t size = stride * height;
    uint8_t *data = calloc(1, size);

    cairo_surface_t *surf = cairo_image_surface_create_for_data(data, CAIRO_FORMAT_ARGB32, width, height, stride);
    cairo_t *cr = cairo_create(surf);

    cairo_set_source_rgba(cr, 1, 1, 1, 0);
    cairo_paint(cr);

    draw_xp_window_border(cr, width, height);
    draw_xp_titlebar(cr, width, active);

    // Close button
    cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
    cairo_rectangle(cr, width - 20, 5, 14, 14);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_move_to(cr, width - 17, 8);
    cairo_line_to(cr, width - 9, 16);
    cairo_move_to(cr, width - 9, 8);
    cairo_line_to(cr, width - 17, 16);
    cairo_stroke(cr);

    wlr_buffer_unlock(buffer->buffer);
    struct wlr_buffer *shm_buf = malloc(sizeof(struct wlr_buffer));
    shm_buf->width = width;
    shm_buf->height = height;
    // Note: In production, wrap data in wlr_shm_pool. This is a simplified placeholder.
    // For a real build, use wlr_buffer_from_dmabuf or SHM pools properly.
    free(data);
    cairo_destroy(cr);
    cairo_surface_destroy(surf);
}
