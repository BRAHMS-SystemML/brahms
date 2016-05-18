# Because the MPI code needs to be compiled with mpicxx, we need
# another CMakeLists.txt file to compile libbrahms-channel-mpich2,
# hence this code in a mpich2/ subdirectory and the sockets code is in
# a sockets/ subdir.
if(COMPILE_WITH_MPICH2)
  set(CMAKE_CXX_FLAGS "${BRAHMS_HOST_DEFINITION} -D__MPI__")
  set(CMAKE_CXX_COMPILER "mpicxx")
  add_library(brahms-channel-mpich2 SHARED
    ../channel.cpp ../deliverer.cpp
    mpi.cpp mpi-receiver.cpp mpi-sender.cpp
    )
  set_target_properties(brahms-channel-mpich2 PROPERTIES SOVERSION 1.0.0)
  install(TARGETS brahms-channel-mpich2 DESTINATION ${LIB_INSTALL_PATH})
endif(COMPILE_WITH_MPICH2)
