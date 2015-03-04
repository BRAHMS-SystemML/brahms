if(COMPILE_PYTHON_BINDING)
  message(STATUS "Compiling python binding (1262)")
  # See BrahmsLibFind.cmake which gets these include paths:
  include_directories(${BRAHMS_NUMPY_INCLUDES} ${BRAHMS_PYTHON_INCLUDES})
  add_library(binding_1262 SHARED python.cpp)
else()
  message(STATUS "Omitting python binding (1262)")
endif(COMPILE_PYTHON_BINDING)

