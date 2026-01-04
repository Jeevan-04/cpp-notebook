cmake_minimum_required(VERSION 3.10)
project(TextEditor)

add_library(TextEditor STATIC TextEditor.cpp)

target_include_directories(TextEditor PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../imgui
)

target_link_libraries(TextEditor PRIVATE imgui)
