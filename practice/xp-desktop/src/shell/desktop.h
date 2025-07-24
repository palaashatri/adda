#ifndef XP_DESKTOP_H
#define XP_DESKTOP_H

#include <wayland-server-core.h>
#include <wlr/types/wlr_scene.h>
#include <cairo.h>

// Desktop background with XP Bliss theme
struct xp_desktop {
    struct wlr_scene_tree *tree;
    struct wlr_scene_buffer *background;
    int width, height;
};

// Create XP desktop background
static struct xp_desktop *create_xp_desktop(struct xp_server *server, int width, int height) {
    struct xp_desktop *desktop = calloc(1, sizeof(struct xp_desktop));
    desktop->width = width;
    desktop->height = height;
    
    desktop->tree = wlr_scene_tree_create(server->scene_tree);
    
    // Create XP Bliss-style background (simplified)
    struct wlr_buffer *buffer = create_xp_background_buffer(width, height);
    desktop->background = wlr_scene_buffer_create(desktop->tree, buffer);
    wlr_buffer_drop(buffer);
    
    return desktop;
}

// Create XP background buffer
static struct wlr_buffer *create_xp_background_buffer(int width, int height) {
    // Create pixel buffer
    size_t stride = width * 4;
    size_t size = stride * height;
    uint32_t *pixels = calloc(1, size);
    
    // Draw XP Bliss background (simplified)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Sky gradient (top)
            if (y < height * 0.6) {
                float ratio = (float)y / (height * 0.6);
                uint8_t r = 102 + (212 - 102) * ratio;
                uint8_t g = 170 + (238 - 170) * ratio;
                uint8_t b = 255 + (255 - 255) * ratio;
                pixels[y * width + x] = (0xFF << 24) | (b << 16) | (g << 8) | r;
            } else {
                // Ground gradient (bottom)
                float ratio = (float)(y - height * 0.6) / (height * 0.4);
                uint8_t r = 153 + (77 - 153) * ratio;
                uint8_t g = 204 + (128 - 204) * ratio;
                uint8_t b = 102 + (51 - 102) * ratio;
                pixels[y * width + x] = (0xFF << 24) | (b << 16) | (g << 8) | r;
            }
        }
    }
    
    // Create wlr_buffer from pixels
    struct wlr_shm_attributes attrs = {
        .fd = -1, // We'll create this properly in actual implementation
        .format = WL_SHM_FORMAT_ARGB8888,
        .width = width,
        .height = height,
        .stride = stride,
        .size = size
    };
    
    // This is a simplified version - actual implementation would create proper SHM buffer
    free(pixels);
    return NULL; // Placeholder
}

#endif