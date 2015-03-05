add_library(brahms-channel-common SHARED channel-common.cpp)
install(TARGETS brahms-channel-common DESTINATION lib)
add_subdirectory (sockets)
add_subdirectory (mpich2)
