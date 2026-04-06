cmake_minimum_required(VERSION 3.10)
project(xp-desktop LANGUAGES C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O2")

find_package(PkgConfig REQUIRED)
pkg_check_modules(WLR REQUIRED wlroots>=0.17)
pkg_check_modules(WAYLAND_SERVER REQUIRED wayland-server>=1.20)
pkg_check_modules(WAYLAND_CLIENT REQUIRED wayland-client>=1.20)
pkg_check_modules(XKB REQUIRED xkbcommon)
pkg_check_modules(CAIRO REQUIRED cairo)
pkg_check_modules(PANGO REQUIRED pangocairo)

find_program(WAYLAND_SCANNER wayland-scanner REQUIRED)
pkg_get_variable(WAYLAND_PROTOCOLS wayland-protocols pkgdatadir)

set(PROTO_DIR ${CMAKE_CURRENT_BINARY_DIR}/protocols)
file(MAKE_DIRECTORY ${PROTO_DIR})

macro(GEN_PROTO NAME XML)
    add_custom_command(OUTPUT ${PROTO_DIR}/${NAME}-client-protocol.h
        COMMAND ${WAYLAND_SCANNER} client-header ${XML} ${PROTO_DIR}/${NAME}-client-protocol.h)
    add_custom_command(OUTPUT ${PROTO_DIR}/${NAME}-protocol.c
        COMMAND ${WAYLAND_SCANNER} private-code ${XML} ${PROTO_DIR}/${NAME}-protocol.c)
endmacro()

GEN_PROTO(xdg-shell ${WAYLAND_PROTOCOLS}/stable/xdg-shell/xdg-shell.xml)

add_executable(xp-compositor
    src/main.c src/render.c src/shell.c src/input.c
    ${PROTO_DIR}/xdg-shell-protocol.c
)
target_include_directories(xp-compositor PRIVATE src ${PROTO_DIR} ${WLR_INCLUDE_DIRS} ${WAYLAND_SERVER_INCLUDE_DIRS} ${XKB_INCLUDE_DIRS} ${CAIRO_INCLUDE_DIRS} ${PANGO_INCLUDE_DIRS})
target_link_libraries(xp-compositor ${WLR_LIBRARIES} ${WAYLAND_SERVER_LIBRARIES} ${XKB_LIBRARIES} ${CAIRO_LIBRARIES} ${PANGO_LIBRARIES} drm pthread m)

add_executable(xp-notepad src/notepad.c ${PROTO_DIR}/xdg-shell-protocol.c)
target_include_directories(xp-notepad PRIVATE ${PROTO_DIR} ${WAYLAND_CLIENT_INCLUDE_DIRS} ${CAIRO_INCLUDE_DIRS} ${PANGO_INCLUDE_DIRS})
target_link_libraries(xp-notepad ${WAYLAND_CLIENT_LIBRARIES} ${CAIRO_LIBRARIES} ${PANGO_LIBRARIES} rt pthread)

install(TARGETS xp-compositor xp-notepad DESTINATION bin)
install(FILES xp-desktop.desktop DESTINATION /usr/share/wayland-sessions/)
