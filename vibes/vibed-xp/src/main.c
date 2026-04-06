#define _POSIX_C_SOURCE 200112L
#include "server.h"
#include <unistd.h>
#include <stdio.h>
#include <wayland-server.h>
#include <wlr/util/log.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_seat.h>

static void server_new_output(struct wl_listener *listener, void *data) {
    struct xp_server *server = wl_container_of(listener, server, new_output);
    struct wlr_output *wlr_out = data;

    struct xp_output *output = calloc(1, sizeof(struct xp_output));
    output->wlr_output = wlr_out;
    output->server = server;
    wl_list_insert(&server->outputs, &output->link);

    output->scene_output = wlr_scene_output_create(server->scene, wlr_out);
    if (!output->scene_output) {
        free(output);
        return;
    }

    wlr_output_layout_add_auto(server->output_layout, wlr_out);
    wlr_cursor_attach_output_layout(server->cursor, server->output_layout);
    wlr_output_init_render(wlr_out, server->allocator, server->renderer);

    struct wlr_output_state state;
    wlr_output_state_init(&state);
    wlr_output_state_set_enabled(&state, true);
    wlr_output_state_set_mode(&state, wlr_output_preferred_mode(wlr_out));
    // Enable VRR if supported
    if (wlr_output->adaptive_sync_status == WLR_OUTPUT_ADAPTIVE_SYNC_SUPPORTED) {
        wlr_output_state_set_adaptive_sync_enabled(&state, true);
    }
    wlr_output_commit_state(wlr_out, &state);
    wlr_output_state_finish(&state);
}

static void server_new_input(struct wl_listener *listener, void *data) {
    struct xp_server *server = wl_container_of(listener, server, new_input);
    struct wlr_input_device *device = data;

    switch (device->type) {
    case WLR_INPUT_DEVICE_KEYBOARD:
        wlr_seat_set_keyboard(server->seat, device);
        break;
    case WLR_INPUT_DEVICE_POINTER:
        wlr_cursor_attach_input_device(server->cursor, device);
        break;
    default: break;
    }

    uint32_t caps = WL_SEAT_CAPABILITY_POINTER | WL_SEAT_CAPABILITY_KEYBOARD;
    wlr_seat_set_capabilities(server->seat, caps);
}

int main(int argc, char *argv[]) {
    wlr_log_init(WLR_INFO, NULL);

    struct xp_server server = {0};
    wl_list_init(&server.outputs);
    wl_list_init(&server.keyboards);

    server.wl_display = wl_display_create();
    server.backend = wlr_backend_autocreate(server.wl_display);
    server.renderer = wlr_renderer_autocreate(server.backend);
    server.allocator = wlr_allocator_autocreate(server.backend, server.renderer);
    wlr_renderer_init_wl_display(server.renderer, server.wl_display);

    server.scene = wlr_scene_create();
    server.desktop_layer = wlr_scene_tree_create(&server.scene->tree);
    server.taskbar_layer = wlr_scene_tree_create(&server.scene->tree);
    server.top_layer = wlr_scene_tree_create(&server.scene->tree);

    server.compositor = wlr_compositor_create(server.wl_display, server.renderer);
    server.subcompositor = wlr_subcompositor_create(server.wl_display);
    server.data_device = wlr_data_device_manager_create(server.wl_display);
    server.output_layout = wlr_output_layout_create(server.wl_display);
    server.xdg_shell = wlr_xdg_shell_create(server.wl_display, 3);
    server.layer_shell = wlr_layer_shell_v1_create(server.wl_display);
    server.cursor = wlr_cursor_create();
    server.cursor_mgr = wlr_xcursor_manager_create(NULL, 24);
    server.seat = wlr_seat_create(server.wl_display, "seat0");

    server.new_output.notify = server_new_output;
    wl_signal_add(&server.backend->events.new_output, &server.new_output);
    server.new_input.notify = server_new_input;
    wl_signal_add(&server.backend->events.new_input, &server.new_input);

    const char *socket = wl_display_add_socket_auto(server.wl_display);
    if (!socket) { wlr_log(WLR_ERROR, "Socket creation failed"); return 1; }

    if (!wlr_backend_start(server.backend)) {
        wlr_log(WLR_ERROR, "Backend start failed"); return 1;
    }

    setenv("WAYLAND_DISPLAY", socket, true);
    setenv("XDG_CURRENT_DESKTOP", "XP", true);
    wlr_log(WLR_INFO, "XP Desktop running on WAYLAND_DISPLAY=%s", socket);

    wl_display_run(server.wl_display);

    wlr_allocator_destroy(server.allocator);
    wlr_backend_destroy(server.backend);
    wl_display_destroy(server.wl_display);
    return 0;
}
