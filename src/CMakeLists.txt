set(TARGET_NAME microui)
add_library(${TARGET_NAME} microui.cpp renderer.cpp atlas.c)
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(
  ${TARGET_NAME}
  PRIVATE OpenGL32
  PUBLIC UIRenderFrame)
target_compile_definitions(${TARGET_NAME} PRIVATE BUILD_MICROUI)
