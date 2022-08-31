set(TARGET_NAME demo)
add_executable(${TARGET_NAME} main.c renderer.c)
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(${TARGET_NAME} PRIVATE microui glfw OpenGL32)
