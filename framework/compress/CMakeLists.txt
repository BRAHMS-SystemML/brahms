add_library (brahms-compress SHARED compress.cpp)

if(ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIRS})
    target_link_libraries(
      brahms-compress ${ZLIB_LIBRARIES}
      )
endif(ZLIB_FOUND)

install(TARGETS brahms-compress DESTINATION ${LIB_INSTALL_PATH})
