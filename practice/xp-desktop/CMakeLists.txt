cmake_minimum_required(VERSION 3.10)
project(xp-desktop LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-parameter")

# Find dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(WLR REQUIRED wlroots)
pkg_check_modules(WAYLAND REQUIRED wayland-server)
pkg_check_modules(WAYLAND_CLIENT REQUIRED wayland-client)
pkg_check_modules(XKBCOMMON REQUIRED xkbcommon)
pkg_check_modules(CAIRO REQUIRED cairo)
pkg_check_modules(PANGO REQUIRED pangocairo)

# Generate Wayland protocols
find_program(WAYLAND_SCANNER_EXECUTABLE NAMES wayland-scanner)
pkg_get_variable(WAYLAND_PROTOCOLS_DIR wayland-protocols pkgdatadir)

# XDG Shell protocol
add_custom_command(
    OUTPUT xdg-shell-protocol.c xdg-shell-client-protocol.h xdg-shell-server-protocol.h
    COMMAND ${WAYLAND_SCANNER_EXECUTABLE} private-code
        ${WAYLAND_PROTOCOLS_DIR}/stable/xdg-shell/xdg-shell.xml
        xdg-shell-protocol.c
    COMMAND ${WAYLAND_SCANNER_EXECUTABLE} client-header
        ${WAYLAND_PROTOCOLS_DIR}/stable/xdg-shell/xdg-shell.xml
        xdg-shell-client-protocol.h
    COMMAND ${WAYLAND_SCANNER_EXECUTABLE} server-header
        ${WAYLAND_PROTOCOLS_DIR}/stable/xdg-shell/xdg-shell.xml
        xdg-shell-server-protocol.h
    DEPENDS ${WAYLAND_PROTOCOLS_DIR}/stable/xdg-shell/xdg-shell.xml
)

# Compositor
add_executable(xp-compositor
    src/compositor/main.c
    xdg-shell-protocol.c
)

target_include_directories(xp-compositor PRIVATE
    ${WLR_INCLUDE_DIRS}
    ${WAYLAND_INCLUDE_DIRS}
    ${XKBCOMMON_INCLUDE_DIRS}
    ${CAIRO_INCLUDE_DIRS}
    ${PANGO_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(xp-compositor
    ${WLR_LIBRARIES}
    ${WAYLAND_LIBRARIES}
    ${XKBCOMMON_LIBRARIES}
    ${CAIRO_LIBRARIES}
    ${PANGO_LIBRARIES}
    drm
    pthread
)

# Applications
add_executable(xp-notepad
    src/applications/notepad.c
    xdg-shell-protocol.c
)

target_include_directories(xp-notepad PRIVATE
    ${WAYLAND_CLIENT_INCLUDE_DIRS}
    ${CAIRO_INCLUDE_DIRS}
    ${PANGO_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(xp-notepad
    ${WAYLAND_CLIENT_LIBRARIES}
    ${CAIRO_LIBRARIES}
    ${PANGO_LIBRARIES}
)

# Install
install(TARGETS xp-compositor xp-notepad DESTINATION bin)
install(FILES xp-desktop.desktop DESTINATION /usr/share/wayland-sessions/)
install(DIRECTORY themes/ DESTINATION share/xp-desktop/themes)