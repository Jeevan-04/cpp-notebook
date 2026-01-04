cmake_minimum_required(VERSION 3.10)
project(cppnb)

set(CMAKE_CXX_STANDARD 20)

# Add subdirectories
add_subdirectory(third_party/imgui)
add_subdirectory(third_party/TextEditor)
add_subdirectory(third_party/imgui_markdown)
add_subdirectory(third_party/ImPlot)

find_package(SDL2 REQUIRED)

# Add glad
add_library(glad STATIC third_party/glad/src/glad.c)
target_include_directories(glad PUBLIC third_party/glad/include)

# Main executable
add_executable(cppnb 
    main.cpp
    src/ui/notebook.cpp
    src/ui/code_cell.cpp
    src/ui/markdown_cell.cpp
    src/utils/logger.cpp
)

# Include directories
target_include_directories(cppnb PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/TextEditor
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui_markdown
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/ImPlot
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glad/include
    ${SDL2_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(cppnb PRIVATE
    glad
    imgui
    TextEditor
    imgui_markdown
    ImPlot
    ${SDL2_LIBRARIES}
)

# macOS specific settings
if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
    target_link_libraries(cppnb PRIVATE "-framework OpenGL")
else()
    target_link_libraries(cppnb PRIVATE OpenGL::GL)
endif()