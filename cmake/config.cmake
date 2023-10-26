# ============================================================================
# Include guards
# ============================================================================

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

# ============================================================================
# Set a default build configuration (Release)
# ============================================================================
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE
      Release
      CACHE STRING "Choose the type of build." FORCE
  )
  set_property(
    CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo"
  )
endif()
string(TOUPPER "${CMAKE_BUILD_TYPE}" U_CMAKE_BUILD_TYPE)

# ============================================================================
# compile using C++20
# ============================================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(CURA_DEPEND)
set(CURA_PRIVATE_LIBS)
set(CURA_PUBLIC_LIBS)

# ============================================================================
# Add dependencies via CPM (cmake/tools.cmake includes cmake/CPM.cmake)
#
# see https://github.com/TheLartians/CPM.cmake for more info
# ============================================================================

include(${CMAKE_CURRENT_LIST_DIR}/CPM.cmake)

CPMAddPackage(
  NAME fmt
  URL https://github.com/fmtlib/fmt/archive/refs/tags/9.1.0.zip
  # GITHUB_REPOSITORY fmtlib/fmt GIT_TAG 02ad5e11da5b2702cc1c9a8fdf750486beee08fe
  OPTIONS "FMT_INSTALL NO" # create an installable target
  # DOWNLOAD_ONLY YES
)

CPMAddPackage("gh:sgorsten/linalg#a3e87da35e32b781a4b6c01cdd5efbe7ae51c737")
if(linalg_ADDED)
  add_library(linalg INTERFACE IMPORTED)
  target_include_directories(linalg INTERFACE "${linalg_SOURCE_DIR}")
  list(APPEND CURA_PUBLIC_LIBS linalg)
endif()

CPMAddPackage(
  NAME tinyobjloader
  GITHUB_REPOSITORY tinyobjloader/tinyobjloader
  GIT_TAG 94d2f7fe1f7742818dbcd0917d11679d055a33de
  DOWNLOAD_ONLY YES
)
if(tinyobjloader_ADDED)
  add_library(tinyobjloader INTERFACE IMPORTED)
  target_include_directories(tinyobjloader INTERFACE "${tinyobjloader_SOURCE_DIR}/")
  list(APPEND CURA_PRIVATE_LIBS tinyobjloader)
endif()


CPMAddPackage(
  NAME spdlog
  URL https://github.com/gabime/spdlog/archive/refs/tags/v1.10.0.zip
  OPTIONS "SPDLOG_INSTALL NO" # create an installable target
          "SPDLOG_FMT_EXTERNAL YES" # use the fmt library we added above instead of the bundled one
)
if(spdlog_ADDED)
  list(APPEND CURA_DEPEND spdlog)
  list(APPEND CURA_PUBLIC_LIBS spdlog::spdlog)
endif()



#Set up location for build products
set_target_properties(
  ${CURA_DEPEND}
  PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CURA_BINARY_DIR}
             LIBRARY_OUTPUT_DIRECTORY ${CURA_BINARY_DIR}
             RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CURA_BINARY_DIR}
             LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CURA_BINARY_DIR}
             RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CURA_BINARY_DIR}
             LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CURA_BINARY_DIR}
             RUNTIME_OUTPUT_DIRECTORY_RELNODEBINFO ${CURA_BINARY_DIR}
             LIBRARY_OUTPUT_DIRECTORY_RELNODEBINFO ${CURA_BINARY_DIR}
             RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CURA_BINARY_DIR}
             LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL ${CURA_BINARY_DIR}
)