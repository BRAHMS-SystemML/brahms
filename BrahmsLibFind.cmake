#
# Library finding code called from top-level CMakeLists.txt file.
# This locates WX, Xaw and Python includes and libs and allows the
# user to specify paths to their matlab installation
#
# It defines a set of variables which are then used in the top level
# CMakeLists.txt file:
#
# BRAHMS_WX_CXXFLAGS
# BRAHMS_WX_CFLAGS
# BRAHMS_XAW_INCLUDE_DIR
# BRAHMS_XAW_LDFLAGS
#

# We need threads
find_package (Threads)
if(THREADS_FOUND)
  message(STATUS "Found Threads with find_package :)")
else()
  message(STATUS "Didn't find Threads with find_package, fallback to check_include_files search for pthread.h.")
  # This is a fallback for Mac:
  include (CheckIncludeFiles)
  check_include_files("pthread.h" CMAKE_HAVE_PTHREAD_H) 
  if(NOT CMAKE_HAVE_PTHREAD_H)
    message(FATAL_ERROR " You need a threading library")
  endif(NOT CMAKE_HAVE_PTHREAD_H)
endif(NOT THREADS_FOUND)

# We need -lrt on some platforms for clock_gettime (on others, clock_gettime is in libc)
if (UNIX AND NOT APPLE)
  include(CheckFunctionExists)
  set(CMAKE_EXTRA_INCLUDE_FILES time.h)
  check_function_exists(clock_gettime HAVE_CLOCK_GETTIME)
  if(NOT HAVE_CLOCK_GETTIME)
    message(STATUS "Didn't find clock_gettime with initial check_function_exists test")
    include(CheckLibraryExists)
    # On systems with glibc < 2.16 clock_gettime is in rt library
    find_library(LIBRT_LIBRARIES rt)
    message(STATUS "LIBRT_LIBRARIES ${LIBRT_LIBRARIES}")
    set(HAVE_CLOCK_GETTIME_IN_RT 1)
    # This doesn't seem to work on Iceberg:
    #check_library_exists(${LIBRT_LIBRARIES} clock_gettime "time.h" HAVE_CLOCK_GETTIME_IN_RT)
    #if(NOT HAVE_CLOCK_GETTIME_IN_LIBRT)
    #  message(FATAL_ERROR "clock_gettime not found")
    #endif(NOT HAVE_CLOCK_GETTIME_IN_LIBRT)
  endif(NOT HAVE_CLOCK_GETTIME)
endif(UNIX AND NOT APPLE)

# If you're going to use matlab bindings, then set the paths here.
if(COMPILE_MATLAB_BINDING)
  set(BRAHMS_MATLAB_INCLUDES "/usr/local/MATLAB/R2014b" "/usr/local/MATLAB/R2014b/extern/include")
endif(COMPILE_MATLAB_BINDING)

# Python stuff.
if(COMPILE_PYTHON_BINDING)
  find_package(PythonLibs)
  if (PYTHONLIBS_FOUND)
    set(BRAHMS_PYTHON_INCLUDES ${PYTHON_INCLUDE_DIRS})
    set(CMD_ARGS "from numpy import *\; print get_include()")
    execute_process(COMMAND /usr/bin/python -c ${CMD_ARGS}
      OUTPUT_VARIABLE BRAHMS_NUMPY_INCLUDES
      ERROR_VARIABLE BRAHMS_NUMPY_TEST_ERR
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(COMPARE EQUAL "${BRAHMS_NUMPY_INCLUDES}" "" NUMPY_NOT_FOUND)
    if(NUMPY_NOT_FOUND)
      message(FATAL_ERROR " You need numpy. On Debian/Ubuntu try `sudo apt-get install python-numpy`")
    endif(NUMPY_NOT_FOUND)
  endif(PYTHONLIBS_FOUND)
endif (COMPILE_PYTHON_BINDING)

if(COMPILE_WITH_MPICH2)
  # Test for mpicxx. FIXME: `which` is unix-specific.
  find_path(BRAHMS_MPICXX_PATH NAMES mpicxx HINTS /usr/bin /usr/local/bin)
  find_path(BRAHMS_MPICXX_PATH NAMES mpicxx)
  #execute_process(COMMAND which mpicxx
  #  OUTPUT_VARIABLE BRAHMS_MPICXX_PATH
  #  ERROR_VARIABLE BRAHMS_MPICXX_TEST_ERR
  #  OUTPUT_STRIP_TRAILING_WHITESPACE)
  string(COMPARE EQUAL "${BRAHMS_MPICXX_PATH}" "" MPICXX_NOT_FOUND)
  if(MPICXX_NOT_FOUND)
    message(FATAL_ERROR " You need mpicxx to compile brahms-channel-mpich2. Try `sudo apt-get install mpich2`")
  endif(MPICXX_NOT_FOUND)
endif(COMPILE_WITH_MPICH2)

# Could use X11_Xt_FOUND X11_Xmu_FOUND etc from find_package(X11). It
# may be that find_package(X11) will much up on Windows.
find_package(X11)

# If we have pkg-config then we can use it:
find_package(PkgConfig)

if(COMPILE_WX_COMPONENT)
  # Use find_package as first attempt, then pkg-config as fallback on this system.
  # Try sudo apt-get install libwxgtk2.8-dev
  find_package(wxWindows)
  string(COMPARE EQUAL "${WX_CONFIG_LIBS}" "" WX_NOT_FOUND)
  if (WX_NOT_FOUND)
    # There's no way to find WX (pkg-config won't find this on my Ubuntu system)
    message(FATAL_ERROR "You need WX windows. On Debian/Ubuntu try `sudo apt-get install libwxgtk2.8-dev`")
  else()
    if (WX_CONFIG_LIBS MATCHES .*gtk.*)
      # all is well, seems to be a graphical wxwindows FIXME: May be different on Windows.
    else()
      message(FATAL_ERROR "You need graphical WX windows. On Debian/Ubuntu try `sudo apt-get install libwxgtk2.8-dev`")
    endif()

    # We know we have WX, so we should be able to exec wx-config to get
    # the compiler flags: FIXME: Windows invocation will be different here.
    execute_process(COMMAND wx-config --cxxflags
      OUTPUT_VARIABLE BRAHMS_WX_CXXFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND wx-config --cflags
      OUTPUT_VARIABLE BRAHMS_WX_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
  endif(WX_NOT_FOUND)
endif(COMPILE_WX_COMPONENT)

if(PKG_CONFIG_FOUND)
  # There's no FindXaw script on my Ubuntu system. Can use pkg-config to check it's present:
  pkg_check_modules(XAW REQUIRED xaw7)
  if (XAW_FOUND)
    # We have XAW_LDFLAGS XAW_INCLUDEDIR
    find_path(BRAHMS_XAW_INCLUDE_DIR Xaw/XawInit.h HINTS ${XAW_INCLUDEDIR} ${XAW_INCLUDE_DIRS})
    set(BRAHMS_XAW_LDFLAGS ${XAW_LDFLAGS})
  else()
    message(FATAL_ERROR "You need libXaw7. On Debian/Ubuntu try `sudo apt-get install libxaw7-dev`")
  endif(XAW_FOUND)
endif()

# end of lib finding.
