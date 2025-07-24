#define _POSIX_C_SOURCE 200112L
#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/util/log.h>
#include <xkbcommon/xkbcommon.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

// Forward declarations
struct xp_server;
struct xp_output;
struct xp_view;

// Core server structure
struct xp_server {
    struct wl_display *wl_display;
    struct wlr_backend *backend;
    struct wlr_renderer *renderer;
    struct wlr_allocator *allocator;
    
    struct wlr_compositor *compositor;
    struct wlr_subcompositor *subcompositor;
    struct wlr_data_device_manager *data_device_manager;
    
    struct wlr_output_layout *output_layout;
    struct wl_list outputs;
    struct wl_listener new_output;
    
    struct wlr_xdg_shell *xdg_shell;
    struct wl_listener new_xdg_surface;
    
    struct wlr_layer_shell_v1 *layer_shell;
    struct wl_listener new_layer_surface;
    
    struct wlr_cursor *cursor;
    struct wlr_xcursor_manager *cursor_mgr;
    struct wl_listener cursor_motion;
    struct wl_listener cursor_motion_absolute;
    struct wl_listener cursor_button;
    struct wl_listener cursor_axis;
    struct wl_listener cursor_frame;
    
    struct wlr_seat *seat;
    struct wl_listener new_input;
    struct wl_listener request_cursor;
    struct wl_listener request_set_selection;
    
    // XP Shell components
    struct xp_desktop *desktop;
    struct xp_taskbar *taskbar;
    struct xp_start_menu *start_menu;
    
    // Views (windows)
    struct wl_list views;
};

// Output structure
struct xp_output {
    struct wl_list link;
    struct xp_server *server;
    struct wlr_output *wlr_output;
    struct wl_listener frame;
    struct wl_listener destroy;
};

// View structure (application windows)
struct xp_view {
    struct wl_list link;
    struct xp_server *server;
    struct wlr_xdg_surface *xdg_surface;
    
    // Position and state
    int x, y;
    int width, height;
    bool mapped;
    bool is_active;
    
    // Listeners
    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener destroy;
    struct wl_listener commit;
    struct wl_listener request_move;
    struct wl_listener request_resize;
    struct wl_listener request_maximize;
    struct wl_listener request_fullscreen;
};

// Include shell components
#include "../shell/desktop.h"
#include "../shell/taskbar.h"
#include "../shell/start_menu.h"

// Output handling
static void output_frame(struct wl_listener *listener, void *data) {
    struct xp_output *output = wl_container_of(listener, output, frame);
    struct wlr_output *wlr_output = output->wlr_output;
    
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    
    // Enable VRR if supported
    if (wlr_output->adaptive_sync_status == WLR_OUTPUT_ADAPTIVE_SYNC_SUPPORTED) {
        wlr_output_enable_adaptive_sync(wlr_output, true);
    }
    
    if (!wlr_output_attach_render(wlr_output, NULL)) {
        return;
    }
    
    int width, height;
    wlr_output_effective_resolution(wlr_output, &width, &height);
    
    wlr_renderer_begin(output->server->renderer, width, height);
    
    // Clear with XP background color
    wlr_renderer_clear(output->server->renderer, (float[]){0.1, 0.3, 0.6, 1.0});
    
    wlr_renderer_end(output->server->renderer);
    wlr_output_commit(wlr_output);
}

static void output_destroy(struct wl_listener *listener, void *data) {
    struct xp_output *output = wl_container_of(listener, output, destroy);
    wl_list_remove(&output->frame.link);
    wl_list_remove(&output->destroy.link);
    wl_list_remove(&output->link);
    free(output);
}

static void server_new_output(struct wl_listener *listener, void *data) {
    struct xp_server *server = wl_container_of(listener, server, new_output);
    struct wlr_output *wlr_output = data;
    
    wlr_log(WLR_INFO, "New output: %s", wlr_output->name);
    
    struct xp_output *output = calloc(1, sizeof(struct xp_output));
    output->wlr_output = wlr_output;
    output->server = server;
    wl_list_insert(&server->outputs, &output->link);
    
    output->frame.notify = output_frame;
    wl_signal_add(&wlr_output->events.frame, &output->frame);
    output->destroy.notify = output_destroy;
    wl_signal_add(&wlr_output->events.destroy, &output->destroy);
    
    struct wlr_output_mode *mode = wlr_output_preferred_mode(wlr_output);
    if (mode != NULL) {
        wlr_output_set_mode(wlr_output, mode);
    }
    
    wlr_output_layout_add_auto(server->output_layout, wlr_output);
    
    wlr_output_enable(wlr_output, true);
    if (!wlr_output_commit(wlr_output)) {
        wlr_log(WLR_ERROR, "Failed to commit output");
        return;
    }
    
    // Configure cursor
    wlr_cursor_attach_output_layout(server->cursor, server->output_layout);
    wlr_cursor_map_to_output(server->cursor, wlr_output);
}

// Seat handling
static void seat_request_cursor(struct wl_listener *listener, void *data) {
    struct xp_server *server = wl_container_of(listener, server, request_cursor);
    struct wlr_seat_pointer_request_set_cursor_event *event = data;
    struct wlr_seat_client *focused_client = server->seat->pointer_state.focused_client;
    
    if (focused_client == event->seat_client) {
        wlr_cursor_set_surface(server->cursor, event->surface,
                              event->hotspot_x, event->hotspot_y);
    }
}

static void server_new_input(struct wl_listener *listener, void *data) {
    struct xp_server *server = wl_container_of(listener, server, new_input);
    struct wlr_input_device *device = data;
    
    wlr_log(WLR_INFO, "New input device: %s", device->name);
    
    switch (device->type) {
    case WLR_INPUT_DEVICE_KEYBOARD:;
        struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
        struct xkb_keymap *keymap = xkb_map_new_from_names(context, NULL,
                                                          XKB_KEYMAP_COMPILE_NO_FLAGS);
        
        wlr_seat_set_keyboard(server->seat, wlr_keyboard_from_input_device(device));
        xkb_keymap_unref(keymap);
        xkb_context_unref(context);
        break;
    case WLR_INPUT_DEVICE_POINTER:
        wlr_cursor_attach_input_device(server->cursor, wlr_pointer_from_input_device(device));
        break;
    default:
        break;
    }
    
    uint32_t caps = WL_SEAT_CAPABILITY_POINTER;
    if (!wl_list_empty(&server->keyboards)) {
        caps |= WL_SEAT_CAPABILITY_KEYBOARD;
    }
    wlr_seat_set_capabilities(server->seat, caps);
}

int main(int argc, char *argv[]) {
    wlr_log_init(WLR_DEBUG, NULL);
    
    struct xp_server server = {0};
    wl_list_init(&server.outputs);
    wl_list_init(&server.views);
    
    server.wl_display = wl_display_create();
    if (!server.wl_display) {
        wlr_log(WLR_ERROR, "Failed to create Wayland display");
        return 1;
    }
    
    server.backend = wlr_backend_autocreate(server.wl_display);
    if (!server.backend) {
        wlr_log(WLR_ERROR, "Failed to create backend");
        return 1;
    }
    
    server.renderer = wlr_renderer_autocreate(server.backend);
    if (!server.renderer) {
        wlr_log(WLR_ERROR, "Failed to create renderer");
        return 1;
    }
    
    server.allocator = wlr_allocator_autocreate(server.backend, server.renderer);
    if (!server.allocator) {
        wlr_log(WLR_ERROR, "Failed to create allocator");
        return 1;
    }
    
    wlr_renderer_init_wl_display(server.renderer, server.wl_display);
    
    server.compositor = wlr_compositor_create(server.wl_display, server.renderer);
    server.subcompositor = wlr_subcompositor_create(server.wl_display);
    server.data_device_manager = wlr_data_device_manager_create(server.wl_display);
    
    server.output_layout = wlr_output_layout_create();
    server.new_output.notify = server_new_output;
    wl_signal_add(&server.backend->events.new_output, &server.new_output);
    
    server.xdg_shell = wlr_xdg_shell_create(server.wl_display, 3);
    server.layer_shell = wlr_layer_shell_v1_create(server.wl_display);
    
    server.cursor = wlr_cursor_create();
    wlr_cursor_attach_output_layout(server.cursor, server.output_layout);
    server.cursor_mgr = wlr_xcursor_manager_create(NULL, 24);
    
    server.seat = wlr_seat_create(server.wl_display, "seat0");
    server.new_input.notify = server_new_input;
    wl_signal_add(&server.backend->events.new_input, &server.new_input);
    server.request_cursor.notify = seat_request_cursor;
    wl_signal_add(&server.seat->events.request_set_cursor, &server.request_cursor);
    
    const char *socket = wl_display_add_socket_auto(server.wl_display);
    if (!socket) {
        wlr_log(WLR_ERROR, "Failed to create Wayland socket");
        return 1;
    }
    
    if (!wlr_backend_start(server.backend)) {
        wlr_log(WLR_ERROR, "Failed to start backend");
        return 1;
    }
    
    setenv("WAYLAND_DISPLAY", socket, true);
    wlr_log(WLR_INFO, "Running XP Desktop on WAYLAND_DISPLAY=%s", socket);
    
    wl_display_run(server.wl_display);
    
    wl_display_destroy_clients(server.wl_display);
    wl_display_destroy(server.wl_display);
    return 0;
}