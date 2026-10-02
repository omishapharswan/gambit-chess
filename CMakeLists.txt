cmake_minimum_required(VERSION 3.16)
project(gambit VERSION 1.0.0 LANGUAGES CXX)

# Release by default so a plain build is fast. Multi-config generators
# (Visual Studio) pick the config at build time, so they are left alone.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Must be called in the top-level file so ctest can run from the build root.
enable_testing()

add_subdirectory(engine)
