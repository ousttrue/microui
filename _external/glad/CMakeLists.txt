set(TARGET_NAME glad)
add_library(${TARGET_NAME} STATIC
            ${CMAKE_CURRENT_LIST_DIR}/../glfw/deps/glad_gl.c)
target_include_directories(${TARGET_NAME}
                           PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../glfw/deps)
