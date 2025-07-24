#include <wayland-client.h>
#include <wayland-client-protocol.h>
#include <xdg-shell-client-protocol.h>
#include <cairo.h>
#include <pango/pangocairo.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

// Simple XP-style Notepad application (pure Wayland)

struct xp_notepad {
    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_compositor *compositor;
    struct xdg_wm_base *xdg_wm_base;
    struct wl_shm *shm;
    
    struct wl_surface *surface;
    struct xdg_surface *xdg_surface;
    struct xdg_toplevel *xdg_toplevel;
    
    struct wl_buffer *buffer;
    int width, height;
    void *shm_data;
    
    // Text content
    char *text_content;
    size_t text_length;
    size_t text_capacity;
    
    // UI state
    int cursor_x, cursor_y;
    bool configured;
};

static void draw_notepad(struct xp_notepad *notepad) {
    // Create cairo surface from SHM data
    cairo_surface_t *surface = cairo_image_surface_create_for_data(
        notepad->shm_data,
        CAIRO_FORMAT_ARGB32,
        notepad->width,
        notepad->height,
        notepad->width * 4
    );
    
    cairo_t *cr = cairo_create(surface);
    
    // Clear with XP face color
    cairo_set_source_rgb(cr, 0.93, 0.92, 0.88); // #ECE9D8
    cairo_paint(cr);
    
    // Draw XP-style border
    cairo_set_source_rgb(cr, 0.04, 0.14, 0.41); // #0A246A
    cairo_set_line_width(cr, 1);
    cairo_rectangle(cr, 0, 0, notepad->width, notepad->height);
    cairo_stroke(cr);
    
    // Draw titlebar
    cairo_pattern_t *pattern = cairo_pattern_create_linear(0, 0, 0, 24);
    cairo_pattern_add_color_stop_rgb(pattern, 0, 0.0, 0.33, 0.89); // #0055E3
    cairo_pattern_add_color_stop_rgb(pattern, 1, 0.0, 0.14, 0.42); // #00246A
    cairo_rectangle(cr, 1, 1, notepad->width - 2, 24);
    cairo_set_source(cr, pattern);
    cairo_fill(cr);
    cairo_pattern_destroy(pattern);
    
    // Draw title text
    PangoLayout *layout = pango_cairo_create_layout(cr);
    PangoFontDescription *desc = pango_font_description_from_string("Tahoma Bold 8");
    pango_layout_set_font_description(layout, desc);
    pango_layout_set_text(layout, "Untitled - Notepad", -1);
    
    cairo_set_source_rgb(cr, 1, 1, 1); // White text
    cairo_move_to(cr, 8, 6);
    pango_cairo_show_layout(cr, layout);
    
    // Draw menu bar placeholder
    cairo_set_source_rgb(cr, 0.83, 0.81, 0.78); // #D4D0C8
    cairo_rectangle(cr, 0, 25, notepad->width, 20);
    cairo_fill(cr);
    
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_move_to(cr, 5, 30);
    pango_layout_set_text(layout, "File Edit View Help", -1);
    pango_cairo_show_layout(cr, layout);
    
    // Draw text area
    cairo_set_source_rgb(cr, 1, 1, 1); // White background
    cairo_rectangle(cr, 2, 47, notepad->width - 4, notepad->height - 49);
    cairo_fill(cr);
    
    // Draw text content
    if (notepad->text_content && notepad->text_length > 0) {
        cairo_set_source_rgb(cr, 0, 0, 0); // Black text
        cairo_move_to(cr, 5, 50);
        pango_layout_set_text(layout, notepad->text_content, notepad->text_length);
        pango_cairo_show_layout(cr, layout);
    }
    
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    g_object_unref(layout);
    pango_font_description_free(desc);
}

static int create_shm_file(size_t size) {
    char template[] = "/tmp/xp-notepad-XXXXXX";
    int fd = mkstemp(template);
    if (fd < 0) return -1;
    
    unlink(template);
    if (ftruncate(fd, size) < 0) {
        close(fd);
        return -1;
    }
    
    return fd;
}

static struct wl_buffer *create_buffer(struct xp_notepad *notepad) {
    int stride = notepad->width * 4;
    int size = stride * notepad->height;
    
    int fd = create_shm_file(size);
    if (fd < 0) return NULL;
    
    void *data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        close(fd);
        return NULL;
    }
    
    struct wl_shm_pool *pool = wl_shm_create_pool(notepad->shm, fd, size);
    struct wl_buffer *buffer = wl_shm_pool_create_buffer(pool, 0, 
        notepad->width, notepad->height, stride, WL_SHM_FORMAT_ARGB8888);
    
    wl_shm_pool_destroy(pool);
    close(fd);
    
    notepad->shm_data = data;
    return buffer;
}

static void xdg_surface_configure(void *data, struct xdg_surface *xdg_surface, uint32_t serial) {
    struct xp_notepad *notepad = data;
    xdg_surface_ack_configure(xdg_surface, serial);
    
    if (!notepad->configured) {
        notepad->configured = true;
        notepad->buffer = create_buffer(notepad);
        draw_notepad(notepad);
        wl_surface_attach(notepad->surface, notepad->buffer, 0, 0);
        wl_surface_commit(notepad->surface);
    }
}

static void xdg_toplevel_configure(void *data, struct xdg_toplevel *xdg_toplevel,
    int32_t width, int32_t height, struct wl_array *states) {
    struct xp_notepad *notepad = data;
    if (width > 0 && height > 0) {
        notepad->width = width;
        notepad->height = height;
    }
}

static const struct xdg_surface_listener surface_listener = {
    .configure = xdg_surface_configure,
};

static const struct xdg_toplevel_listener toplevel_listener = {
    .configure = xdg_toplevel_configure,
};

int main(int argc, char *argv[]) {
    struct xp_notepad notepad = {0};
    notepad.width = 600;
    notepad.height = 400;
    notepad.text_content = strdup("");
    notepad.text_length = 0;
    notepad.text_capacity = 1024;
    
    notepad.display = wl_display_connect(NULL);
    if (!notepad.display) {
        fprintf(stderr, "Failed to connect to Wayland display\n");
        return 1;
    }
    
    notepad.registry = wl_display_get_registry(notepad.display);
    // Registry handling would go here (simplified)
    
    notepad.surface = wl_compositor_create_surface(notepad.compositor);
    notepad.xdg_surface = xdg_wm_base_get_xdg_surface(notepad.xdg_wm_base, notepad.surface);
    xdg_surface_add_listener(notepad.xdg_surface, &surface_listener, &notepad);
    
    notepad.xdg_toplevel = xdg_surface_get_toplevel(notepad.xdg_surface);
    xdg_toplevel_add_listener(notepad.xdg_toplevel, &toplevel_listener, &notepad);
    xdg_toplevel_set_title(notepad.xdg_toplevel, "XP Notepad");
    
    wl_surface_commit(notepad.surface);
    
    while (wl_display_dispatch(notepad.display) != -1) {
        // Main event loop
    }
    
    return 0;
}