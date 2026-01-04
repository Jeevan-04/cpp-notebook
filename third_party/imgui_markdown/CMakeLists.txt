cmake_minimum_required(VERSION 3.10)
project(imgui_markdown)

add_library(imgui_markdown STATIC imgui_markdown.cpp)

target_include_directories(imgui_markdown PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../imgui
)

target_link_libraries(imgui_markdown PRIVATE imgui)
