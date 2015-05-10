# AUTOMOC requires 2.8.6. Per-target include directories would require 2.8.8
cmake_minimum_required(VERSION 2.8.6)
project(BRAHMS)

# BRAHMS version info
set(VERSION_BRAHMS_MAJ 0)
set(VERSION_BRAHMS_MIN 8)
set(VERSION_BRAHMS_REL 0)
set(VERSION_BRAHMS_REV 0)

# From CMAKE_SYSTEM work out which of __OSX__, __GLN__, __NIX__ or
# __WIN__ are required
message(STATUS "Operating system: " ${CMAKE_SYSTEM})
if(CMAKE_SYSTEM MATCHES Linux.*)
  set(BRAHMS_HOST_DEFINITION "-D__GLN__")
elseif(CMAKE_SYSTEM MATCHES BSD.*)
  set(BRAHMS_HOST_DEFINITION "-D__NIX__")
elseif(APPLE)
  set(BRAHMS_HOST_DEFINITION "-D__OSX__")
elseif(CMAKE_SYSTEM MATCHES Win.*)
  set(BRAHMS_HOST_DEFINITION "-D__WIN__")
else()
  message(ERROR "Operating system not supported: " ${CMAKE_SYSTEM})
endif()

# CFLAGS
set(CMAKE_CXX_FLAGS ${BRAHMS_HOST_DEFINITION})
set(CMAKE_C_FLAGS ${BRAHMS_HOST_DEFINITION})

# This needs work - it's too simplistic and I've no idea if it would
# work on Mac or Windows.
if(CMAKE_SYSTEM_PROCESSOR MATCHES .*64.*)
  set(ARCH_BITS 64)
else()
  set(ARCH_BITS 32)
endif()
message(STATUS "Processor: " ${CMAKE_SYSTEM_PROCESSOR} " Arch bits: " ${ARCH_BITS})

# Optional features. All default to OFF.
option(COMPILE_MATLAB_BINDING "Compile the matlab binding" OFF)
option(COMPILE_PYTHON_BINDING "Compile the Python binding" OFF)
option(COMPILE_WITH_MPICH2 "Compile the mpich2 channel communications layer" OFF)
option(COMPILE_WX_COMPONENT "Compile the image numeric component which requires WX" OFF)
option(STANDALONE_INSTALL "Compile and install for standalone use (SystemML folder)" ON)

if(STANDALONE_INSTALL)
  message(STATUS "Standalone install - choose unix style install with `cmake -DSTANDALONE_INSTALL=OFF`")
  set(STANDALONE 1)
  # Set some locally defined installation paths:
  set(LIB_INSTALL_PATH SystemML/BRAHMS/bin)
  set(BIN_INSTALL_PATH SystemML/BRAHMS/bin)
  set(INCLUDE_INSTALL_PATH SystemML/BRAHMS/include)
  set(SHARE_BRAHMS_INSTALL_PATH SystemML/BRAHMS)
  # Statically link libbrahmsengine etc for a standalone build (could
  # probably statically link for non-standalone build too, to be
  # honest).
  set(BRAHMS_ENGINE_LINK_TYPE STATIC)
  # Inform the user of the install prefix and how to change it:
  message(STATUS "Install location: ${CMAKE_INSTALL_PREFIX}/SystemML")
  message(STATUS "  (This can be changed with `cmake -DCMAKE_INSTALL_PREFIX=/some/place`")
else()
  message(STATUS "Unix style install - choose standalone install with `cmake -DSTANDALONE_INSTALL=ON`")
  set(STANDALONE 0)

  set(LIB_INSTALL_PATH lib)
  set(BIN_INSTALL_PATH bin)
  set(INCLUDE_INSTALL_PATH include)
  set(SHARE_BRAHMS_INSTALL_PATH share/brahms)

  set(BRAHMS_ENGINE_LINK_TYPE SHARED)

  message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
  message(STATUS "  (This can be changed with `cmake -DCMAKE_INSTALL_PREFIX=/some/place`")
endif()

# A section to ensure library linking works on Apple.
if(APPLE)
  set(CMAKE_MACOSX_RPATH ON)
  # use, i.e. don't skip the full RPATH for the build tree
  set(CMAKE_SKIP_BUILD_RPATH  FALSE)
  # when building, don't use the install RPATH already
  # (but later on when installing)
  set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
  # the RPATH to be used when installing
  set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
  # add the automatically determined parts of the RPATH
  # which point to directories outside the build tree to the install RPATH
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
  # the RPATH to be used when installing, but only if it's not a system directory
  list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
  if("${isSystemDir}" STREQUAL "-1")
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
  endif("${isSystemDir}" STREQUAL "-1")
endif(APPLE)

# Lib finding.
include (BrahmsLibFind.cmake)

# CMake configuration header
configure_file (
  "${PROJECT_SOURCE_DIR}/BrahmsConfig.h.in"
  "${PROJECT_BINARY_DIR}/BrahmsConfig.h"
)

# for BrahmsConfig.h
include_directories ("${CMAKE_BINARY_DIR}")

# Framework include directories
include_directories ("${PROJECT_SOURCE_DIR}/framework/engine")
include_directories ("${PROJECT_SOURCE_DIR}/framework/public")
include_directories ("${PROJECT_SOURCE_DIR}/framework/channel")
include_directories ("${PROJECT_SOURCE_DIR}/framework/compress")
include_directories ("${PROJECT_SOURCE_DIR}/framework/public")

# In the component directory, we'll refer to #include
# "components/data.h" and "components/process.h" so include the base
# project source directory.
include_directories ("${PROJECT_SOURCE_DIR}")
# These includes provide a path to the legacy component interface header files
include_directories ("${PROJECT_SOURCE_DIR}/components/std/data/numeric")
include_directories ("${PROJECT_SOURCE_DIR}/components/std/data/spikes")
include_directories ("${PROJECT_SOURCE_DIR}/components/std/util/rng")

add_subdirectory(framework)
add_subdirectory(components)
add_subdirectory(support)

# Ensure the BrahmsConfig.h header gets installed
install(FILES ${CMAKE_BINARY_DIR}/BrahmsConfig.h DESTINATION ${INCLUDE_INSTALL_PATH})

# For debugging, you can list variables like this:
set(DEBUG_VARIABLES OFF)
if(DEBUG_VARIABLES)
  get_cmake_property(_variableNames VARIABLES)
  foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
  endforeach()
endif(DEBUG_VARIABLES)
