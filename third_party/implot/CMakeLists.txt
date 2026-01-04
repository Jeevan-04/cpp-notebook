cmake_minimum_required(VERSION 3.10)
project(ImPlot)

add_library(ImPlot STATIC implot.cpp implot_items.cpp implot_demo.cpp)

target_include_directories(ImPlot PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../imgui
)

target_link_libraries(ImPlot PRIVATE imgui)
