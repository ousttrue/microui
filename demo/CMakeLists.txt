set(TARGET_NAME demo)
add_executable(${TARGET_NAME} main.cpp ui.cpp)
target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}
          ${CMAKE_CURRENT_LIST_DIR}/../_external/glfw/include
          ${CMAKE_CURRENT_LIST_DIR}/../src)
target_link_libraries(${TARGET_NAME} PRIVATE microui glfw OpenGL32)
