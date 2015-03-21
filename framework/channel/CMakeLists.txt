add_library(brahms-channel-common SHARED channel-common.cpp)

if(APPLE)
if(COMPILE_WITH_MPICH2)
  target_link_libraries(
    brahms-channel-common brahms-channel-sockets brahms-channel-mpich2
    )
else()
  target_link_libraries(
    brahms-channel-common brahms-channel-sockets brahms-engine-base
    )
endif(COMPILE_WITH_MPICH2)
endif(APPLE)

install(TARGETS brahms-channel-common DESTINATION lib)

add_subdirectory (sockets)

if(COMPILE_WITH_MPICH2)
  add_subdirectory (mpich2)
endif(COMPILE_WITH_MPICH2)
