#ifndef XP_START_MENU_H
#define XP_START_MENU_H

#include <wayland-server-core.h>
#include <wlr/types/wlr_scene.h>

struct xp_start_menu {
    struct wlr_scene_tree *tree;
    struct wlr_scene_rect *background;
    struct wlr_scene_rect *sidebar;
    struct wl_list menu_items;
    int x, y, width, height;
    bool visible;
};

struct xp_menu_item {
    struct wl_list link;
    struct wlr_scene_tree *tree;
    struct wlr_scene_rect *background;
    struct wlr_scene_buffer *text;
    char label[256];
    char *exec_cmd;
    int x, y, width, height;
    bool hover;
};

// Create XP start menu
static struct xp_start_menu *create_xp_start_menu(struct xp_server *server) {
    struct xp_start_menu *menu = calloc(1, sizeof(struct xp_start_menu));
    wl_list_init(&menu->menu_items);
    
    menu->x = 0;
    menu->y = 0; // Will be positioned above taskbar
    menu->width = 300;
    menu->height = 400;
    menu->visible = false;
    
    menu->tree = wlr_scene_tree_create(server->scene_tree);
    wlr_scene_node_set_position(&menu->tree->node, menu->x, menu->y);
    wlr_scene_node_set_enabled(&menu->tree->node, false);
    
    // Menu background
    float bg_color[4] = {0.83f, 0.81f, 0.78f, 1.0f}; // #D4D0C8
    menu->background = wlr_scene_rect_create(menu->tree, menu->width, menu->height, bg_color);
    
    // Sidebar
    float sidebar_color[4] = {0.0f, 0.33f, 0.89f, 1.0f}; // #0055E3
    menu->sidebar = wlr_scene_rect_create(menu->tree, 30, menu->height, sidebar_color);
    
    // Add menu items
    add_start_menu_items(menu);
    
    return menu;
}

// Add items to start menu
static void add_start_menu_items(struct xp_start_menu *menu) {
    const char *items[][2] = {
        {"Programs", NULL},
        {"Documents", NULL},
        {"Settings", NULL},
        {"Search", NULL},
        {"Help and Support", NULL},
        {"Run...", NULL},
        {"", NULL}, // Separator
        {"Shut Down...", NULL}
    };
    
    int y_offset = 10;
    for (int i = 0; i < 8; i++) {
        if (strlen(items[i][0]) == 0) {
            // Separator - skip for now
            y_offset += 10;
            continue;
        }
        
        struct xp_menu_item *item = calloc(1, sizeof(struct xp_menu_item));
        strncpy(item->label, items[i][0], sizeof(item->label) - 1);
        item->exec_cmd = items[i][1] ? strdup(items[i][1]) : NULL;
        
        item->x = 35;
        item->y = y_offset;
        item->width = menu->width - 40;
        item->height = 20;
        
        item->tree = wlr_scene_tree_create(menu->tree);
        wlr_scene_node_set_position(&item->tree->node, item->x, item->y);
        
        // Background (transparent normally)
        float bg_color[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        item->background = wlr_scene_rect_create(item->tree, item->width, item->height, bg_color);
        
        wl_list_insert(&menu->menu_items, &item->link);
        y_offset += 25;
    }
}

#endif