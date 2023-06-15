cmake_minimum_required(VERSION 3.15)
set(PROJECT Rasterizer)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(source)

#Copy the assets folder to the build directory
add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets
)
add_dependencies(${PROJECT} copy_assets)