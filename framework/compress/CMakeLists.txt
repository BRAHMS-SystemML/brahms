add_library (brahms-compress SHARED compress.cpp)

if(ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIRS})
    target_link_libraries(
      brahms-compress ${ZLIB_LIBRARIES}
      )
endif(ZLIB_FOUND)

if(STANDALONE_INSTALL)
  install(TARGETS brahms-compress DESTINATION BRAHMS/bin)
else()
  install(TARGETS brahms-compress DESTINATION lib)
endif()
