# AUTOMOC requires 2.8.6. Per-target include directories would require 2.8.8
cmake_minimum_required(VERSION 2.8.6)
PROJECT(BRAHMS)

# BRAHMS version info
set(VERSION_BRAHMS_MAJ 0)
set(VERSION_BRAHMS_MIN 8)
set(VERSION_BRAHMS_REL 0)
set(VERSION_BRAHMS_REV 0)

# From CMAKE_SYSTEM #define __OSX__ or __GLN__ etc
if(CMAKE_SYSTEM MATCHES Linux.*)
  message(STATUS "CMAKE_SYSTEM: " ${CMAKE_SYSTEM})
  set(BRAHMS_HOST_DEFINITION __GLN__)
elseif(CMAKE_SYSTEM MATCHES BSD.*)
  set(BRAHMS_HOST_DEFINITION __NIX__)
elseif(CMAKE_SYSTEM MATCHES Mac.*)
  set(BRAHMS_HOST_DEFINITION __OSX__)
elseif(CMAKE_SYSTEM MATCHES Win.*)
  set(BRAHMS_HOST_DEFINITION __WIN__)
else()
  message(ERROR "Platform not supported: " ${CMAKE_SYSTEM})
endif()

# CMake configuration header
configure_file (
  "${PROJECT_SOURCE_DIR}/BrahmsConfig.h.in"
  "${PROJECT_BINARY_DIR}/BrahmsConfig.h"
)

include_directories ("${PROJECT_SOURCE_DIR}/framework/engine/base")
include_directories ("${PROJECT_SOURCE_DIR}/framework/public")
add_subdirectory(framework)



