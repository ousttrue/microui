set(TARGET_NAME microui)
add_library(${TARGET_NAME} STATIC microui.cpp)
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
