# Note: channel-common is compiled into brahms-engine-base

# brahms-channel-sockets
add_subdirectory (sockets)

# brahms-channel-mpich2
if(COMPILE_WITH_MPICH2)
  message(STATUS "Compiling brahms-channel-mpich2")
  add_subdirectory (mpich2)
else()
  message(STATUS "Omitting brahms-channel-mpich2")
  message(STATUS "  (Include brahms-channel-mpich2 with `cmake -DCOMPILE_WITH_MPICH2=ON`)")
endif(COMPILE_WITH_MPICH2)
