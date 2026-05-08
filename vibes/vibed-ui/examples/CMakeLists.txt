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
