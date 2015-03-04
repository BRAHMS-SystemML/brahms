#
# Library finding code called from top-level CMakeLists.txt file.
# This locates WX, Xaw, Xv and possibly zlib.
#
# It defines a set of variables which are then used in the top level
# CMakeLists.txt file:
#
# BRAHMS_WX_CXXFLAGS
# BRAHMS_WX_CFLAGS
# BRAHMS_XAW_INCLUDE_DIR
# BRAHMS_XAW_LDFLAGS
#

# If you're going to use matlab bindings, then set the paths here.
set(BRAHMS_MATLAB_INCLUDES "/usr/local/MATLAB/R2014b" "/usr/local/MATLAB/R2014b/extern/include")

# Python stuff.
if (COMPILE_PYTHON_BINDING)
  find_package(PythonLibs)
  if (PYTHONLIBS_FOUND)
    set(BRAHMS_PYTHON_INCLUDES ${PYTHON_INCLUDE_DIRS})
    set(CMD_ARGS "-c 'from numpy import *; print get_include()'")
    execute_process(COMMAND /usr/bin/python ${CMD_ARGS}
      OUTPUT_VARIABLE BRAHMS_NUMPY_INCLUDES ERROR_VARIABLE BRAHMS_NUMPY_TEST_ERR OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(COMPARE EQUAL "${BRAHMS_NUMPY_INCLUDES}" "" NUMPY_NOT_FOUND)
    if(NUMPY_NOT_FOUND)
      message(ERROR " You need numpy: ${BRAHMS_NUMPY_TEST_ERR} On Debian/Ubuntu try `sudo apt-get install python-numpy`")
    endif(NUMPY_NOT_FOUND)
  endif(PYTHONLIBS_FOUND)
endif (COMPILE_PYTHON_BINDING)

# Could use X11_Xt_FOUND X11_Xmu_FOUND etc from find_package(X11). It
# may be that find_package(X11) will much up on Windows.
find_package(X11)

# If we have pkg-config then we can use it:
find_package(PkgConfig)

# Use find_package as first attempt, then pkg-config as fallback on this system.
# Try sudo apt-get install libwxgtk2.8-dev  find_package(wxWindows) # This does work if you have libwxgtk2.8-dev
find_package(wxWindows)
string(COMPARE EQUAL "${WX_CONFIG_LIBS}" "" WX_NOT_FOUND)
if (WX_NOT_FOUND)
  # There's no way to find WX (pkg-config won't find this on my Ubuntu system)
  message(ERROR "You need WX windows. On Debian/Ubuntu try `sudo apt-get install libwxgtk2.8-dev`")
else()
  # We know we have WX, so we should be able to exec wx-config to get the compiler flags:
  execute_process(COMMAND wx-config --cxxflags
    OUTPUT_VARIABLE BRAHMS_WX_CXXFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(COMMAND wx-config --cflags
    OUTPUT_VARIABLE BRAHMS_WX_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
endif(WX_NOT_FOUND)

if (PKG_CONFIG_FOUND)
  # There's no FindXaw script on my Ubuntu system. Can use pkg-config to check it's present:
  pkg_check_modules (XAW REQUIRED xaw7)
  if (XAW_FOUND)
    # We have XAW_LDFLAGS XAW_INCLUDEDIR
    find_path(BRAHMS_XAW_INCLUDE_DIR Xaw/XawInit.h HINTS ${XAW_INCLUDEDIR} ${XAW_INCLUDE_DIRS})
    set(BRAHMS_XAW_LDFLAGS ${XAW_LDFLAGS})
  else()
    message(ERROR "You need libXaw7. On Debian/Ubuntu try `sudo apt-get install libxaw7-dev`")
  endif(XAW_FOUND)
endif()

# end of lib finding.

# For debugging, you can list variables like this:
#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#  message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()
