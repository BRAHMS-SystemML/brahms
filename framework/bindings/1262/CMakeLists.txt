if(COMPILE_PYTHON_BINDING)
  # See BrahmsLibFind.cmake which gets these include paths:
  include_directories(${BRAHMS_NUMPY_INCLUDES} ${BRAHMS_PYTHON_INCLUDES})
  add_library(binding_1262 SHARED python.cpp)
endif(COMPILE_PYTHON_BINDING)

