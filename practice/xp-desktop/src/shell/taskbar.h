#ifndef XP_TASKBAR_H
#define XP_TASKBAR_H

#include <wayland-server-core.h>
#include <wlr/types/wlr_scene.h>

struct xp_taskbar {
    struct wlr_scene_tree *tree;
    struct wlr_scene_rect *background;
    struct xp_start_button *start_button;
    struct xp_system_tray *system_tray;
    struct wl_list app_buttons;
    int width, height;
};

struct xp_start_button {
    struct wlr_scene_tree *tree;
    struct wlr_scene_rect *background;
    struct wlr_scene_buffer *icon;
    int x, y, width, height;
    bool pressed;
};

struct xp_app_button {
    struct wl_list link;
    struct wlr_scene_tree *tree;
    struct wlr_scene_rect *background;
    char title[256];
    int view_id;
    int x, y, width, height;
    bool active;
};

struct xp_system_tray {
    struct wlr_scene_tree *tree;
    struct wlr_scene_rect *background;
    struct xp_clock *clock;
    int x, y, width, height;
};

struct xp_clock {
    struct wlr_scene_tree *tree;
    struct wlr_scene_buffer *text_buffer;
    char time_str[9];
    int x, y;
    struct wl_event_source *timer;
};

// Create XP taskbar
static struct xp_taskbar *create_xp_taskbar(struct xp_server *server, int width) {
    struct xp_taskbar *taskbar = calloc(1, sizeof(struct xp_taskbar));
    wl_list_init(&taskbar->app_buttons);
    
    taskbar->width = width;
    taskbar->height = 30;
    
    taskbar->tree = wlr_scene_tree_create(server->scene_tree);
    wlr_scene_node_set_position(&taskbar->tree->node, 0, server->output_height - 30);
    
    // Taskbar background with XP gradient
    float color[4] = {0.83f, 0.81f, 0.78f, 1.0f}; // #D4D0C8
    taskbar->background = wlr_scene_rect_create(taskbar->tree, width, 30, color);
    
    // Create start button
    taskbar->start_button = create_xp_start_button(taskbar);
    
    // Create system tray
    taskbar->system_tray = create_xp_system_tray(taskbar, width - 150);
    
    return taskbar;
}

// Create XP start button
static struct xp_start_button *create_xp_start_button(struct xp_taskbar *taskbar) {
    struct xp_start_button *button = calloc(1, sizeof(struct xp_start_button));
    
    button->x = 0;
    button->y = 0;
    button->width = 100;
    button->height = 30;
    
    button->tree = wlr_scene_tree_create(taskbar->tree);
    wlr_scene_node_set_position(&button->tree->node, button->x, button->y);
    
    // Start button background with XP blue gradient
    float color[4] = {0.0f, 0.33f, 0.89f, 1.0f}; // #0055E3
    button->background = wlr_scene_rect_create(button->tree, button->width, button->height, color);
    
    return button;
}

#endif