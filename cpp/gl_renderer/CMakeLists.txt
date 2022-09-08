set(TARGET_NAME gl_renderer)
add_library(${TARGET_NAME} SHARED renderer.cpp atlas.c)
target_compile_definitions(${TARGET_NAME} PRIVATE BUILD_GLRENDERER)
target_link_libraries(${TARGET_NAME} PRIVATE UIRenderFrame OpenGL32)
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

add_executable(atlas_to_bytes atlas_to_bytes.cpp atlas.c)
