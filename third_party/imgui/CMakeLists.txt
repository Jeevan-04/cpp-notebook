cmake_minimum_required(VERSION 3.10)
project(imgui)

find_package(SDL2 REQUIRED)

set(IMGUI_SOURCES
    imgui.cpp
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    backends/imgui_impl_sdl2.cpp
    backends/imgui_impl_opengl3.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})

target_include_directories(imgui PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/backends
    ${SDL2_INCLUDE_DIRS}/SDL2  # Add this line to point to SDL2 subdirectory
)

if(APPLE)
    target_link_libraries(imgui PRIVATE "-framework OpenGL" ${SDL2_LIBRARIES})
else()
    target_link_libraries(imgui PRIVATE OpenGL::GL ${SDL2_LIBRARIES})
endif()
