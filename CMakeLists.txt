cmake_minimum_required(VERSION 3.15)

project(
  cura
  VERSION 1.0
  LANGUAGES CXX
)

# ============================================================================
# Configure dependencies
# ============================================================================
include(cmake/config.cmake)

# ============================================================================
# Create library
# ============================================================================
set(cura_lib_SOURCES)
list(
  APPEND
  cura_lib_SOURCES

  include/cura/buffer.h
  include/cura/camera.h
  include/cura/depth_shader.h
  include/cura/light.h
  include/cura/line.h
  include/cura/mat.h
  include/cura/math.h
  include/cura/mesh.h
  include/cura/model.h
  include/cura/normal_map_shader.h
  include/cura/pipeline.h
  include/cura/quad_shader.h
  include/cura/rasterizer.h
  include/cura/shader.h
  include/cura/texture.h
  include/cura/transforms.h
  include/cura/triangle.h
  include/cura/vec.h
  include/cura/vertex.h

  #source files

  #01
  #src/01Triangle/assignment01Triangle.cpp



)

add_library(cura_lib OBJECT ${cura_lib_SOURCES})


# Link dependencies
target_link_libraries(
  cura_lib
  PRIVATE ${CURA_PRIVATE_LIBS}
  PUBLIC ${CURA_PUBLIC_LIBS}
)

target_include_directories(
  cura_lib PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                   $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>
)


# ============================================================================
# Add executables and link with cura_lib
# ============================================================================

add_executable(assignment01 src/01Triangle/assignment01Triangle.cpp)
target_link_libraries(assignment01 PRIVATE cura_lib)


add_executable(part1 src/main.cpp)
target_link_libraries(part1 PRIVATE cura_lib)
