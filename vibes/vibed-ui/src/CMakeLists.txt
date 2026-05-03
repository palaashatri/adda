set(PLATFORM_SOURCES
    platform/Platform.cpp
)

if(WIN32)
    list(APPEND PLATFORM_SOURCES platform/windows/WinPlatform.cpp)
elseif(APPLE)
    list(APPEND PLATFORM_SOURCES platform/mac/MacPlatform.cpp)
    set_source_files_properties(platform/mac/MacPlatform.cpp PROPERTIES LANGUAGE OBJCXX)
elseif(UNIX)
    list(APPEND PLATFORM_SOURCES platform/linux/LinuxPlatform.cpp)
endif()

add_library(core STATIC
    core/Application.cpp
    core/Event.cpp
    core/Timer.cpp
    core/Logger.cpp
)

add_library(platform STATIC ${PLATFORM_SOURCES})

add_library(render STATIC
    render/Renderer.cpp
    render/legacy/SoftwareRenderer.cpp
    render/modern/ModernRenderer.cpp
)

add_library(ui STATIC
    ui/View.cpp
    ui/ViewTree.cpp
    ui/Layout.cpp
    ui/FlexLayout.cpp
    ui/Theme.cpp
    ui/Color.cpp
    ui/Font.cpp
    ui/Button.cpp
    ui/Label.cpp
)

add_library(compositor STATIC
    compositor/Compositor.cpp
    compositor/Surface.cpp
    compositor/WindowManager.cpp
)

add_library(shell STATIC
    shell/Panel.cpp
    shell/Launcher.cpp
    shell/ShellApp.cpp
)

foreach(module_target core platform render ui compositor shell)
    target_include_directories(${module_target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
endforeach()

if(WIN32)
    target_link_libraries(platform PRIVATE user32 gdi32)
elseif(APPLE)
    find_library(APPKIT_FRAMEWORK AppKit REQUIRED)
    find_library(COCOA_FRAMEWORK Cocoa REQUIRED)
    find_library(COREGRAPHICS_FRAMEWORK CoreGraphics REQUIRED)
    find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)
    target_link_libraries(platform PRIVATE
        ${APPKIT_FRAMEWORK}
        ${COCOA_FRAMEWORK}
        ${COREGRAPHICS_FRAMEWORK}
        ${FOUNDATION_FRAMEWORK}
    )
elseif(UNIX)
    find_package(X11 REQUIRED)
    target_link_libraries(platform PRIVATE X11::X11)
endif()

target_link_libraries(ui PRIVATE core render)
target_link_libraries(core PUBLIC platform)
target_link_libraries(render PUBLIC platform)
target_link_libraries(compositor PUBLIC core)
target_link_libraries(shell PUBLIC core ui compositor render)