add_executable(demo_basic demo_basic.cpp)
target_link_libraries(demo_basic PRIVATE core ui render)
target_include_directories(demo_basic PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_executable(demo_controls demo_controls.cpp)
target_link_libraries(demo_controls PRIVATE core ui render)
target_include_directories(demo_controls PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_executable(demo_text_editor demo_text_editor.cpp)
target_link_libraries(demo_text_editor PRIVATE core ui render)
target_include_directories(demo_text_editor PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_executable(demo_event_inspector demo_event_inspector.cpp)
target_link_libraries(demo_event_inspector PRIVATE core ui render)
target_include_directories(demo_event_inspector PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_executable(demo_button_lab demo_button_lab.cpp)
target_link_libraries(demo_button_lab PRIVATE core ui render)
target_include_directories(demo_button_lab PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_executable(demo_shell_preview demo_shell_preview.cpp)
target_link_libraries(demo_shell_preview PRIVATE shell)
target_include_directories(demo_shell_preview PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_executable(demo_file_manager demo_file_manager.cpp)
target_link_libraries(demo_file_manager PRIVATE core ui render)
target_include_directories(demo_file_manager PRIVATE ${CMAKE_SOURCE_DIR}/src)

# On Windows with MSVC, suppress the console window so only the GUI window appears.
# /ENTRY:mainCRTStartup keeps standard main() while switching to SUBSYSTEM:WINDOWS.
if(MSVC)
    set(DEMO_TARGETS
        demo_basic demo_controls demo_text_editor
        demo_event_inspector demo_button_lab demo_shell_preview
        demo_file_manager
    )
    foreach(tgt ${DEMO_TARGETS})
        target_link_options(${tgt} PRIVATE "/SUBSYSTEM:WINDOWS" "/ENTRY:mainCRTStartup")
    endforeach()
endif()
