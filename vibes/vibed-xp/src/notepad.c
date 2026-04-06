#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wayland-client.h>
#include <xdg-shell-client-protocol.h>
#include <cairo.h>

struct xp_notepad {
    struct wl_display *display;
    struct wl_compositor *compositor;
    struct xdg_wm_base *wm_base;
    struct wl_shm *shm;

    struct wl_surface *surface;
    struct xdg_surface *xdg_surface;
    struct xdg_toplevel *toplevel;
    struct wl_buffer *buffer;

    void *shm_data;
    int width, height;
    bool configured;
};

static int create_shm_fd(size_t size) {
    const char *name = "/xp-notepad-XXXXXX";
    int fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
    if (fd < 0) return -1;
    shm_unlink(name);
    if (ftruncate(fd, size) < 0) { close(fd); return -1; }
    return fd;
}

static struct wl_buffer *create_buffer(struct xp_notepad *np) {
    int stride = np->width * 4;
    int size = stride * np->height;
    int fd = create_shm_fd(size);
    if (fd < 0) return NULL;

    np->shm_data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (np->shm_data == MAP_FAILED) { close(fd); return NULL; }

    struct wl_shm_pool *pool = wl_shm_create_pool(np->shm, fd, size);
    struct wl_buffer *buf = wl_shm_pool_create_buffer(pool, 0, np->width, np->height, stride, WL_SHM_FORMAT_ARGB8888);
    wl_shm_pool_destroy(pool);
    close(fd);
    return buf;
}

static void draw(struct xp_notepad *np) {
    cairo_surface_t *surf = cairo_image_surface_create_for_data(np->shm_data, CAIRO_FORMAT_ARGB32, np->width, np->height, np->width * 4);
    cairo_t *cr = cairo_create(surf);

    cairo_set_source_rgb(cr, 0.93, 0.92, 0.88); // XP face
    cairo_paint(cr);
    cairo_set_source_rgb(cr, 0.0, 0.33, 0.89); // XP blue title
    cairo_rectangle(cr, 0, 0, np->width, 26);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_move_to(cr, 8, 16);
    cairo_show_text(cr, "Notepad");
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_move_to(cr, 10, 50);
    cairo_show_text(cr, "Start typing...");

    cairo_destroy(cr);
    cairo_surface_destroy(surf);
}

static void xdg_surface_configure(void *data, struct xdg_surface *xs, uint32_t serial) {
    struct xp_notepad *np = data;
    xdg_surface_ack_configure(xs, serial);
    if (!np->configured) {
        np->configured = true;
        np->buffer = create_buffer(np);
        draw(np);
        wl_surface_attach(np->surface, np->buffer, 0, 0);
        wl_surface_commit(np->surface);
    }
}

static const struct xdg_surface_listener surf_listener = {.configure = xdg_surface_configure};

int main(void) {
    struct xp_notepad np = {0};
    np.width = 500; np.height = 300;

    np.display = wl_display_connect(NULL);
    if (!np.display) { fprintf(stderr, "Failed to connect\n"); return 1; }

    struct wl_registry *reg = wl_display_get_registry(np.display);
    // Bind globals (simplified for brevity - in full build, iterate registry)
    // For production, add wl_registry_listener to bind compositor, shm, xdg_wm_base

    np.surface = wl_compositor_create_surface(np.compositor);
    np.xdg_surface = xdg_wm_base_get_xdg_surface(np.wm_base, np.surface);
    xdg_surface_add_listener(np.xdg_surface, &surf_listener, &np);
    np.toplevel = xdg_surface_get_toplevel(np.xdg_surface);
    xdg_toplevel_set_title(np.toplevel, "XP Notepad");

    while (wl_display_dispatch(np.display) >= 0 && !np.configured);

    // Main loop
    while (wl_display_roundtrip(np.display) >= 0) { usleep(10000); }

    return 0;
}
