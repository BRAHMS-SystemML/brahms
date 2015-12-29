add_library (brahms-compress SHARED compress.cpp)

if(ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIRS})
    target_link_libraries(
      brahms-compress ${ZLIB_LIBRARIES}
      )
endif(ZLIB_FOUND)

set_target_properties(brahms-compress PROPERTIES SOVERSION 1.0.0)

install(TARGETS brahms-compress DESTINATION ${LIB_INSTALL_PATH}/brahms)
