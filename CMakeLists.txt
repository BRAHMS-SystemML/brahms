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
  set(BRAHMS_HOST_DEFINITION "-D__GLN__")
elseif(CMAKE_SYSTEM MATCHES BSD.*)
  set(BRAHMS_HOST_DEFINITION "-D__NIX__")
elseif(CMAKE_SYSTEM MATCHES Darwin.*)
  set(BRAHMS_HOST_DEFINITION "-D__OSX__")
elseif(CMAKE_SYSTEM MATCHES Win.*)
  set(BRAHMS_HOST_DEFINITION "-D__WIN__")
else()
  message(ERROR "Platform not supported: " ${CMAKE_SYSTEM})
endif()

# CFLAGS
set(CMAKE_CXX_FLAGS ${BRAHMS_HOST_DEFINITION})
message(STATUS "Flags " ${CMAKE_CXX_FLAGS})

# This needs work - it's too simplistic and I've no idea if it would
# work on Mac or Windows.
if(CMAKE_SYSTEM_PROCESSOR MATCHES .*64.*)
  set(ARCH_BITS 64)
else()
  set(ARCH_BITS 32)
endif()
message(STATUS "Processor " ${CMAKE_SYSTEM_PROCESSOR} " Arch bits: " ${ARCH_BITS})

# CMake configuration header
configure_file (
  "${PROJECT_SOURCE_DIR}/BrahmsConfig.h.in"
  "${PROJECT_BINARY_DIR}/BrahmsConfig.h"
)

# Framework include directories
include_directories ("${PROJECT_SOURCE_DIR}/framework/engine")
include_directories ("${PROJECT_SOURCE_DIR}/framework/public")
include_directories ("${PROJECT_SOURCE_DIR}/framework/channel")
include_directories ("${PROJECT_SOURCE_DIR}/framework/compress")
include_directories ("${PROJECT_SOURCE_DIR}/framework/public")

include_directories ("${CMAKE_BINARY_DIR}") # for BrahmsConfig.h

add_subdirectory(framework)

#add_executable(brahms-execute2
#  framework/execute/main.cpp framework/execute/info.cpp framework/execute/os.cpp
#  framework/execute/tfs.cpp framework/execute/version.cpp
#  )
#target_link_libraries(brahms-execute2 brahms-engine)

